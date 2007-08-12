// h_endian.h
// Copyright (c) 2004 P. A. Bagyenda <bagyenda@dsmagic.com>
//
// Defines some macros for Endian switching of structures and types in instform.h
// from symbian makesis package
#ifndef __H_ENDIAN_INCLUDED__
#define __H_ENDIAN_INCLUDED__

#include <assert.h>

// ===========================================================================
// Macros for endian switch
// ===========================================================================

#ifdef __BIG_ENDIAN__
// byte re-order of  int4/uint4
#define flipi(x) do { \
assert(sizeof (x) == 4); \
unsigned char *__px = (unsigned char *)&(x); \
unsigned char __x = __px[0]; __px[0] = __px[3]; __px[3] = __x; \
__x = __px[1]; __px[1] = __px[2]; __px[2] = __x; \
} while(0)

// byte re-order of short/ushort
#define flips(x) do { \
assert(sizeof (x) == 2); \
unsigned char *__px = (unsigned char *)&(x); \
unsigned char __x = __px[0]; __px[0] = __px[1]; __px[1] = __x; \
} while(0)
	

#define vflipi(x) \
  ((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
                     (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
                     (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
                     (((unsigned long int)(x) & 0xff000000U) >> 24)))
 
#define vflips(x) \
((unsigned short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
                      (((unsigned short int)(x) & 0xff00) >> 8))) \


// Macros for simple arrays
#define pflipi(a,n) do {\
          for (unsigned long __j = 0; __j < (n); __j++) flipi((a)[__j]); \
          } while(0)

#define pflips(a,n) do {\
          for (unsigned long __j = 0; __j < (n); __j++) flips((a)[__j]); \
          } while(0)


#define flipIMAGE_DOS_HEADER(x) do {\
	flips((x).e_magic); \
	flips((x).e_cblp); \
	flips((x).e_cp); \
	flips((x).e_crlc); \
	flips((x).e_cparhdr); \
	flips((x).e_minalloc); \
	flips((x).e_maxalloc); \
	flips((x).e_ss); \
	flips((x).e_sp); \
	flips((x).e_csum); \
	flips((x).e_ip); \
	flips((x).e_cs); \
	flips((x).e_lfarlc); \
	flips((x).e_ovno); \
	pflips((x).e_res,4); \
	flips((x).e_oemid); \
	flips((x).e_oeminfo); \
	pflips((x).e_res2,10); \
	flipi((x).e_lfanew); \
	} while(0)

#define flipIMAGE_OS2_HEADER(x) do {\
	flips((x).ne_magic); \
	flips((x).ne_enttab); \
	flips((x).ne_cbenttab); \
	flipi((x).ne_crc); \
	flips((x).ne_flags); \
	flips((x).ne_autodata); \
	flips((x).ne_heap); \
	flips((x).ne_stack); \
	flipi((x).ne_csip); \
	flipi((x).ne_sssp); \
	flips((x).ne_cseg); \
	flips((x).ne_cmod); \
	flips((x).ne_cbnrestab); \
	flips((x).ne_segtab); \
	flips((x).ne_rsrctab); \
	flips((x).ne_restab); \
	flips((x).ne_modtab); \
	flips((x).ne_imptab); \
	flipi((x).ne_nrestab); \
	flips((x).ne_cmovent); \
	flips((x).ne_align); \
	flips((x).ne_cres); \
	flips((x).ne_pretthunks); \
	flips((x).ne_psegrefTUint8); \
	flips((x).ne_swaparea); \
	flips((x).ne_expver); \
	} while(0)

#define flipIMAGE_FILE_HEADER(x) do {\
	flips((x).Machine); \
	flips((x).NumberOfSections); \
	flipi((x).TimeDateStamp); \
	flipi((x).PointerToSymbolTable); \
	flipi((x).NumberOfSymbols); \
	flips((x).SizeOfOptionalHeader); \
	flips((x).Characteristics); \
	} while(0)

#define flipIMAGE_DATA_DIRECTORY(x) do {\
	flipi((x).VirtualAddress); \
	flipi((x).Size); \
	} while(0)

