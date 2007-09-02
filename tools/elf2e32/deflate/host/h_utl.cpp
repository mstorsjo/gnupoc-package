// H_UTL.CPP
//
// Copyright (c) 1995-2004 Symbian Software Ltd.  All rights reserved.
//


#define __INCLUDE_CAPABILITY_NAMES__

#ifdef __MSVCDOTNET__
#include <string>
#else //!__MSVCDOTNET__
#include <string.h>
#endif //__MSVCDOTNET__

#include <stdarg.h>
#include <stdlib.h>

#include <e32std.h>

#include "h_utl.h"

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
	case EDiagnostic:
		cerr << "DIAGNOSTIC MESSAGE: " << iText;
		iLogFile << "DIAGNOSTIC MESSAGE: "<<iText;
		break;
	default:
		cerr << "ERROR: Invalid print type" << endl;
		r=KErrGeneral;
		}
	cout.flush();
	iLogFile.flush();
	return r;
    }
/*
TVersion::TVersion()
	{}
TVersion::TVersion(TInt aMajor, TInt aMinor, TInt aBuild)
	: iMajor((TInt8)aMajor), iMinor((TInt8)aMinor), iBuild((TInt16)aBuild)
	{}

istrstream &operator>>(istrstream &is, TVersion &aVersion)
//
// Input a TVersion with syntax: major[.minor][(build)]
//	
	{

	char *str=is.str();
	TInt build=0;
	memset(&aVersion, sizeof(TVersion), 0);	
	TInt i;
	TInt len=strlen(str);
	for (i=0; i<len; i++)
		if (str[i]=='(')
			break;
	if (i<len)
		build=atoi(str+i+1);
	aVersion.iMajor = (TInt8)Min(KMaxTInt8, atoi(str));
	int majorV = atoi(str);
	// iMajor is defined as TInt8 so it should not be bigger than 127
	if (majorV > 127)
		{ 
		cout << "\n Warning: major version must be in range 0 - 127 \n";
		}
	char* pMinor = strchr(str, '.');
	if (pMinor)
		{
		pMinor++; 
		aVersion.iMinor = (TInt8)Min(KMaxTInt8, atoi(pMinor));
		int minorV = atoi(pMinor);
		// iMinor is defined as TInt8 so it should not be bigger than 127
		if (minorV > 127)
			{ 
			cout << "\n Warning: minor version must be in range 0 - 127 \n";
			}
		}	
	aVersion.iBuild=(TInt16)build;
	return is;
	}

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


#define KHoursToMicroSeconds	Int64(3600000000)
#define KDaysToMicroSeconds		(Int64(24)*KHoursToMicroSeconds)
const TInt KMinutesToMicroSeconds = 60000000;
const TInt KSecondsToMicroSeconds =  1000000;

const TInt8 mTab[2][12]=
    {
    {31,28,31,30,31,30,31,31,30,31,30,31}, // 28 days in Feb
    {31,29,31,30,31,30,31,31,30,31,30,31}  // 29 days in Feb
    };

TInt Time::LeapYearsUpTo(TInt aYear)
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

TBool Time::IsLeapYear(TInt aYear)
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


Int64 ConvertTime(TInt aDay, TInt aMonth, TInt aYear, TInt aHour, TInt aMinute, TInt aSecond, TInt aMilliSeconds)
//
// converts TDateTime into a TTime, doesnt check for overflows
//
	{
	
	TInt days=365*aYear+Time::LeapYearsUpTo(aYear);
	TBool isleap=Time::IsLeapYear(aYear);
	for (TInt ii=0; ii<aMonth; ii++)
	    days+=(mTab[isleap][ii]);	
	days+=aDay;
	TInt sum=aMilliSeconds+aSecond*KSecondsToMicroSeconds;
	return(Int64(days)*KDaysToMicroSeconds+Int64(aHour)*KHoursToMicroSeconds
	      +Int64(KMinutesToMicroSeconds)*Int64(aMinute)+Int64(sum));
	}

TInt StringToTime(Int64 &aTime, char *aString)
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
	istrstream val(aString,strlen(aString));
	val >> dec >> day; // locks istrstream in decimal mode for further extractions
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

TBool IsBracketedHex(const char* s, const char* brackets, TInt digits, TUint32& aValue)
	{
	if (s[0]!=brackets[0] || s[1+digits]!=brackets[1])
		return EFalse;
	TInt i;
	TUint32 x = 0;
	for (i=1; i<=digits; ++i)
		{
		TInt c = s[i];
		if (c>='a' && c<='z') c-=32;
		if (c<'0' || (c>'9' && c<'A') || c>'F')
			return EFalse;
		c-='0';
		if (c>9) c-=7;
		x = (x<<4) | (TUint32)c;
		}
	aValue = x;
	return ETrue;
	}

TInt CheckForDecimalVersion(const char* begin, const char* s, TUint32& aValue)
	{
	aValue = 0;
	if (s <= begin || *s != '}')
		return 0;
	TUint32 v[2] = {0,0};
	TUint32 m = 1;
	TInt pos = 0;
	const char* s0 = s + 1;
	for (--s; s >= begin; --s)
		{
		int c = *s;
		if (c >= '0' && c <= '9')
			{
			v[pos] += m * (c - '0');
			if (v[pos] >= 65536u)
				return 0;
			m *= 10;
			}
		else if (c == '.')
			{
			m = 1;
			if (++pos >= 2)
				return 0;
			}
		else if (c == '{')
			break;
		else
			return 0;
		}
	if (s < begin)
		return 0;
	aValue = (v[1] << 16) | v[0];
	return s0 - s;
	}

// Decompose a name of the form NAME{MMMMmmmm}[UUUUUUUU].EXT where the bracketed
// sections and extension are both optional.
// Return a newly malloc-ed string containing NAME.EXT
// Set aUid = 0xUUUUUUUU if present, 0 if not
// Set aModuleVersion = 0xMMMMmmmm if present, 0 if not
// Set aFlags according to which of these are present
char* SplitFileName(const char* aName, TUint32& aUid, TUint32& aModuleVersion, TUint32& aFlags)
	{
	TFileNameInfo f(aName, ETrue);
	aUid = f.iUid3;
	aModuleVersion = f.iModuleVersion;
	aFlags = f.iFlags;
	TInt nl = f.iBaseLength;
	TInt el = f.iTotalLength - f.iExtPos;
	TInt tl = nl + el;
	char* t = (char*)malloc(tl + 1);
	if (t)
		{
		memcpy(t, aName, nl);
		if (el)
			memcpy(t + nl, aName + f.iExtPos, el);
		t[tl] = 0;
		}
	return t;
	}


// Decompose a name of the form NAME{MMMMmmmm}.EXT where the bracketed
// sections and extension are both optional.
// Return a newly malloc-ed string containing NAME.EXT
// Set aModuleVersion = 0xMMMMmmmm if present, 0 if not
// Set aFlags according to whether version present
char* SplitFileName(const char* aName, TUint32& aModuleVersion, TUint32& aFlags)
	{
	TFileNameInfo f(aName, EFalse);
	aModuleVersion = f.iModuleVersion;
	aFlags = f.iFlags;
	TInt nl = f.iBaseLength;
	TInt el = f.iTotalLength - f.iExtPos;
	TInt tl = nl + el;
	char* t = (char*)malloc(tl + 1);
	if (t)
		{
		memcpy(t, aName, nl);
		if (el)
			memcpy(t + nl, aName + f.iExtPos, el);
		t[tl] = 0;
		}
	return t;
	}


// Parse a filename and convert decimal version number to hex
char* NormaliseFileName(const char* aName)
	{
	TFileNameInfo f(aName, EFalse);
	TInt nl = f.iBaseLength;
	TInt el = f.iTotalLength - f.iExtPos;
	TInt tl = nl + el;
	if (f.iFlags & EVerPresent)
		tl += 10;
	char* t = (char*)malloc(tl + 1);
	if (t)
		{
		memcpy(t, aName, nl);
		if (f.iFlags & EVerPresent)
			sprintf(t + nl, "{%08x}%s", f.iModuleVersion, aName + f.iExtPos);
		else if (el)
			memcpy(t + nl, aName + f.iExtPos, el);
		t[tl] = 0;
		}
	return t;
	}

TFileNameInfo::TFileNameInfo(const char* aFileName, TBool aLookForUid)
	{
	iFileName = aFileName;
	TInt l = strlen(aFileName);
	iTotalLength = l;
	TInt remain = l;
	iFlags = 0;
	iUid3 = 0;
	iModuleVersion = 0;
	iBaseLength = l;
	iExtPos = l;
	const char* s = iFileName + l;
	for (; s>=iFileName && *s!='.' && *s!='}' && (!aLookForUid || *s!=']'); --s) {}
	if (s<iFileName)
		return;
	if (*s == '.')
		{
		iExtPos = s - iFileName;
		if (iExtPos == 0)
			{
			iBaseLength = 0;
			return;
			}
		remain = iExtPos;
		--s;
		}
	else if (s != iFileName + l)
		return;
	if (aLookForUid && remain>=10 && IsBracketedHex(s-9, "[]", 8, iUid3))
		{
		iFlags |= EUidPresent;
		remain -= 10;
		s -= 10;
		}
	if (remain>=10 && IsBracketedHex(s-9, "{}", 8, iModuleVersion))
		{
		iFlags |= EVerPresent;
		remain -= 10;
		s -= 10;
		}
	else
		{
		TInt n = CheckForDecimalVersion(iFileName, s, iModuleVersion);
		if (n>0)
			{
			iFlags |= EVerPresent;
			remain -= n;
			s -= n;
			}
		}
	iBaseLength = remain;
	}


#define PARSE_CAPABILITIES_ERROR(aMessage) Print(EError, "%s\n",aMessage)
#define PARSE_CAPABILITIES_ERROR2(aMessage1,aMessage2) Print(EError, "%s%s\n",aMessage1,aMessage2)

TInt ParseCapabilitiesArg(SCapabilitySet& aCapabilities, const char *aText)
//
// This is a cun'n'paste copy of the function in BASE\WINS\SPECIFIC\PROPERTY.CPP
// Keep both of these versions up to date with each other
//
	{
	memset(&aCapabilities,0,sizeof(aCapabilities));
	char c;
	while((c=*aText)!=0)
		{
		if(c<=' ')
			{
			++aText;
			continue;
			}
		int invert=0;
		if(c=='+')
			{
			++aText;
			c=*aText;
			}
		if(c=='-')
			{
			invert=1;
			++aText;
			}
		const char* name = aText;
		while((c=*aText)>' ')
			{
			if(c=='-' || c=='+')
				break;
			++aText;
			}
		TInt n = aText-name;
		TInt i;

		if(n==3 && strnicmp("all",name,n)==0)
			{
			if(invert)
				{
				PARSE_CAPABILITIES_ERROR("Capability '-ALL' not allowed");
				return KErrArgument;
				}
			for(i=0; i<ECapability_Limit; i++)
				{
				if(CapabilityNames[i])
					aCapabilities[i>>5] |= (1<<(i&31));
				}
			continue;
			}

		if(n==4 && strnicmp("none",name,n)==0)
			{
			if(invert)
				{
				PARSE_CAPABILITIES_ERROR("Capability '-NONE' not allowed");
				return KErrArgument;
				}
			memset(&aCapabilities,0,sizeof(aCapabilities));
			continue;
			}

		for(i=0; i<ECapability_Limit; i++)
			{
			const char* cap = CapabilityNames[i];
			if(!cap)
				continue;
			if((int)strlen(cap)!=n)
				continue;
			if(strnicmp(cap,name,n)!=0)
				continue;
			break;
			}
		if(i>=ECapability_Limit)
			{
			char badName[32];
			if(n>=sizeof(badName)) n=sizeof(badName)-1;
			memcpy(badName,name,n);
			badName[n]=0;
			PARSE_CAPABILITIES_ERROR2("Unrecognised capability name: ",badName);
			return KErrArgument;
			}
		if(invert)
			aCapabilities[i>>5] &= ~(1<<(i&31));
		else
			aCapabilities[i>>5] |= (1<<(i&31));
		}
	return KErrNone;
	}

TInt ParseBoolArg(TBool& aValue, const char *aText)
	{
	if (_stricmp(aText, "on")==0 || _stricmp(aText, "yes")==0 || _stricmp(aText, "1")==0 || strlen(aText)==0)
		{
		aValue = ETrue;
		return KErrNone;
		}
	if (_stricmp(aText, "off")==0 || _stricmp(aText, "no")==0 || _stricmp(aText, "0")==0 )
		{
		aValue = EFalse;
		return KErrNone;
		}
	Print(EError, "Expected a boolean on/off value but found %s\n",aText);
	return KErrArgument;
	}
*/
