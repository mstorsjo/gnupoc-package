// H_MEM.CPP
//
// Copyright (c) 1995-1999 Symbian Ltd.  All rights reserved.
//

#include <stdlib.h>   
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <stdio.h>
#include <memory.h>
#include <e32std.h>
#include <e32def.h>
#include "h_utl.h"

TAny *HMem::Alloc(TAny * const aBaseAddress,const TUint32 aImageSize)
	{
	if (aBaseAddress != 0)
		{
		Print(EError, "Allocating at a specified address is not supported on this system.\n");
		return 0;
		}

	TAny *address = malloc(aImageSize);
	return (address);
	}

void HMem::Free(TAny * const aMem)
	{
	free(aMem);
	}

void HMem::Copy(TAny * const aDestAddr,const TAny * const aSourceAddr,const TUint32 aLength)
	{
	memcpy(aDestAddr,aSourceAddr,(size_t)aLength);
	}

void HMem::Set(TAny * const aDestAddr, const TUint8 aFillChar, const TUint32 aLength)
	{		
	memset(aDestAddr, aFillChar, aLength);
	}

void HMem::FillZ(TAny * const aDestAddr, const TUint32 aLength)
	{
	memset(aDestAddr, 0, aLength);
	}

TUint HMem::CheckSum(TUint *aPtr, TInt aSize)
	{
	TUint sum=0;
	aSize/=4;
	while (aSize-->0)
		sum+=*aPtr++;
	return sum;
	}

TUint HMem::CheckSum8(TUint8 *aPtr, TInt aSize)
	{
	TUint sum=0;
	while (aSize-->0)
		sum+=*aPtr++;
	return sum;
	}

TUint HMem::CheckSumOdd8(TUint8 *aPtr, TInt aSize)
	{
	return CheckSumEven8(aPtr+2, aSize-2);
	}

TUint HMem::CheckSumEven8(TUint8 *aPtr, TInt aSize)
	{
	TUint sum=0;
	while (aSize>0)
		{
		sum+=(TUint)aPtr[0]+aPtr[1];
		aPtr+=4;
		aSize-=4;
		}
	return sum;
	}
