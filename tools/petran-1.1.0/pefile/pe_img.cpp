// PE_IMG.CPP
//
// Copyright (c) 1996-1999 Symbian Ltd.  All rights reserved.
//
#include <stdlib.h>
#include <string.h>
#include <e32image.h>
#include <h_utl.h>
#include <fstream.h>
#include "h_endian.h"

//
// E32 Image files
//
E32ImageFile::E32ImageFile()
//
// Constructor
//
	: iData(NULL), iSize(0), iHeader(NULL), iFileName(NULL), iExportName(NULL)
	{}

E32ImageFile::~E32ImageFile()
//
// Destructor
//
	{

	free(iData);
	delete [] iExportName;
	delete [] iFileName;
	}

void E32ImageFile::Adjust(TInt aSize)
//
// Adjust the size of allocated data and fix the member data
//
	{

	if (iSize==0)
		{
		iSize=ALIGN4(aSize);
		iData=(char *)malloc(iSize);
		memset(iData, 0, iSize);
		}
	else
		{
		TInt oldsize=iSize;
		iSize=ALIGN4(aSize);
		iData=(char *)realloc(iData, iSize);
		if (oldsize>iSize)
			memset(iData+oldsize, 0, iSize-oldsize);
		}
	if (iData==NULL)
		iSize=0;
	iHeader=(E32ImageHeader *)iData;
	}

void E32ImageFile::SetStackSize(TInt aSize)
	{
	iHeader->iStackSize=aSize;
	}
void E32ImageFile::SetHeapSizeMin(TInt aSize)
	{
	iHeader->iHeapSizeMin=aSize;
	}
void E32ImageFile::SetHeapSizeMax(TInt aSize)
	{
	iHeader->iHeapSizeMax=aSize;
	}

TUint E32ImageFile::TextOffset()
//
// Return the offset of the text section
//
	{
	return 0;
	}
TUint E32ImageFile::ImportAddressTableOffset()
//
// Return the offset of the iat
//
	{
	return iHeader->iTextSize;
	}
TUint E32ImageFile::ConstOffset()
//
// return the offset of the const data
//
	{
	return iConstOffset;
	}
TUint E32ImageFile::DataOffset()
//
// return the offset of the initialised data
//
	{
	return iHeader->iCodeSize;
	}
TUint E32ImageFile::BssOffset()
//
// return the offset from the start of code where the bss is linked
//
	{
	return DataOffset()+iHeader->iDataSize;
	}

TInt E32ImageFile::IsValid()
//
// returns true if the e32image file is valid
//
	{
	if (iHeader)
		return strncmp((char *)&iHeader->iSignature, "EPOC", 4)==0;
	return FALSE;
	}

TUint E32ImageFile::CheckSumCode()
//
// Return the sum of all words in the text and rdata sections
//
	{
	  TInt size=iHeader->iCodeSize;
	  if (size==0)
	    return 0;
	  TUint *p=(TUint *)(iData+iHeader->iCodeOffset);
	  pflipi(p,size/4); // re order since checksum assumes little endian
	  TUint res = HMem::CheckSum(p, size);
	  pflipi(p,size/4); // then re order again...
	  return res;
	}

TUint E32ImageFile::CheckSumData()
//
//
//
	{

	TInt size=iHeader->iDataSize;
	if (size==0)
		return 0;
	TUint *p=(TUint *)(iData+iHeader->iDataOffset);

	pflipi(p,size/4); // re order since checksum assumes little endian
	TUint res = HMem::CheckSum(p, size);
	pflipi(p,size/4);
	
	return res;
	}

TInt E32ImageFile::IsDll()
//
//
//
	{
	return iHeader->iFlags&KImageDll;
	}

void E32ImageFile::CreateExportName(char *aName)
	{
	
	if (TUid::Uid(iHeader->iUid3)==KNullUid)
		{
		iExportName=strdup(aName);
		return;
		}
	iExportName=new char [strlen(aName)+1+10];
	char *d=iExportName;
	char *p=aName;
	char *e=p+strlen(aName);
	while (p<e)
		{
		if (*p=='.')
			break;
		*d++=*p++;
		}
	sprintf(d, "[%08x]", (unsigned int)iHeader->iUid3);
	d+=10;
	while (p<=e)
		*d++=*p++;
	}
