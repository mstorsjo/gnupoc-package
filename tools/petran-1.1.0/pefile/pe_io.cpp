// PE_IO.CPP
//
// Copyright (c) 1996-1999 Symbian Ltd.  All rights reserved.
//

#include <fstream>
#include <e32image.h>
#include <h_utl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include "h_endian.h"

ofstream &operator<<(ofstream &os, const E32ImageFile &aImage)
//
// Output an E32ImageFile
//
	{
	  TUint size = aImage.iSize;
	  flipE32ImageHeader(*aImage.iHeader); // re order, write then revert
	  os.write(aImage.iData, size);
	  flipE32ImageHeader(*aImage.iHeader); // re order, write then revert

	  return os;
	}

ifstream &operator>>(ifstream &is, E32ImageFile &aImage)
//
// Input an E32ImageFile
//
	{

	is.read(aImage.iData, aImage.iSize);
	aImage.iHeader=(E32ImageHeader *)aImage.iData;

	flipE32ImageHeader(*aImage.iHeader); // re order

	aImage.iExportName=NULL;
	return is;
	}

TInt E32ImageFile::IsE32ImageFile(char *aFileName)
	{

	ifstream ifile(aFileName, ios::in|ios::binary);
	if(!ifile.is_open())
		return FALSE;
	E32ImageHeader h;
	E32ImageFile f;
	f.iHeader=&h;
	ifile.read((char *)&h, sizeof(E32ImageHeader));

	flipE32ImageHeader(h); // re order

	ifile.close();


	return f.IsValid();
	}

TInt E32ImageFile::Open(const TText *const aFileName)
//
// Open an E32 Image file
//
	{
	struct stat s;
	const int ret = stat((char *)aFileName, &s);
	if (ret==-1) 
		{
		Print(EError,"Cannot open %s for input (%s).\n",aFileName, strerror(errno));
		return 1;
		}
	Adjust(s.st_size);
	ifstream ifile((char *)aFileName, ios::in|ios::binary);
	if(!ifile.is_open())
		{
		Print(EError,"Cannot open %s for input.\n",aFileName);
		return 1;
		}
	ifile >> *this; // re order will happen inside operator...
	ifile.close();
	if (!IsValid())
		return KErrGeneral;
	iFileName=strdup((char *)aFileName);
	if (iFileName==NULL)
		return KErrNoMemory;
	return KErrNone;
	}
