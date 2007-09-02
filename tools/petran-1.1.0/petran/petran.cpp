// PETRAN.CPP
//
// Copyright (c) 1996-1999 Symbian Ltd.  All rights reserved.
//

//
// PE file preprocessor for E32
//

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <stdlib.h>
#include <e32std.h>
#include <e32image.h>
#include <h_utl.h>
#include <h_ver.h>

#include <deflate.h>

using namespace std;

void DeflateCompress(char* bytes, TInt size, ostream& os);

extern int gAlignConstSection;
extern TUint gConstSectionAddressMask;

int gVerbose=FALSE;
char *gFile1=NULL;
char *gFile2=NULL;
unsigned int gStack=0;
unsigned int gHeapMin=0;
unsigned int gHeapMax=0;
TUid gUid1=KNullUid;
TUid gUid2=KNullUid;
TUid gUid3=KNullUid;
int gCallEntryPoints=TRUE;
int gFixedAddress=FALSE;
int gPriority=EPriorityForeground;
int gAllowDllData=FALSE;
TUint gDataBase=0;

int gSetStack=FALSE;
int gSetHeap=FALSE;
int gSetUid1=FALSE;
int gSetUid2=FALSE;
int gSetUid3=FALSE;
int gSetCallEntryPoints=FALSE;
int gSetFixedAddress=FALSE;
int gSetPriority=FALSE;
int gCompress=FALSE;
int gSetCompress=FALSE;

TBool gDumpPe = FALSE;



int dodumppe(char *ifilename)
{
  PEFile pefile;
  
  if (!pefile.Init((TText *)ifilename))
    return KErrGeneral;
  TInt r=pefile.ReadSectionHeaders();
  if (r!=KErrNone) return r;
  r=pefile.ReadData();
  if (r!=KErrNone) return r;
  
  pefile.DumpPeHeaders();
  pefile.Close();

  return KErrNone;
}
int dotran(char *ifilename, char *ofilename)
	{
	E32ImageFile f;
	int r=f.Translate((TText *)ifilename, gDataBase, gAllowDllData);
	if (r!=KErrNone)
		return r;
	if (gSetStack)
		f.SetStackSize(gStack);
	if (gSetHeap)
		{
		f.SetHeapSizeMin(gHeapMin);
		f.SetHeapSizeMax(gHeapMax);
		}
	if (!gSetUid1)
		gUid1=TUid::Uid(f.iHeader->iUid1);
	if (!gSetUid2)
		gUid2=TUid::Uid(f.iHeader->iUid2);
	if (!gSetUid3)
		gUid3=TUid::Uid(f.iHeader->iUid3);
	f.SetUids(gUid1, gUid2, gUid3);
	if (gSetCallEntryPoints)
		f.SetCallEntryPoints(gCallEntryPoints);
	if (gSetPriority)
		{
		if (f.iHeader->iFlags&KImageDll)
			Print(EWarning,"Cannot set priority of a DLL.\n");
		else
			f.SetPriority((TProcessPriority)gPriority);
		}
	if (gSetFixedAddress)
		{
		if (f.iHeader->iFlags&KImageDll)
			Print(EWarning,"Cannot set fixed address for DLL.\n");
		else
			f.SetFixedAddress(gFixedAddress);
		}
	if (gCompress) {
		f.iHeader->iFlags |= 0x01000000;
		f.iHeader->iCheckSumData = 0x101f7afc;
	}

	ofstream ofile(ofilename, ios_base::binary);
	if (!ofile)
		{
		Print(EError,"Cannot open %s for output.\n",ofilename);
		return 1;
		}
	ofile << f;
	TUint32 len = ofile.tellp();
	ofile.close();

	if (gCompress) {
		len -= 124;
		ifstream ifile(ofilename, ios_base::binary);
		if (!ifile)
			{
			Print(EError,"Cannot open %s for input.\n",ofilename);
			return 1;
			}
		TUint8* header = new TUint8[124];
		ifile.read((char*) header, 124);
		TUint8* data = new TUint8[len];
		ifile.read((char*) data, len);
		ifile.close();

		ofstream ofile(ofilename, ios::binary);
		if (!ofile)
			{
			Print(EError,"Cannot open %s for output.\n",ofilename);
			return 1;
			}
		ofile.write((char*)header, 124);
		TUint8 uncompressedLength[4];
		uncompressedLength[0] = (len >>  0) & 0xff;
		uncompressedLength[1] = (len >>  8) & 0xff;
		uncompressedLength[2] = (len >> 16) & 0xff;
		uncompressedLength[3] = (len >> 24) & 0xff;
		ofile.write((char*) uncompressedLength, 4);
		DeflateCompress((char*) data, len, ofile);
		ofile.close();

		delete [] data;
		delete [] header;
	}


	if (gVerbose)
		f.Dump((TText *)ofilename);
	return KErrNone;
	}


