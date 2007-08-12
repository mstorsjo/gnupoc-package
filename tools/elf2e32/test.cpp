#include <e32base.h>

EXPORT_C void fooBar() {
}

static TUint8 myStaticData[42];
static TUint8 myStaticInitializedData[] = { 1, 2, 3, 4, 5 };
extern TUint8 myExternInitializedData[];
TUint8 myExternInitializedData[] = { 6, 7, 8, 9, 10 };
extern TUint8 myExternData[];
TUint8 myExternData[42];

GLDEF_C TInt E32Main() {
	TUint8 localArray[27];
	TUint8 localInitializedArray[] = { 27, 28, 39 };
	myStaticInitializedData[0] = myStaticInitializedData[3];
	myStaticData[0] = myStaticData[3];
	User::Leave(42);
	return 0;
}

