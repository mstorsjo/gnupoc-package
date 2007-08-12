// H_UTL.CPP
//
// Copyright (c) 1995-1999 Symbian Ltd.  All rights reserved.
//


#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <e32std.h>
#include "h_utl.h"

using namespace std;

// from u32std.h
class TInt64A : public TInt64
	{
public:
	IMPORT_C void Add(const TInt64 &aVal);
	IMPORT_C void Sub(const TInt64 &aVal);
	IMPORT_C void Mul(const TInt64 &aVal);
	IMPORT_C void MulTop(const TInt64 &aVal);
	IMPORT_C void Div(const TInt64 &aVal,TInt64 *aRemainder);
	IMPORT_C TInt Cmp(const TInt64 &aVal);
	IMPORT_C void Lsr(TInt aShift);
	IMPORT_C void Asr(TInt aShift);
	IMPORT_C void Lsl(TInt aShift);
	IMPORT_C void Neg();
	IMPORT_C void Inc();
	IMPORT_C void Dec();
	};


TBool PVerbose=ETrue;

HPrint H;

HPrint::~HPrint()
	{
	iLogFile.close();
	}

void HPrint::SetLogFile(TText *aFileName)
	{
	iLogFile.open((const char *)aFileName);
	}

TInt HPrint::PrintString(TPrintType aType,const char *aFmt,...)
//
// Print text, noting where to send it.
//
	{

	TInt r=KErrNone;
	va_list list;
	va_start(list,aFmt);
	vsnprintf((char *)iText,KMaxStringLength,aFmt,list);
	va_end(list);
	switch (aType)
		{
	case EAlways:
		cout << iText;
		iLogFile << iText;
		break;
	case EScreen:
		cout << iText;
		break;
	case ELog:
		if (iVerbose)
			cout << iText;
		iLogFile << iText;
		break;
	case EWarning:
		cerr << "WARNING: " << iText;
		iLogFile << "WARNING: "<<iText;
		break;
	case EError:
		cerr << "ERROR: " << iText;
		iLogFile << "ERROR: " << iText;
		r=KErrGeneral;
		break;
	case EPeError:
		if (PVerbose)
			{
			cerr << "ERROR: " << iText;
			iLogFile << "ERROR: " << iText;
			}
		r=KErrGeneral;
		break;
	case ESevereError:
		cerr << "ERROR: " << iText;
		iLogFile << "ERROR: " << iText;
		r=KErrGeneral;
		break;
	default:
		cerr << "ERROR: Invalid print type" << endl;
		r=KErrGeneral;
		}
	cout.flush();
	iLogFile.flush();
	return r;
    }

//EXPORT_C TTime::TTime(TInt64 aTime) : iTime(aTime) {}

/*EXPORT_C TTime &TTime::operator=(TInt64 aTime)
	{

	iTime=aTime;
	return(*this);
	}*/

EXPORT_C TInt64::TInt64(TInt aVal)
//
// Constructor
//
	: iLow(aVal),iHigh(0)
	{

	if (aVal<0)
		iHigh|=0xffffffffu;
	}

/*EXPORT_C TInt64::TInt64(TUint aVal)
//
// Constructor
//
	: iLow(aVal),iHigh(0)
	{}*/

/*EXPORT_C TUint TInt64::High() const
	{
	return iHigh;
	}
*/
/*EXPORT_C TUint TInt64::Low() const
	{
	return iLow;
	}*/

/*EXPORT_C TInt64 &TInt64::operator=(const TInt64 &aVal)
//
// Assignment
//
	{

	iLow=aVal.iLow;
	iHigh=aVal.iHigh;
	return(*this);
	}*/

EXPORT_C TInt64 TInt64::operator*(const TInt64 &aVal) const
//
// Binary *
//
	{

	TInt64 r(*this);
	((TInt64A *)&r)->Mul(aVal);
	return(r);
	}

EXPORT_C TInt64 TInt64::operator+(const TInt64 &aVal) const
//
// Binary +
//
	{

	TInt64 r(*this);
	((TInt64A *)&r)->Add(aVal);
	return(r);
	}

void TInt64A::Mul(const TInt64 &a1)
//
// Multiply
//
	{
	const unsigned long long tmp1 = (unsigned long long)a1.High() << 32 | a1.Low();
	const unsigned long long tmp2 = (unsigned long long)iHigh << 32 | iLow;
	const unsigned long long result = tmp1 * tmp2;
	iHigh = result >> 32;
	iLow = result & 0xffffffff;
	}

void TInt64A::Add(const TInt64 &a1)
//
// Add
//
	{
	const unsigned long long tmp1 = (unsigned long long)a1.High() << 32 | a1.Low();
	const unsigned long long tmp2 = (unsigned long long)iHigh << 32 | iLow;
	const unsigned long long result = tmp1 + tmp2;
	iHigh = result >> 32;
	iLow = result & 0xffffffff;
	}

EXPORT_C TInt TInt64::operator==(const TInt64 &aVal) const
//
// Test for equality
//
	{

	return (aVal.iLow==iLow && aVal.iHigh == iHigh);
	}