int dodump(char *ifilename)
	{

	E32ImageFile f;

	struct stat s;
	const int ret = stat((char *)ifilename, &s);
	if (ret==-1) 
		{
		Print(EError,"Cannot open %s for input (%s).\n",ifilename, strerror(errno));
		return 1;
		}

	f.Adjust(s.st_size);

	ifstream ifile(ifilename, ios::in|ios::binary);
	if(!ifile.is_open())
		{
		Print(EError,"Cannot open %s for input.\n",ifilename);
		return 1;
		}
	ifile >> f;
	ifile.close();
	f.Dump((TText *)ifilename);
	return KErrNone;
	}

int doalter(char *ifilename)
	{

	E32ImageFile f;

	struct stat s;
	const int ret = stat((char *)ifilename, &s);
	if (ret==-1) 
		{
		Print(EError,"Cannot open %s for input (%s).\n",ifilename, strerror(errno));
		return 1;
		}
	const unsigned long fsize = s.st_size;


	f.Adjust(fsize);

	ifstream ifile(ifilename, ios::in|ios::binary);
	if(!ifile.is_open())
		{
		Print(EError,"Cannot open %s for input.\n",ifilename);
		return 1;
		}
	ifile >> f;
	ifile.close();

	if (gDataBase)
		{
		Print(EWarning, "Ignoring -datalinkaddress Switch");
		}
	if (gSetStack)
		f.SetStackSize(gStack);
	if (gSetHeap)
		{
		f.SetHeapSizeMin(gHeapMin);
		f.SetHeapSizeMax(gHeapMax);
		}
	if (!gSetUid1)
		gUid1=TUid::Uid(f.iHeader->iUid1);
	if (!gSetUid2)
		gUid2=TUid::Uid(f.iHeader->iUid2);
	if (!gSetUid3)
		gUid3=TUid::Uid(f.iHeader->iUid3);
	f.SetUids(gUid1, gUid2, gUid3);
	if (gSetCallEntryPoints)
		f.SetCallEntryPoints(gCallEntryPoints);
	if (gSetPriority)
		{
		if (f.iHeader->iFlags&KImageDll)
			Print(EWarning,"Cannot set priority of a DLL.\n");
		else
			f.SetPriority((TProcessPriority)gPriority);
		}
	if (gSetFixedAddress)
		{
		if (f.iHeader->iFlags&KImageDll)
			Print(EWarning,"Cannot set fixed address for DLL.\n");
		else
			f.SetFixedAddress(gFixedAddress);
		}
	ofstream ofile(ifilename, ios::binary);
	if (!ofile)
		{
		Print(EError,"Cannot open %s for output.\n",ifilename);
		return 1;
		}
	ofile << f;
	ofile.close();
	if (gVerbose)
		f.Dump((TText *)ifilename);
	return KErrNone;
	}

int helpme(char *aStr)
	{
	Print(EAlways,"Syntax: %s [options] pefile outputfile\n",aStr);
	Print(EAlways,"        %s [options] e32imagefile\n",aStr);
	Print(EAlways,"option: [-v] [[-no]call[entrypoint]] [-priority <priority>]\n");
	Print(EAlways,"        [-stack <size>] [-heap <min> <max>] [-uid<n> <uid>]\n");
	Print(EAlways,"        [-allowdlldata] [-datalinkaddress <base>] [-fixed] [-moving]\n");
	Print(EAlways,"        [-align-const-section] [-const-section-address-mask <mask>] [-dump-pe]\n");
	Print(EAlways,"        [-[no]compress]\n");
	return KErrArgument;
	}

int isNumber(char *aStr)
	{
	return (aStr[0]>='0') && (aStr[0]<='9');
	}

int getUIntArg(unsigned int &aVal, int argc, char *argv[], int i)
	{
	if (i>=argc)
		return KErrArgument;
	if (!isNumber(argv[i]))
		return KErrArgument;
	aVal = strtoul(argv[i], NULL, 0 /* auto-base */);
	return KErrNone;
	}

int getPriorityArg(int &aVal, int argc, char *argv[], int i)
	{

	if (i>=argc)
		return KErrArgument;
	if (isNumber(argv[i]))
		{
		stringstream s(argv[i]);
		s>>aVal;
		}
	else
		{
		if (strcasecmp(argv[i], "low")==0)
			aVal=EPriorityLow;
		else if (strncasecmp(argv[i], "background",4)==0)
			aVal=EPriorityBackground;
		else if (strncasecmp(argv[i], "foreground",4)==0)
			aVal=EPriorityForeground;
		else if (strcasecmp(argv[i], "high")==0)
			aVal=EPriorityHigh;
		else if (strncasecmp(argv[i], "windowserver",3)==0)
			aVal=EPriorityWindowServer;
		else if (strncasecmp(argv[i], "fileserver",4)==0)
			aVal=EPriorityFileServer;
		else if (strncasecmp(argv[i], "realtime",4)==0)
			aVal=EPriorityRealTimeServer;
		else if (strncasecmp(argv[i], "supervisor",3)==0)
			aVal=EPrioritySupervisor;
		else
			{
			Print(EError, "Unrecognised priority\n");
			return KErrArgument;
			}
		}
	if (aVal<EPriorityLow || aVal>EPrioritySupervisor)
		{
		Print(EError, "Priority out of range\n");
		return KErrArgument;
		}
	return KErrNone;
	}


