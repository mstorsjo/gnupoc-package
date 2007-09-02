// H_UTL.H
//
// Copyright (c) 1995-1999 Symbian Ltd.  All rights reserved.
//

#if !defined(__H_UTL_H__)
#define __H_UTL_H__
//
#include <stdio.h>
#include <iostream.h>
#include <sstream>
#include <fstream.h>
#include <e32std.h>

#define ALIGN4K(a) ((a+0xfff)&0xfffff000)
#define ALIGN4(a) ((a+0x3)&0xfffffffc)

using namespace std;

#ifdef HEAPCHK
#define NOIMAGE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
void HeapCheck();
#endif
#define Print H.PrintString
//
const TInt KMaxStringLength=0x100;
//
class HFile
	{
public:
	static TBool Open(const TText * const aFileName, TInt32 * const aFileHandle);
	static TBool Read(const TInt32 aFileHandle, TAny * const aBuffer, const TUint32 aCount);
	static TBool Seek(const TInt32 aFileHandle, const TUint32 aOffset);
	static TUint32 GetPos(const TInt32 aFileHandle);
	static TAny Close(const TInt32 aFileHandle);
	static TUint32 GetLength(TText *aName);
	static TUint32 Read(TText *aName, TAny *someMem);
	};
//
class HMem
	{
public:
	static TAny *Alloc(TAny * const aBaseAddress,const TUint32 aImageSize);
	static void Free(TAny * const aMem);
	static void Copy(TAny * const aDestAddr,const TAny * const aSourceAddr,const TUint32 aLength);
	static void Set(TAny * const aDestAddr, const TUint8 aFillChar, const TUint32 aLength);
	static void FillZ(TAny * const aDestAddr, const TUint32 aLength);

	static TUint CheckSum(TUint *aPtr, TInt aSize);
	static TUint CheckSum8(TUint8 *aPtr, TInt aSize);
	static TUint CheckSumOdd8(TUint8 *aPtr, TInt aSize);
	static TUint CheckSumEven8(TUint8 *aPtr, TInt aSize);
	};
//
enum TPrintType {EAlways, EScreen, ELog, EWarning, EError, EPeError, ESevereError};
//
class HPrint
	{
public:
	~HPrint();
	void SetLogFile(TText *aFileName);
	TInt PrintString(TPrintType aType,const char *aFmt,...);
public:
	TText iText[KMaxStringLength];
	TBool iVerbose;
private:
	ofstream iLogFile;
	};
//
extern HPrint H;
extern TBool PVerbose;
//
// TAny *operator new(TUint aSize);
void operator delete(TAny *aPtr);
//
stringstream &operator>>(stringstream &is, TVersion &aVersion);
//
TInt StringToTime(TInt64 &aTime, char *aString);

void ByteSwap(TUint &aVal);
void ByteSwap(TUint16 &aVal);
void ByteSwap(TUint *aPtr, TInt aSize);




/**
 Convert string to number.
*/
template <class T>
TInt Val(T& aVal, char* aStr)
	{

	T x;
	stringstream val(aStr);
	val >> x;
	if (!val.eof() || val.fail())
		return KErrGeneral;
	aVal=x;
	return KErrNone;
	}


#endif
