//
// e32def.h
//
#include <stdint.h>

typedef void TAny;
/*
typedef signed char TInt8;
typedef unsigned char TUint8;
typedef short int TInt16;
typedef unsigned short int TUint16;
typedef long int TInt32;
typedef unsigned long int TUint32;
*/
typedef int8_t TInt8;
typedef uint8_t TUint8;
typedef int16_t TInt16;
typedef uint16_t TUint16;
typedef int32_t TInt32;
typedef uint32_t TUint32;

typedef signed int TInt;
typedef unsigned int TUint;
typedef double TReal;
typedef int TBool;


typedef unsigned char TText;

typedef TUint32 TLinAddr;

#define TRUE 1
#define FALSE 0

#define IMPORT_C
#define EXPORT_C