TVersion::TVersion()
	{}
TVersion::TVersion(TInt aMajor, TInt aMinor, TInt aBuild)
	: iMajor((TInt8)aMajor), iMinor((TInt8)aMinor), iBuild((TInt16)aBuild)
	{}

TInt Locate(const char *aString, char aChar)
//
// Locate aChar in aString
//
	{

	if (aString==NULL)
		return KErrNotFound;
	TInt i=0;
	while (*aString!=0)
		{
		if (*aString==aChar)
			return i;
		aString++;
		i++;
		}
	return KErrNotFound;
	}


#define KDaysToMicroSeconds TInt64(20,500654080U)
#define KHoursToMicroSeconds TInt64(0,3600000000U)
const TUint KMinutesToMicroSeconds = 60000000;
const TUint KSecondsToMicroSeconds =  1000000;

const TInt8 mTab[2][12]=
    {
    {31,28,31,30,31,30,31,31,30,31,30,31}, // 28 days in Feb
    {31,29,31,30,31,30,31,31,30,31,30,31}  // 29 days in Feb
    };

static TInt LeapYearsUpTo(TInt aYear)
//
// from 0AD to present year according to the rule above
//
	{

	if (aYear<=0)
		return(aYear/4);
	if (aYear<=1600)
		return(1+((aYear-1)/4));
	TInt num=401; // 1600/4+1
	aYear-=1601;
	num+=(aYear/4-aYear/100+aYear/400);
	return(num);
	}

static TBool IsLeapYear(TInt aYear)
//
// up to and including 1600 leap years were every 4 years,since then leap years are every 4 years unless
// the year falls on a century which is not divisible by 4 (ie 1900 wasnt,2000 will be)
// for simplicity define year 0 as a leap year
//
	{

	if (aYear>1600)
    	return(!(aYear%4) && (aYear%100 || !(aYear%400)));
	return(!(aYear%4));
	}


TInt64 ConvertTime(TInt aDay, TInt aMonth, TInt aYear, TInt aHour, TInt aMinute, TInt aSecond, TInt aMilliSeconds)
//
// converts TDateTime into a TTime, doesnt check for overflows
//
	{
	
	TInt days=365*aYear+LeapYearsUpTo(aYear);
	TBool isleap=IsLeapYear(aYear);
	for (TInt ii=0; ii<aMonth; ii++)
	    days+=(mTab[isleap][ii]);	
	days+=aDay;
	TUint sum=aMilliSeconds+aSecond*KSecondsToMicroSeconds;
	return(TInt64(days)*KDaysToMicroSeconds+
	       TInt64(aHour)*KHoursToMicroSeconds+
	      TInt64(KMinutesToMicroSeconds)*TInt64(aMinute)+TInt64(sum));
	}

TInt StringToTime(TInt64 &aTime, char *aString)
//
// Convert string to time. String is in the format:
//
// dd/mm/yyyy hh:mm:ss.mmmmmmm
//
	{

	TInt day=1;
	TInt month=1;
	TInt year=1997;
	TInt hour=10;
	TInt minute=10;
	TInt sec=0;
	TInt mill=0;
	char ch;
	stringstream val(aString);
	val >> day; // locks istrstream in decimal mode for further extractions
	val >> ch;
	if (ch!='/')
		return KErrGeneral;
	val >> month;
	val >> ch;
	if (ch!='/')
		return KErrGeneral;
	val >> year;
	val >> ch;

	if (ch=='_')
		{
		// time too.
		val >> hour;
		val >> ch;
		if (ch!=':')
			return KErrGeneral;
		val >> minute;
		val >> ch;
		if (ch!=':')
			return KErrGeneral;
		val >> sec;
		val >> ch;
		if (ch=='.')
			{
			val >> mill;
			}
		}

	if (day<1 || day>31)
		return KErrArgument;
	if (month<1 || month>12)
		return KErrArgument;
	if (year<1970 || year>2060)
		return KErrArgument;
	if (hour<0 || hour>23)
		return KErrArgument;
	if (minute<0 || minute>59)
		return KErrArgument;
	if (sec<0 || sec>59)
		return KErrArgument;
	if (mill<0 || mill>999999)
		return KErrArgument;

	aTime=ConvertTime(day-1, month-1, year, hour, minute, sec, mill);
	return KErrNone;
	}

void ByteSwap(TUint &aVal)
	{
	TUint t0=aVal & 0xff;
	TUint t1=(aVal>>8)  & 0xff;
	TUint t2=(aVal>>16) & 0xff;
	TUint t3=aVal>>24;
	aVal=(t0 << 24) | (t1 << 16) | (t2 << 8) | (t3);
	}

void ByteSwap(TUint16 &aVal)
	{
	TUint16 t0=(TUint16)((aVal >> 8) & 0xff);
	TUint16 t1=(TUint16)(aVal & 0xff);
	aVal=(TUint16)((t1 << 8) | t0);
	}

void ByteSwap(TUint *aPtr, TInt aSize)
	{

	while ((aSize-=4)>=0)
		ByteSwap(*aPtr++);
	}