#define flipIMAGE_OPTIONAL_HEADER(x) do {\
	flips((x).Magic); \
	flipi((x).SizeOfCode); \
	flipi((x).SizeOfInitializedData); \
	flipi((x).SizeOfUninitializedData); \
	flipi((x).AddressOfEntryPoint); \
	flipi((x).BaseOfCode); \
	flipi((x).BaseOfData); \
	flipi((x).ImageBase); \
	flipi((x).SectionAlignment); \
	flipi((x).FileAlignment); \
	flips((x).MajorOperatingSystemVersion); \
	flips((x).MinorOperatingSystemVersion); \
	flips((x).MajorImageVersion); \
	flips((x).MinorImageVersion); \
	flips((x).MajorSubsystemVersion); \
	flips((x).MinorSubsystemVersion); \
	flipi((x).Reserved1); \
	flipi((x).SizeOfImage); \
	flipi((x).SizeOfHeaders); \
	flipi((x).CheckSum); \
	flips((x).Subsystem); \
	flips((x).DllCharacteristics); \
	flipi((x).SizeOfStackReserve); \
	flipi((x).SizeOfStackCommit); \
	flipi((x).SizeOfHeapReserve); \
	flipi((x).SizeOfHeapCommit); \
	flipi((x).LoaderFlags); \
	flipi((x).NumberOfRvaAndSizes); \
	for (int __j=0;__j<IMAGE_NUMBEROF_DIRECTORY_ENTRIES;__j++) flipIMAGE_DATA_DIRECTORY((x).DataDirectory[__j]); \
	} while(0)

#define flipIMAGE_ROM_OPTIONAL_HEADER(x) do {\
	flips((x).Magic); \
	flipi((x).SizeOfCode); \
	flipi((x).SizeOfInitializedData); \
	flipi((x).SizeOfUninitializedData); \
	flipi((x).AddressOfEntryPoint); \
	flipi((x).BaseOfCode); \
	flipi((x).BaseOfData); \
	flipi((x).BaseOfBss); \
	flipi((x).GprMask); \
	pflipi((x).CprMask,4); \
	flipi((x).GpValue); \
	} while(0)

#define flipIMAGE_NT_HEADERS(x) do {\
	flipi((x).Signature); \
	flipIMAGE_FILE_HEADER((x).FileHeader); \
	flipIMAGE_OPTIONAL_HEADER((x).OptionalHeader); \
	} while(0)

#define flipIMAGE_ROM_HEADERS(x) do {\
	flipIMAGE_FILE_HEADER((x).FileHeader); \
	flipIMAGE_ROM_OPTIONAL_HEADER((x).OptionalHeader); \
	} while(0)

#define flipIMAGE_SECTION_HEADER(x) do {\
	flipi((x).Misc.PhysicalAddress); \
	flipi((x).VirtualAddress); \
	flipi((x).SizeOfRawData); \
	flipi((x).PointerToRawData); \
	flipi((x).PointerToRelocations); \
	flipi((x).PointerToLinenumbers); \
	flips((x).NumberOfRelocations); \
	flips((x).NumberOfLinenumbers); \
	flipi((x).Characteristics); \
	} while(0)

#define flipIMAGE_SYMBOL(x) do {\
	flipi((x).Value); \
	flips((x).SectionNumber); \
	flips((x).Type); \
	} while(0)

#define flipIMAGE_RELOCATION(x) do {\
	flipi((x).VirtualAddress); \
	flipi((x).SymbolTableIndex); \
	flips((x).Type); \
	} while(0)

#define flipIMAGE_BASE_RELOCATION(x) do {\
	flipi((x).VirtualAddress); \
	flipi((x).SizeOfBlock); \
	} while(0)

