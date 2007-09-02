//
// e32std.h
//
#ifndef E32STD_H__
#define E32STD_H__

#include <e32def.h>


enum TFalse {EFalse=FALSE};
enum TTrue {ETrue=TRUE};

const TInt KErrNone=0;
const TInt KErrNotFound=(-1);
const TInt KErrGeneral=(-2);
//const TInt KErrCancel=(-3);
const TInt KErrNoMemory=(-4);
//const TInt KErrNotSupported=(-5);
const TInt KErrArgument=(-6);


enum TProcessPriority
	{
	EPriorityLow=150,
	EPriorityBackground=250,
	EPriorityForeground=350,
	EPriorityHigh=450,
	EPriorityWindowServer=650,
	EPriorityFileServer=750,
	EPriorityRealTimeServer=850,
	EPrioritySupervisor=950
	};

#define KNullUid TUid::Null()

const TInt KMaxCheckedUid=3;


class TUid
	{
public:
	TInt operator==(const TUid& aUid) const;
	TInt operator!=(const TUid& aUid) const;
	static inline TUid Uid(TInt aUid);
	static inline TUid Null();
public:
	TInt32 iUid;
	};

inline TUid TUid::Uid(TInt aUid)
	{TUid uid={aUid};return uid;}
inline TUid TUid::Null()
	{TUid uid={0};return uid;}


class TUidType
	{
public:
	TUidType();
	TUidType(TUid aUid1);
	TUidType(TUid aUid1,TUid aUid2);
	TUidType(TUid aUid1,TUid aUid2,TUid aUid3);
	TInt operator==(const TUidType& aUidType) const;
	TInt operator!=(const TUidType& aUidType) const;
	const TUid& operator[](TInt anIndex) const;
	TUid MostDerived() const;
	TBool IsPresent(TUid aUid) const;
	TBool IsValid() const;
private:
	TUid iUid[KMaxCheckedUid];
	};

class TCheckedUid
	{
public:
	TCheckedUid();
	TCheckedUid(const TUidType& aUidType);
	//	TCheckedUid(const TDesC8& aPtr);
	void Set(const TUidType& aUidType);
	//	void Set(const TDesC8& aPtr);
	//	TPtrC8 Des() const;
	//	inline const TUidType& UidType() const;
protected:
	TUint Check() const;
private:
	TUidType iType;
	TUint iCheck;
	};

class TVersion
	{
public:
	TVersion();
	TVersion(TInt aMajor,TInt aMinor,TInt aBuild);
	//	TVersionName Name();
public:
	TInt8 iMajor;
	TInt8 iMinor;
	TInt16 iBuild;
	};

class TInt64
	{
public:
	inline TInt64();
	TInt64(TInt aVal);
	inline TInt64(TUint aVal);
	inline TInt64(TUint aHigh,TUint aLow);
	TInt64(TReal aVal);
	inline void Set(TUint aHigh,TUint aLow);
	inline TUint Low() const;
	inline TUint High() const;
	TInt GetTInt() const;
	TReal GetTReal() const;
	TInt64 &operator=(TInt aVal);
	inline TInt64& operator=(TUint aVal);
	TInt64& operator=(TReal aVal);
	TInt64& operator+=(const TInt64 &aVal);
	TInt64& operator-=(const TInt64 &aVal);
	TInt64& operator*=(const TInt64 &aVal);
	TInt64& operator/=(const TInt64 &aVal);
	TInt64& operator%=(const TInt64 &aVal);
	TInt64& operator>>=(TInt aShift);
	TInt64& operator<<=(TInt aShift);
	TInt64 operator+() const;
	TInt64 operator-() const;
	TInt64& operator++();
	TInt64 operator++(TInt);
	TInt64& operator--();
	TInt64 operator--(TInt);
	TInt64 operator+(const TInt64 &aVal) const;
	TInt64 operator-(const TInt64 &aVal) const;
	TInt64 operator*(const TInt64 &aVal) const;
	TInt64 operator/(const TInt64 &aVal) const;
	TInt64 operator%(const TInt64 &aVal) const;
	TInt64 operator>>(TInt aShift) const;
	TInt64 operator<<(TInt aShift) const;
	void Lsr(TInt aShift);
	void Mul10();
	TInt MulTop(const TInt64 &aVal);
	void DivMod(const TInt64 &aVal,TInt64 &aRemainder);
	TInt operator==(const TInt64 &aVal) const;
	TInt operator!=(const TInt64 &aVal) const;
	TInt operator>=(const TInt64 &aVal) const;
	TInt operator<=(const TInt64 &aVal) const;
	TInt operator>(const TInt64 &aVal) const;
	TInt operator<(const TInt64 &aVal) const;
protected:
	TUint iLow;
	TUint iHigh;
	};


inline TInt64::TInt64()
	{}
inline TInt64::TInt64(TUint aVal)
	: iLow(aVal),iHigh(0)
	{}
inline TInt64::TInt64(TUint aHigh,TUint aLow)
	: iLow(aLow),iHigh(aHigh)
	{}
inline TUint TInt64::Low() const
	{return(iLow);}
inline TUint TInt64::High() const
	{return(iHigh);}


template<class T> T* PtrAdd(T* ptr, int val) { return (T*) (((uint8_t*)ptr) + val); } 

#define _FOFF(type, field) ((int)&(((type*)NULL)->field))

template<class T> T Min(T a, T b) { if (a < b) return a; return b; }

class User {
public:
	static void Invariant();
};


#define __ASSERT_ALWAYS(x, y) do { if (!(x)) { y; } } while (0)
#define ASSERT(x) do { if (!(x)) { fprintf(stderr, "Assertion failed at %s:%d\n", __FILE__, __LINE__); } } while (0)


#endif
