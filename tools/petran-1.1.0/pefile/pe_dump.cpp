// PE_DUMP.CPP
//
// Copyright (c) 1996-1999 Symbian Ltd.  All rights reserved.
//

#include <e32image.h>
#include <h_utl.h>
#include <string.h>
#include "h_endian.h"

void PriorityToStr(TProcessPriority aPri, char *aStr)
	{

	if (aPri==EPrioritySupervisor)
		strcpy(aStr,"Supervisor");

	else if (aPri>EPriorityRealTimeServer)
		sprintf(aStr, "RealTime+%d", aPri-EPriorityRealTimeServer);
	else if (aPri==EPriorityRealTimeServer)
		strcpy(aStr,"RealTime");

	else if (aPri>EPriorityFileServer)
		sprintf(aStr, "FileServer+%d", aPri-EPriorityFileServer);
	else if (aPri==EPriorityFileServer)
		strcpy(aStr,"FileServer");

	else if (aPri>EPriorityWindowServer)
		sprintf(aStr, "WindowServer+%d", aPri-EPriorityWindowServer);
	else if (aPri==EPriorityWindowServer)
		strcpy(aStr,"WindowServer");

	else if (aPri>EPriorityHigh)
		sprintf(aStr, "High+%d", aPri-EPriorityHigh);
	else if (aPri==EPriorityHigh)
		strcpy(aStr,"High");

	else if (aPri>EPriorityForeground)
		sprintf(aStr, "Foreground+%d", aPri-EPriorityForeground);
	else if (aPri==EPriorityForeground)
		strcpy(aStr,"Foreground");

	else if (aPri>EPriorityBackground)
		sprintf(aStr, "Background+%d", aPri-EPriorityBackground);
	else if (aPri==EPriorityBackground)
		strcpy(aStr,"Background");

	else if (aPri>EPriorityLow)
		sprintf(aStr, "Low+%d", aPri-EPriorityLow);
	else if (aPri==EPriorityLow)
		strcpy(aStr,"Low");

	else
		sprintf(aStr, "Illegal (%d)", aPri);
	}

void nl()
	{
	Print(EAlways, "\n");
	}

void E32ImageFile::Dump(TText *aFileName)
	{
	if (IsValid())
		{
		Print(EAlways, "E32ImageFile '%s'\n", aFileName);
		DumpHeader();
		DumpData();
		}
	else
		Print(EAlways, "This is not an E32 image file.\n");
	}

void E32ImageFile::DumpHeader()
	{
	Print(EAlways, "V%d.%02d(%03d)", iHeader->iVersion.iMajor,iHeader->iVersion.iMinor,iHeader->iVersion.iBuild);
	Print(EAlways, "\tTime Stamp: %08x,%08x\n", iHeader->iTime.High(), iHeader->iTime.Low());
	Print(EAlways, (char *)&iHeader->iSignature);
	if (iHeader->iFlags&KImageDll)
		Print(EAlways, " Dll for ");
	else
		Print(EAlways, " Exe for ");
	switch (iHeader->iCpu)
		{
	case ECpuX86:
		Print(EAlways, "X86 CPU\n");
		break;
	case ECpuArm:
		Print(EAlways, "ARM CPU\n");
		break;
	case ECpuMCore:
		Print(EAlways, "M*Core CPU\n");
		break;
	case ECpuUnknown:
		Print(EAlways, "Unknown CPU\n");
		break;
	default:
		Print(EAlways, "something or other\n");
		break;
		}
	if (!(iHeader->iFlags&KImageDll))
		{
		char str[80];
		PriorityToStr(iHeader->iPriority, str);
		Print(EAlways, "Priority %s\n", str);
		if (iHeader->iFlags&KImageFixedAddressExe)
			Print(EAlways, "Fixed process\n");
		}
	if (iHeader->iFlags&KImageNoCallEntryPoint)
		Print(EAlways, "Entry points are not called\n");
	Print(EAlways, "Uids:\t\t%08x %08x %08x (%08x)\n", iHeader->iUid1, iHeader->iUid2, iHeader->iUid3, iHeader->iCheck);
	Print(EAlways, "File Size:\t%08x\n", iSize);
	Print(EAlways, "Code Size:\t%08x\n", iHeader->iCodeSize);
	Print(EAlways, "Data Size:\t%08x\n", iHeader->iDataSize);
	Print(EAlways, "Chk code/data:\t%08x/%08x\n", iHeader->iCheckSumCode, iHeader->iCheckSumData);
	Print(EAlways, "Min Heap Size:\t%08x\n", iHeader->iHeapSizeMin);
	Print(EAlways, "Max Heap Size:\t%08x\n", iHeader->iHeapSizeMax);
	Print(EAlways, "Stack Size:\t%08x\n", iHeader->iStackSize);
	Print(EAlways, "Code link addr:\t%08x\n", iHeader->iCodeBase);
	Print(EAlways, "Data link addr:\t%08x\n", iHeader->iDataBase);
	Print(EAlways, "Code reloc offset:\t%08x\n", iHeader->iCodeRelocOffset);
	Print(EAlways, "Data reloc offset:\t%08x\n", iHeader->iDataRelocOffset);
	Print(EAlways, "Dll ref table count: %d\n", iHeader->iDllRefTableCount);

	if (iHeader->iCodeSize || iHeader->iDataSize || iHeader->iBssSize || iHeader->iImportOffset)
		Print(EAlways, "\tOffset\tSize\tRelocs\tNumOfRelocs\n");

	Print(EAlways, "Code\t%06x\t%06x", iHeader->iCodeOffset, iHeader->iCodeSize);
	if (iHeader->iCodeRelocOffset)
		{
		E32RelocSection *r=(E32RelocSection *)(iData+iHeader->iCodeRelocOffset);
		Print(EAlways, "\t%06x\t%06x", iHeader->iCodeRelocOffset, vflipi(r->iNumberOfRelocs));
		}
	else
		Print(EAlways, "\t\t");
	Print(EAlways, "\t\t+%06x (entry pnt)", iHeader->iEntryPoint);
	nl();

	Print(EAlways, "Data\t%06x\t%06x", iHeader->iDataOffset, iHeader->iDataSize);
	if (iHeader->iDataRelocOffset)
		{
		E32RelocSection *r=(E32RelocSection *)(iData+iHeader->iDataRelocOffset);
		Print(EAlways, "\t%06x\t%06x", iHeader->iDataRelocOffset, vflipi(r->iNumberOfRelocs));
		}
	nl();

	Print(EAlways, "Bss\t\t%06x\n", iHeader->iBssSize);

	if (iHeader->iExportDirOffset)
		Print(EAlways, "Export\t%06x\t%06x\t\t\t\t(%d entries)\n", iHeader->iExportDirOffset, iHeader->iExportDirCount*4, iHeader->iExportDirCount);
	if (iHeader->iImportOffset)
		Print(EAlways, "Import\t%06x\n", iHeader->iImportOffset);
	}