// Some fields were pointers, made into integers
#define flipIMAGE_EXPORT_DIRECTORY(x) do {\
	flipi((x).Characteristics); \
	flipi((x).TimeDateStamp); \
	flips((x).MajorVersion); \
	flips((x).MinorVersion); \
	flipi((x).Name); \
	flipi((x).Base); \
	flipi((x).NumberOfFunctions); \
	flipi((x).NumberOfNames); \
        flipi((x).AddressOfFunctions); \
        flipi((x).AddressOfNames); \
        flipi((x).AddressOfNameOrdinals); \
	} while(0)

#define flipIMAGE_IMPORT_BY_NAME(x) do {\
	flips((x).Hint); \
	} while(0)

#define flipIMAGE_THUNK_DATA(x) do {\
	flipi((x).u1.Ordinal); \
	} while(0)

#define flipIMAGE_IMPORT_DESCRIPTOR(x) do {\
	flipi((x).Characteristics); \
	flipi((x).TimeDateStamp); \
	flipi((x).ForwarderChain); \
	flipi((x).Name); \
	} while(0)

#define flipE32ImportBlock(x) do {\
         flipi((x).iOffsetOfDllName); \
         flipi((x).iNumberOfImports); \
         } while(0)

#define flipTVersion(x) do {\
         flips((x).iBuild); \
         } while(0)

#define flipi64(x) do {\
        (x) = TInt64(vflipi((x).High()), vflipi((x).Low())); \
        } while (0)

// Do not flip iSignature field...
#define flipE32ImageHeader(x) do {\
        flipi((x).iUid1); \
        flipi((x).iUid2); \
        flipi((x).iUid3); \
        flipi((x).iCheck); \
        flipi((x).iCpu); \
        flipi((x).iCheckSumCode); \
        flipi((x).iCheckSumData); \
        flipTVersion((x).iVersion); \
        flipi64((x).iTime); \
        flipi((x).iFlags); \
        flipi((x).iCodeSize); \
        flipi((x).iDataSize); \
        flipi((x).iHeapSizeMin); \
        flipi((x).iHeapSizeMax); \
        flipi((x).iStackSize); \
        flipi((x).iBssSize); \
        flipi((x).iEntryPoint); \
        flipi((x).iCodeBase); \
        flipi((x).iDataBase); \
        flipi((x).iDllRefTableCount); \
        flipi((x).iExportDirOffset); \
        flipi((x).iExportDirCount); \
        flipi((x).iTextSize); \
        flipi((x).iCodeOffset); \
        flipi((x).iDataOffset); \
        flipi((x).iImportOffset); \
        flipi((x).iCodeRelocOffset); \
        flipi((x).iDataRelocOffset); \
        flipi((x).iPriority); \
        } while(0)

#else
#define flipi(x) (void)0
#define flips(x) (void)0
#define pflipi(a,n) (void)0
#define pflips(a,n) (void)0

#define vflipi(x) (x)
#define vflips(x) (x)

#define flipIMAGE_DOS_HEADER(x) (void)0
#define flipIMAGE_OS2_HEADER(x) (void)0
#define flipIMAGE_FILE_HEADER(x) (void)0
#define flipIMAGE_DATA_DIRECTORY(x) (void)0
#define flipIMAGE_OPTIONAL_HEADER(x) (void)0
#define flipIMAGE_ROM_OPTIONAL_HEADER(x) (void)0
#define flipIMAGE_NT_HEADERS(x) (void)0
#define flipIMAGE_ROM_HEADERS(x) (void)0
#define flipIMAGE_SECTION_HEADER(x) (void)0
#define flipIMAGE_SYMBOL(x) (void)0
#define flipIMAGE_RELOCATION(x) (void)0
#define flipIMAGE_BASE_RELOCATION(x) (void)0
#define flipIMAGE_EXPORT_DIRECTORY(x) (void)0
#define flipIMAGE_IMPORT_BY_NAME(x) (void)0
#define flipIMAGE_THUNK_DATA(x) (void)0
#define flipIMAGE_IMPORT_DESCRIPTOR(x) (void)0

#define flipE32ImportBlock(x) (void)0

#define flipE32ImageHeader(x) (void)0
#define flipTVersion(x) (void)0
#define flipi64(x) (void)0
#endif

#endif