int processCL(int argc, char *argv[])
	{

	int r=KErrNone;
	int i=1;
	while (i<argc)
		{
		if (strcasecmp("-v", argv[i])==0)
			gVerbose=TRUE;
		else if (strcasecmp("-stack", argv[i])==0)
			{
			i++;
			gSetStack=TRUE;
			r=getUIntArg(gStack, argc, argv, i);
			}
		else if (strcasecmp("-uid1", argv[i])==0)
			{
			i++;
			gSetUid1=TRUE;
			unsigned int id;
			r=getUIntArg(id, argc, argv, i);
			gUid1=TUid::Uid(id);
			}
		else if (strcasecmp("-uid2", argv[i])==0)
			{
			i++;
			gSetUid2=TRUE;
			unsigned int id;
			r=getUIntArg(id, argc, argv, i);
			gUid2=TUid::Uid(id);
			}
		else if (strcasecmp("-uid3", argv[i])==0)
			{
			i++;
			gSetUid3=TRUE;
			unsigned int id;
			r=getUIntArg(id, argc, argv, i);
			gUid3=TUid::Uid(id);
			}
		else if (strncasecmp("-nocall", argv[i], 7)==0)
			{
			gSetCallEntryPoints=TRUE;
			gCallEntryPoints=FALSE;
			}
		else if (strncasecmp("-call", argv[i], 5)==0)
			{
			gSetCallEntryPoints=TRUE;
			gCallEntryPoints=TRUE;
			}
		else if (strncasecmp("-fixed", argv[i], 3)==0)
			{
			gSetFixedAddress=TRUE;
			gFixedAddress=TRUE;
			}
		else if (strncasecmp("-moving", argv[i], 3)==0)
			{
			gSetFixedAddress=TRUE;
			gFixedAddress=FALSE;
			}
		else if (strncasecmp("-priority", argv[i], 4)==0)
			{
			i++;
			gSetPriority=TRUE;
			r=getPriorityArg(gPriority,argc,argv,i);
			}
		else if (strncasecmp("-heap", argv[i], 4)==0)
			{
			i++;
			gSetHeap=TRUE;
			r=getUIntArg(gHeapMin, argc, argv, i);
			if (r==KErrNone)
				r=getUIntArg(gHeapMax, argc, argv, ++i);
			}
		else if (strncasecmp("-allowdlldata", argv[i], 6)==0)
			{
			gAllowDllData=TRUE;
			}
		else if (strncasecmp("-compress", argv[i], 9)==0)
			{
			gCompress=TRUE;
			gSetCompress=TRUE;
			}
		else if (strncasecmp("-nocompress", argv[i], 11)==0)
			{
			gCompress=FALSE;
			gSetCompress=TRUE;
			}
		else if (strncasecmp("-datalinkaddress", argv[i], 5)==0)
			{
			i++;
			r=getUIntArg(gDataBase, argc, argv, i);
			}
		else if (strncasecmp("-align-const-section", argv[i], 20)==0)
			{
			gAlignConstSection=TRUE;
			}
		else if (strncasecmp("-const-section-address-mask", argv[i], 27)==0)
			{
			i++;
			r=getUIntArg(gConstSectionAddressMask, argc, argv, i);
			}
		else if (strncasecmp("-dump-pe", argv[i], 8) == 0) {

		  gDumpPe = TRUE;
		}
		else if (gFile1==NULL)
			{
			gFile1=argv[i];
			}
		else if (gFile2==NULL)
			{
			gFile2=argv[i];
			}
		else
			r=KErrArgument;
		if (r!=KErrNone)
			return r;
		i++;
		}
	return KErrNone;
	}

int main(int argc, char *argv[])
	{
	Print(EAlways,"\nPETRAN - PE file preprocessor");
  	Print(EAlways," V%02d.%02d (Build %03d)\n",MajorVersion,MinorVersion,Build);
  	Print(EAlways,Copyright);

	int r=processCL(argc, argv);
	if (r!=KErrNone)
		return helpme(argv[0]);
	if (gFile2)
		return dotran(gFile1, gFile2);
	if (gDumpPe)
	  return dodumppe(gFile1);
	if ((gSetStack || gSetUid1 || gSetUid2 || gSetUid3 || gSetCallEntryPoints || gSetPriority || gSetHeap) && gFile1)
		return doalter(gFile1);
	if (gFile1)
		return dodump(gFile1);
	helpme(argv[0]);
	return KErrArgument;
	}