void dump(TUint *aData, TInt aLength)
	{
	TUint *p=aData;
	TInt i=0;
	char line[256];
	char *cp=(char*)aData;
	TInt j=0;
	memset(line,' ',sizeof(line));
	while (i<aLength)
		{
		TInt ccount=0;
		char* linep=&line[8*9+2];
		Print(EAlways, "%06x:", i);
		while (i<aLength && ccount<8)
			{
			  TUint dd = *p++;  
			  Print(EAlways," %08x", vflipi(dd));
			  i+=4;
			  ccount++;
			for (j=0; j<4; j++)
				{
				unsigned char c=*cp++;
				if (c<32 || c>127)
					{
					c = '.';
					}
				*linep++ = c;
				}
			}
		*linep='\0';
		Print(EAlways, "%s", line+(ccount*9));
		nl();
		}
	}

void dumprelocs(char *aRelocs)
	{

	TInt num=((E32RelocSection *)aRelocs)->iNumberOfRelocs;

	flipi(num); // re order

	Print(EAlways, "%d relocs\n", num);
	aRelocs+=sizeof(E32RelocSection);
	TInt printed=0;
	while (num>0)
		{
		TInt page=*(TUint *)aRelocs;
		TInt size=*(TUint *)(aRelocs+4);
		
		flipi(page);
		flipi(size);
		
		TInt pagesize=size;
		size-=8;
		TUint16 *p=(TUint16 *)(aRelocs+8);
		while (size>0)
			{
			TUint16 a=*p++;
			
			flips(a); // re order

			if ((a&0xf000)==0x3000)
				{
				Print(EAlways, "%08x ", page+(a&0x0fff));
				printed++;
				if (printed>7)
					{
					nl();
					printed=0;
					}
				}
			size-=2;
			num--;
			}
		aRelocs+=pagesize;
		}
	nl();
	}


void E32ImageFile::DumpData()
	{

	Print(EAlways, "\nCode (text size=%08x)\n", iHeader->iTextSize);
	dump((TUint *)(iData+iHeader->iCodeOffset), iHeader->iCodeSize);
	if (iHeader->iCodeRelocOffset)
		dumprelocs(iData+iHeader->iCodeRelocOffset);

	if (iHeader->iDataOffset)
		{
		Print(EAlways, "\nData\n");
		dump((TUint *)(iData+iHeader->iDataOffset), iHeader->iDataSize);
		if (iHeader->iDataRelocOffset)
			dumprelocs(iData+iHeader->iDataRelocOffset);
		}
	if (iHeader->iImportOffset)
		{
		E32ImportSection *isection=(E32ImportSection *)(iData+iHeader->iImportOffset);
		Print(EAlways, "\nIdata\tSize=%08x\n", vflipi(isection->iSize));
		Print(EAlways, "Offset of import address table (relative to code section): %08x\n", iHeader->iTextSize);
		TInt d;
		E32ImportBlock *b=(E32ImportBlock *)(isection+1);
		for (d=0; d<iHeader->iDllRefTableCount; d++)
			{
			  E32ImportBlock bb = *b; // for ease, so we can byte swap below
			  
			  flipE32ImportBlock(bb); 
			char *dllname=iData+iHeader->iImportOffset+bb.iOffsetOfDllName;
			TInt n=bb.iNumberOfImports;
			Print(EAlways, "%d imports from %s\n", bb.iNumberOfImports, dllname);
			TUint *p=(TUint *)(((char *)b)+sizeof(E32ImportBlock));
			while (n--) {
			  TUint ord = *p++;			  
			  Print(EAlways, "\t%d\n", vflipi(ord));
			}
			b=(E32ImportBlock *)(((char *)b)+bb.Size());
			}
		}
	}
