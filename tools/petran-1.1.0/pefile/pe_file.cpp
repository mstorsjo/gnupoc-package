// PE_FILE.CPP
//
// Copyright (c) 1995-1999 Symbian Ltd.  All rights reserved.
//


#include <string.h>
#include <e32std.h>
#include <e32rom.h>
#include <e32image.h>
#include "pe_defs.h"
#include "pe_file.h"
#include "h_utl.h"
#include "h_endian.h"

TBool hadText, hadReloc = EFalse;
TUint32 PEFile::iRomMemBase=0;
TUint32 PEFile::iRomLinearBase=0;

PEFile::PEFile()
	:iMemBase(0),iEntryPoint(0),iImageSize(0),iCodeSize(0),iDataSize(0),
	iHeapReservedSize(0),iHeapCommittedSize(0),iStackReservedSize(0),iStackCommittedSize(0),
	iBssSize(0),iBssOffset(0),iSectionAlign(0),iExpDirectoryOffset(0),iDataOffset(0),
	iImageIsDll(EFalse),
	iHeader(0),iExpDirectory(0),iImpDescriptor(0),iFileName(0),iFileHandle(0),
	iLinkedBase(0),iStartOfHeaders(0),iSizeOfHeaders(0),iNumSections(0),
	iRomRunAddr(0),iRamRunAddr(0),iRomDelta(0),iRamDelta(0),iHadDataSection(EFalse),
	iBssSectionLinkedAddr(0),iBssSectionAddr(0),iBssSectionSize(0),
	iDataSectionLinkedAddr(0),iDataSectionAddr(0),iDataSectionSize(0),
	iCodeSectionAddr(0),iCodeSectionSize(0),
	iRDataSectionAddr(0),iRDataSectionSize(0),
	iCRTSectionAddr(0),iCRTSectionSize(0),
	iExportDataDir(0)
//
// Constructor
//
	{

	for (TInt i=0; i<KNumberOfSections; i++)
		{
		iSectionHeader[i]=NULL;
		iSectionData[i]=NULL;
		}
	}


PEFile::~PEFile()
//
// Destructor
//
	{

	delete [] iFileName;
	for (TInt i=0; i<KNumberOfSections; i++)
		{
		delete iSectionHeader[i];
		delete iSectionData[i];
		}
	}


TBool PEFile::Init(const TText * const aFileName)
//
// Reads the PE headers to fill in lots of nice instance variables with info about the file
//
 	{

 	delete [] iFileName;	
	iFileName = new TText[strlen((const char *)aFileName)+1];
	strcpy ((char *)iFileName, (const char *)aFileName);

	iHeader = (PIMAGE_NT_HEADERS)(HMem::Alloc(0,sizeof(IMAGE_DOS_HEADER)+sizeof(IMAGE_NT_HEADERS)));
	if (!iHeader)
		{
		Print(EPeError,"Failed to allocate memory for headers.\n");
		return EFalse;
		}

	TInt error = HFile::Open(iFileName, &iFileHandle);
	if (error!=0)
		return EFalse;

	if (!HFile::Read(iFileHandle,iHeader,sizeof(IMAGE_DOS_HEADER)))
		{
		Print(EPeError,"Unable to read file %s.\n",iFileName);
		HFile::Close(iFileHandle);
		return EFalse;
		}

	flipIMAGE_DOS_HEADER(*(PIMAGE_DOS_HEADER)iHeader); // Re order it.

	if (IsValidDOSHeader((PIMAGE_DOS_HEADER)iHeader)) // read in the rest, overwriting the DOS header
		iStartOfHeaders = ((PIMAGE_DOS_HEADER)iHeader)->e_lfanew;
	else
		iStartOfHeaders = 0;

	if (!HFile::Seek(iFileHandle, iStartOfHeaders))
		{
		Print(EPeError,"File %s is not large enough to contain valid headers.\n",iFileName);
		HFile::Close(iFileHandle);
		return EFalse;
		}

 	if (!HFile::Read(iFileHandle,iHeader,sizeof(IMAGE_NT_HEADERS)))
		{
		Print(EPeError,"Unable to read NT headers.\n");
		HFile::Close(iFileHandle);
		return EFalse;
		}
	
	flipIMAGE_NT_HEADERS(*iHeader); // Re order it.
	if (!IsValidNTHeader(iHeader))
		{
		Print(EPeError,"Invalid NT header.\n");
		HFile::Close(iFileHandle);
		return EFalse;
		}

	if (!(IsValidFileHeader((PIMAGE_FILE_HEADER)&iHeader->FileHeader)))
		{
		Print(EPeError,"Invalid file header.\n");
		HFile::Close(iFileHandle);
		return EFalse;
		}

//	PIMAGE_NT_HEADERS pNTHeader = iHeader;
	PIMAGE_FILE_HEADER pFileHeader = (PIMAGE_FILE_HEADER)&iHeader->FileHeader;
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&iHeader->OptionalHeader;
//	PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)(iHeader+1);

	iImageSize = pOptionalHeader->SizeOfImage;
	iCodeSize = pOptionalHeader->SizeOfCode;
	iDataSize = pOptionalHeader->SizeOfInitializedData;
	iEntryPoint = pOptionalHeader->AddressOfEntryPoint;
	iHeapReservedSize = pOptionalHeader->SizeOfHeapReserve;
	iHeapCommittedSize = pOptionalHeader->SizeOfHeapCommit;
	iStackReservedSize = 0x2000;
	iStackCommittedSize = 0x2000;
	iBssSize = pOptionalHeader->SizeOfUninitializedData;
	iSectionAlign =	pOptionalHeader->SectionAlignment;
 	if (pFileHeader->Characteristics & IMAGE_FILE_DLL)
		iImageIsDll = ETrue;
	else
		iImageIsDll = EFalse;
 	iLinkedBase=pOptionalHeader->ImageBase;
	iNumSections = pFileHeader->NumberOfSections;
	iSizeOfHeaders = pOptionalHeader->SizeOfHeaders;
	iExportDataDir=pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	iExportDirSize=pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
//
	iCpu=pFileHeader->Machine;
	HMem::Free(iHeader);
	iHeader=0;
	return ETrue;
	}

void PEFile::Close()
//
// close the pe file
//
	{
	HFile::Close(iFileHandle);
	}

TInt PEFile::ReadExportDirectory()
//
// Read in just the export directory
//
	{

	if (iExportDataDir==0)
		return KErrNotFound;
	TInt r=ReadSectionHeaders();
	if (r!=KErrNone)
		return r;
	iSectionData[KConstSection]=ReadSectionData(iSectionHeader[KConstSection]);
	iExpDirectoryOffset=iSectionHeader[KConstSection]->VirtualAddress;
	iExpDirectory=(PIMAGE_EXPORT_DIRECTORY)(iSectionData[KConstSection]+iExportDataDir-iExpDirectoryOffset);
	
#if 0 // Not byte swapped, therefore all access to it must re order
	if (iExpDirectory) flipIMAGE_EXPORT_DIRECTORY(*iExpDirectory); 
#endif
	return KErrNone;
	}

TInt PEFile::ReadSectionHeaders()
//
// Read in the section headers
//
	{
	
	TInt i;
	for (i=0; i<KNumberOfSections; i++)
		iSectionHeader[i]=NULL;
	HFile::Seek(iFileHandle, iStartOfHeaders+sizeof(IMAGE_NT_HEADERS));
	for (i=0; i<(TInt)iNumSections; i++)
		{
		PIMAGE_SECTION_HEADER header = new IMAGE_SECTION_HEADER;
		if (!HFile::Read(iFileHandle, header, sizeof(IMAGE_SECTION_HEADER)))
			return Print(EError, "Cannot read section header.\n");

		flipIMAGE_SECTION_HEADER(*header); // byte re order

		if (CmpSectionName(header, ".text"))
			iSectionHeader[KTextSection]=header;
		else if (CmpSectionName(header, ".rdata"))
			iSectionHeader[KConstSection]=header;
		else if (CmpSectionName(header, ".edata"))
			iSectionHeader[KExportSection]=header;
		else if (CmpSectionName(header, ".data"))
			iSectionHeader[KDataSection]=header;
		else if (CmpSectionName(header, ".bss"))
			iSectionHeader[KBssSection]=header;
		else if (CmpSectionName(header, ".idata"))
			iSectionHeader[KImportSection]=header;
		else if (CmpSectionName(header, ".reloc"))
			iSectionHeader[KRelocSection]=header;
		else if (CmpSectionName(header, ".CRT"))
			iSectionHeader[KCrtSection]=header;
		else if (CmpSectionName(header, ".stab"))
			delete header;
		else if (CmpSectionName(header, ".stabstr"))
			delete header;
		else if (CmpSectionName(header,".E32_UID"))
			delete header;
		else if (CmpSectionName(header,".rsrc"))
			delete header;
		else
			{
			Print(EWarning, "Section '%.8s' removed.\n", header->Name);
			delete header;
			}
		}
	return KErrNone;
	}

char *PEFile::ReadSectionData(PIMAGE_SECTION_HEADER aPeHeader)
//
// Read in the data for this section
//
	{
	char *section=NULL;
	if (aPeHeader)
		{
		section=(char *)HMem::Alloc(NULL, aPeHeader->SizeOfRawData);
		if (section==NULL)
			return NULL;
		HFile::Seek(iFileHandle, aPeHeader->PointerToRawData);
		HFile::Read(iFileHandle, section, aPeHeader->SizeOfRawData);
		}
	return section;
	}

TInt PEFile::ReadData()
//
//
//
	{

	TInt i;
	for (i=0; i<KNumberOfSections; i++)
		{
		if (iSectionHeader[i])
			{
			iSectionData[i]=ReadSectionData(iSectionHeader[i]);
			if (iSectionData[i]==NULL)
				return Print(EError, "Cannot read %s section data.\n", iSectionHeader[i]->Name);
			}
		else
			iSectionData[i]=NULL;
		}
	return KErrNone;
	}

TInt PEFile::NumberOfImports() const
//
// Count the total number of imports for this image
//
	{
	
	char *importData=iSectionData[KImportSection];
	PIMAGE_SECTION_HEADER importHeader=iSectionHeader[KImportSection];
	if (importData==NULL)
		return 0;
	TInt n=0;
	TUint *src=(TUint *)importData;

	while (*src) // No need for byte re order here, since 0 is unchanged under byte swap operation.
	             // Also, we don't want to muck with the underlying data!
		{
		TUint vaoffset=src[4];
		flipi(vaoffset); // Re-order

		TUint offset=vaoffset-importHeader->VirtualAddress; // find the offset into the section of import addr table
		TUint *p=(TUint *)(importData+offset);
		while (*p++) // Here too, no need for byte re-order
			n++;
		src+=5; // sizeof pe import block/4
		}
	return n;
	}

TInt PEFile::NumberOfImportDlls() const
//
// Count the number of referenced Dlls
//
	{

	char *importData=iSectionData[KImportSection];
//	PIMAGE_SECTION_HEADER importHeader=iSectionHeader[KImportSection];
	if (importData==NULL)
		return 0;
	TInt n=0;
	while (*(TUint *)importData) // No need for byte re order (see above)
		{
		n++;
		importData+=5*4; // size of pe import block
		}
	return n;
	}

TInt PEFile::NumberOfExports() const
//
// Count the number of exported symbols
//
	{

	if (iExportDataDir==0)
		return 0;
	TInt x = ((PIMAGE_EXPORT_DIRECTORY)iSectionData[KExportSection])->NumberOfFunctions;
	flipi(x); // Re order.
	return x;
	}

TInt PEFile::NumberOfRelocs()
//
// Count the number of reloctions
//
	{

	if (iSectionData[KRelocSection]==NULL)
		return 0;
	char *relocs=iSectionData[KRelocSection];
	TInt n=0;
	TInt dudrelocs=0;
	TInt blocksize;
	TUint page;
	TInt size=iSectionHeader[KRelocSection]->Misc.VirtualSize;


	
	while (size>0)
		{
		page=*(TUint *)relocs;

		blocksize=*(TInt *)(relocs+4);

		flipi(page); // re order
		flipi(blocksize);
		
		if (blocksize==0)
			break;
		size-=blocksize;
		TUint16 *p=(TUint16 *)(relocs+8);

		relocs+=blocksize;
		blocksize-=8;
		while (blocksize>0)
		  {
		    TUint16 pp = *p;
		    flips(pp); // Re order 
		    TInt rtype=(pp&0xf000)>>12;
		    if (rtype==IMAGE_REL_BASED_HIGHLOW)
		      {
			TUint va=page+(pp&0xfff);
			TInt section=FindSectionByVa(va, iSectionHeader);
			if (section==KTextSection || section==KConstSection || section==KDataSection || section==KCrtSection)
			  n++;
			else
			  dudrelocs++;
		      }
		    else if (rtype!=IMAGE_REL_BASED_ABSOLUTE)	// used for padding
		      Print(EWarning, "Relocation type other than IMAGE_REL_BASED_HIGHLOW has been ignored.\n");
		    p++;
		    blocksize-=2;
		  }
		}
#if defined(_DEBUG)
	if (dudrelocs>0)
		Print(EWarning, "Image '%s' has %d relocations pointing at uninitialised data.\n", iFileName, dudrelocs);
#endif

	return n;
	}

void PEFile::GetRelocs(TUint *aReloc, TUint *aRelocSection, TInt /*aNumberOfRelocs*/)
//
// load the relocs from the reloc section into relocation and relocsection arrays
//
	{

	TUint *relocation=aReloc;
	TUint *relocsection=aRelocSection;
	char *aRelocData=iSectionData[KRelocSection];

	TUint16 *relocs=(TUint16 *)aRelocData;
	TInt relocsize=iSectionHeader[KRelocSection]->Misc.VirtualSize;

	TUint offset;
	TUint page;
	TInt i=0;
	

	while (relocsize>0)
		{
		page=*(TUint *)relocs;

		flipi(page); // re order

		relocs+=2;
		TInt size=*(TUint *)relocs;

		flipi(size); // re order

		if (size==0)
			break;
		
		relocsize-=size;
		relocs+=2;
		size-=8;
		
		while (size>0)
			{
			  TUint16 x = *relocs++;
			  offset = vflips(x); // re order
			
			TInt type=offset&0xf000;
			if (type==0x3000)
				{
				TUint va=page+(offset&0xfff);
				TInt section=FindSectionByVa(va, iSectionHeader);
				if (section==KTextSection || section==KConstSection || section==KDataSection || section==KCrtSection)
					{
					relocsection[i]=section;
					relocation[i++]=va; // No byte ordering here...
					}
				}
			size-=2;
			}
		}


	}

TInt PEFile::Normalise()
//
// Remove the MSVC anomalies
//
	{

	// MSVC puts export data in with .rdata
	if (iExportDataDir && iSectionHeader[KExportSection]==NULL)
		{
		if (!PEFile::VirtualAddressInSection(iExportDataDir, iSectionHeader[KConstSection]))
			return Print(EError, "Can't find exports in this PE file.\n");
		else
			{
			iSectionHeader[KExportSection]=new IMAGE_SECTION_HEADER;
			iSectionHeader[KExportSection]->VirtualAddress=iExportDataDir;
			iSectionHeader[KExportSection]->Misc.VirtualSize=iExportDirSize;
			iSectionHeader[KExportSection]->SizeOfRawData=iExportDirSize;
			iSectionData[KExportSection]=new char [iExportDirSize];
			if (iSectionData[KExportSection]==NULL)
				return Print(EError, "Out of memory.\n");
			memcpy(iSectionData[KExportSection], iSectionData[KConstSection]+iExportDataDir-iSectionHeader[KConstSection]->VirtualAddress, iExportDirSize);
			// adjust .rdata so it does not include .edata
			iSectionHeader[KConstSection]->Misc.VirtualSize-=iExportDirSize;
			iSectionHeader[KConstSection]->SizeOfRawData-=iExportDirSize;
			char *c=new char [iSectionHeader[KConstSection]->SizeOfRawData];
			if (c==NULL)
				return Print(EError, "Out of memory.\n");
			memcpy(c, iSectionData[KConstSection], iSectionHeader[KConstSection]->SizeOfRawData);
			delete iSectionData[KConstSection];
			iSectionData[KConstSection]=c;
			}
		}
	// Stupid compilers generate .idata sections even when there are no imports
	if (iSectionHeader[KImportSection])
		{
		if (NumberOfImports()==0)
			{
			delete iSectionHeader[KImportSection];
			delete iSectionData[KImportSection];
			iSectionHeader[KImportSection]=NULL;
			iSectionData[KImportSection]=NULL;
			}
		}
	return KErrNone;
	}


TInt PEFile::HasInitialisedData(PIMAGE_SECTION_HEADER aHeader)
//
// Returns true if the pe file section contains any initialised data
//
	{

	if (aHeader==NULL)
		return FALSE;
	if (aHeader->SizeOfRawData==0)
		return FALSE;
	return TRUE;
	}

void PEFile::CopySectionData(TAny *source, TAny *dest, TUint32 fileLength, TUint32 memLength)
	{

	if (fileLength <= memLength)
		{
		Print(EScreen,"   Copying %08x bytes from file at %08x to memory at %08x\n", fileLength, source, dest);
		HMem::Copy(dest,source,fileLength);
		dest = (TAny *)((TUint32)dest + fileLength);
		TUint32 remainingSize = memLength - fileLength;
		Print(EScreen,"   Zeroing remaining %08x bytes at %08x\n", remainingSize, dest);
		HMem::Set(dest, 0, remainingSize);
		}
	else
		{
		Print(EScreen,"   Copying %08x bytes from file at %08x to memory at %08x\n", memLength, source, dest);
		HMem::Copy(dest,source,memLength);
		}
	}		


// XXX Needs some more scrutiny re byteswaping
TBool PEFile::ProcessRelocData(TAny *relocData,TInt dataSize)
{
  
  TBool ret = ETrue;
  TBool hadBadRelocs=EFalse;

  PIMAGE_BASE_RELOCATION pRelocData = (PIMAGE_BASE_RELOCATION)((TUint32)relocData);
  Print(ELog,"   Info on .reloc section...\n");

	
	while (pRelocData->SizeOfBlock != 0)
		{
		TUint16 relocType;
		TUint32 relocOffset;
		TUint32 *relocAddr;

		TUint32 va = pRelocData->VirtualAddress, bsize = pRelocData->SizeOfBlock;
		
		flipi(va); // re order
		flipi(bsize);
		
		Print(ELog,"      Virtual address: %08x  size: %08x\n",va, bsize);
			
		TUint numEntries = (bsize-sizeof(*pRelocData))/sizeof(TUint16);
		TUint16 *pEntry = (TUint16 *)((TUint32)pRelocData+sizeof(*pRelocData));
		
		for (TUint i=0; i<numEntries; i++)
			{
			  // Extract the top 4 bits of the relocation entry. This is the type
			  TUint16 pe = *pEntry;
			  flips(pe);
			  relocType = (TUint16)((pe & 0xF000)>>12);
			  // The rest of the field is the offset
			  relocOffset = (pe & 0x0FFF);
			switch (relocType)
				{
				case 0: // Just padding
					pEntry++;
					break;

				case 1:
				case 2:
				case 4:
				case 5:
					Print(EPeError,".reloc section, relocation type not handled.\n");
					ret = EFalse;
					goto done;
					break;

				case 3:
					{
					if (va==0) // bug in .reloc section of arm ekern.exe
						{
						pEntry++;
						break;
						}
					TUint thisReloc=0;
					relocAddr = (TUint32 *)((TUint32)iMemBase + (TUint32)va + relocOffset);
					TUint32 reloc = *relocAddr;					
					flipi(reloc); // re order
					
					if (IsInCode((TUint32)relocAddr) || IsInData((TUint32)relocAddr))
						{
						if (IsInDataReloc(reloc))
					   		{
							if (iImageIsDll)
								{
								Print(EPeError,"Dlls should have no RAM (data) relocations.\n");
								ret = EFalse;
								goto done;
								}
							thisReloc=reloc+iRamDelta;
							}
						else
							thisReloc=reloc+iRomDelta;
						*relocAddr = thisReloc; // this line here to enable breaking on values of thisReloc 
						}
					else
						hadBadRelocs=ETrue;
					pEntry++;
					}
					break;
				default:
					Print(EPeError,".reloc section, invalid relocation type.\n");
					ret = EFalse;
					goto done;
				}
			}
		dataSize-=bsize;
		if(dataSize<=0)
			break;
		pRelocData = (PIMAGE_BASE_RELOCATION)((TUint32)pRelocData+bsize);
		}

	if (hadBadRelocs)
		Print(EPeError,"File %s has relocation in invalid section\n",iFileName);
 done:

	return ret;
	}

TBool PEFile::IsInCode(TUint32 anAddr)
	{
	if ((anAddr>=iCodeSectionAddr) && (anAddr<(iCodeSectionAddr+iCodeSectionSize)))
		return(ETrue);
	if ((anAddr>=iRDataSectionAddr) && (anAddr<(iRDataSectionAddr+iRDataSectionSize)))
		return(ETrue);
	if ((anAddr>=iCRTSectionAddr) && (anAddr<(iCRTSectionAddr+iCRTSectionSize)))
		return(ETrue);
	return(EFalse);
	}

TBool PEFile::IsInData(TUint32 anAddr)
	{
	if ((anAddr>=iDataSectionAddr) && (anAddr<(iDataSectionAddr+iDataSectionSize)))
		return(ETrue);
	if ((anAddr>=iBssSectionAddr) && (anAddr<(iBssSectionAddr+iBssSectionSize)))
		return(ETrue);
	return(EFalse);
	}

TBool PEFile::IsInDataReloc(TUint32 anAddr)
	{
	if ((anAddr>=iDataSectionLinkedAddr) && (anAddr<(iDataSectionLinkedAddr+iDataSectionSize)))
		return(ETrue);
	if ((anAddr>=iBssSectionLinkedAddr) && (anAddr<(iBssSectionLinkedAddr+iBssSectionSize)))
		return(ETrue);
	return(EFalse);
	}
 

TBool PEFile::IsValidDOSHeader(PIMAGE_DOS_HEADER aDOSHeader)
 	{
	if (aDOSHeader->e_magic!=IMAGE_DOS_SIGNATURE)
		{
		Print(EPeError,"File does not have valid DOS MZ signature.\n");
		return EFalse;
		}
	else
		return ETrue;
	}			  



TBool PEFile::IsValidNTHeader(PIMAGE_NT_HEADERS aNTHeader)
 	{
 	if (aNTHeader->Signature != IMAGE_NT_SIGNATURE )
		{
		Print(EPeError,"File does not have valid NT PE signature.\n");
		return EFalse;
		}
	else
	 return ETrue;
	}  


TBool PEFile::IsValidFileHeader(PIMAGE_FILE_HEADER aFileHeader)
 	{
	if ((aFileHeader->Machine != IMAGE_FILE_MACHINE_I386) 
		&& (aFileHeader->Machine != 0xa00) 
		&& (aFileHeader->Machine != 0xb00) 
		&& (aFileHeader->Machine !=IMAGE_FILE_MACHINE_ALPHA))
		{
		Print(EPeError,"File is not a valid i386, ARM, M*Core or ALPHA executable.\n");
		return EFalse;
		}

	if (aFileHeader->SizeOfOptionalHeader == 0)
		{
		Print(EPeError,"Optional header is 0 bytes in length - this is probably an object, not an executable\n");
		return EFalse;
		}

  	if (!(aFileHeader->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE))
		{
		Print(EPeError,"File is not a valid executable - probably linker error\n");
		return EFalse;
		}

	return ETrue;
	}


// Get details of the next import to fix-up in the current file. Fill in the name of the dll 
//it is imported from, the ordinal number and the address to write back to.
#define ORDINAL_DONE 0x40000000

TImportStat PEFile::GetNextImport(TText * &aDllName, TUint16 &aOrdinal, TUint32 * &aThunk)
	{
	PIMAGE_THUNK_DATA pLookupTable = 0;
	TUint32 *pThunk = 0;
	TUint32 rawOrdinal = ORDINAL_DONE;
	TImportStat res = EImpDone;
	PIMAGE_IMPORT_DESCRIPTOR impDesc = iImpDescriptor;
	TText *expDllName = 0;

	if (impDesc == 0)
		return EImpDone; // This file imports nothing
		
 	while ((rawOrdinal & ORDINAL_DONE) && ((impDesc->TimeDateStamp!=0 ) || (impDesc->Name!=0)))
		{
		  TUint32 ch = impDesc->Characteristics;
		  TUint32 nm = impDesc->Name;
		  TUint32 ft = (TUint32)impDesc->FirstThunk;

		  flipi(ch); // re order
		  flipi(nm);
		  flipi(ft);
		  
		  expDllName = (TText *)(iMemBase + nm);
		  pLookupTable = (PIMAGE_THUNK_DATA)(iMemBase + ch);
		  pThunk = (TUint32 *)(iMemBase + ft);

		  while ((rawOrdinal & ORDINAL_DONE) && (pLookupTable->u1.AddressOfData != 0))
 			{
			  TUint32 ord = pLookupTable->u1.Ordinal;
			  
			  flipi(ord); // re order
 			if (ord & IMAGE_ORDINAL_FLAG )
			  rawOrdinal = ord;
			else
				{
				Print(EPeError,"in file %s\n",iFileName);
				Print(EPeError,"It is importing a symbol by name from %s\n",expDllName);
				return EImpError;
				}
			pThunk++;
			pLookupTable++;
			}	
		impDesc++;
		}

	if (!(rawOrdinal & ORDINAL_DONE))
		{

		pThunk--;
		pLookupTable--;
		res = EImpSuccess;
		aDllName = expDllName;
		aThunk = pThunk;
		aOrdinal = (TUint16)(rawOrdinal & 0xFFFF);

		pLookupTable->u1.Ordinal |= vflipi(ORDINAL_DONE);
		}

	return res;
	}

// XXX check usage of function..
TUint32 PEFile::GetFixUp(const TUint16 aOrdinal)
//
// Look through export directory to find fix-up for given ordinal
//
	{

	TUint32 ordBase = iExpDirectory->Base;
	flipi(ordBase); // re order because this struct is not re ordered
	TUint32 **af = iExpDirectory->AddressOfFunctions;	
	flipi(af); // as above ...

	TUint32 *functions = (TUint32 *)((TUint32)af + iMemBase);
	TUint32 fixupAddr = functions[aOrdinal-ordBase] + iRomRunAddr;

	return fixupAddr;
	}


TText *PEFile::GetImageName()
	{

	  if (iExpDirectory) {
	    TUint32 nm = iExpDirectory->Name;
	    flipi(nm);
	    
	    return (TText *)(iMemBase + nm);
	  } else
		return (TText *)0;
	}

TUint8 *PEFile::GetExportName(const TUint16 aOrdinal)
//
// Look through export directory to find name for a given ordinal
//
	{
	TUint32 ordBase = iExpDirectory->Base;
	TUint32 nNames = iExpDirectory->NumberOfNames;
	TUint32  **af = iExpDirectory->AddressOfNames;
	flipi(nNames);
	flipi(ordBase); // re order
	flipi(af);

	TUint32 *names = (TUint32 *)(iSectionData[KConstSection]+(TUint)af-iExpDirectoryOffset);// + iMemBase);
	if (aOrdinal <= nNames)
		{
		TUint8 *theName = (TUint8 *)(names[aOrdinal-ordBase]-iExpDirectoryOffset+iSectionData[KConstSection]);
		return theName;
		}
	else
		return (TUint8 *)0;
	}


TUint PEFile::GetNumberOfExportedFunctions()
	{
	  return vflipi(iExpDirectory->NumberOfFunctions);
	}


TUint PEFile::GetOrdinalBase()
	{

	return vflipi(iExpDirectory->Base);
	}


TBool PEFile::ExportSectionExists()
	{

	if (iExpDirectory)
		return ETrue;
	else
		return EFalse;
	}


TBool PEFile::ImportSectionExists()
	{

	if (iImpDescriptor)
		return ETrue;
	else
		return EFalse;
	}


TUint PEFile::RoundToSectionSize(TUint aSize)
//
// Round to the nearest size in sections
//
	{
	TUint sAlign = iSectionAlign;
	return ((aSize+sAlign-1)/sAlign)*sAlign ;
	}


void PEFile::DumpPeHeaders()
//
// Print out loads of stuff from the PE header
//
	{
	TInt err = HFile::Open(iFileName, &iFileHandle);
	if (err!=0) 
		return;

	iHeader = (PIMAGE_NT_HEADERS)(HMem::Alloc(0,iSizeOfHeaders-iStartOfHeaders));
	if (!iHeader)
		return;

	if (!HFile::Seek(iFileHandle, iStartOfHeaders))
		return;

 	if (!HFile::Read(iFileHandle, iHeader, iSizeOfHeaders-iStartOfHeaders))
		return;

	PIMAGE_FILE_HEADER pFileHeader = (PIMAGE_FILE_HEADER)&iHeader->FileHeader;
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&iHeader->OptionalHeader;
	PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)(iHeader+1);

	flipIMAGE_FILE_HEADER(*pFileHeader);
	flipIMAGE_OPTIONAL_HEADER(*pOptionalHeader);

	
 	printf("File header\n");
 	printf("-----------\n\n");
	TText *szMachine=0;
	switch( pFileHeader->Machine )
		{
		case 0xa00:
			szMachine = (TText *)"ARM"; break;
		case 0xb00:
			szMachine = (TText *)"M*Core"; break;
		case IMAGE_FILE_MACHINE_I386:
			szMachine = (TText *)"i386"; break;
		case IMAGE_FILE_MACHINE_I860:
			szMachine = (TText *)"i860"; break;
		case IMAGE_FILE_MACHINE_R3000:
			szMachine = (TText *)"R3000"; break;
		case IMAGE_FILE_MACHINE_R4000:
			szMachine = (TText *)"R4000"; break;
		case IMAGE_FILE_MACHINE_ALPHA:
			szMachine = (TText *)"Alpha"; break;
		case IMAGE_FILE_MACHINE_POWERPC:
			szMachine = (TText *)"IBM PowerPC"; break;
		default:
			printf ("ERROR - machine not specified.\n");
			szMachine = (TText *)"unknown";
			break;
		}

	printf("\n    Machine: %s (Id=%04x)",szMachine, pFileHeader->Machine);
	if ((pFileHeader->Machine != 0xa00)		// ARM
		&& (pFileHeader->Machine != 0xb00))	// M*Core
		printf("..........ERROR!!");
    printf("\n    Number of sections : %04x",pFileHeader->NumberOfSections);
    printf("\n    Time date stamp : %08lx",pFileHeader->TimeDateStamp);
	if (pFileHeader->TimeDateStamp == 0)
		printf("..........ERROR!!");
    printf("\n    Pointer to symbol table : %08lx",pFileHeader->PointerToSymbolTable);
    printf("\n    Number of symbols : %08lx",pFileHeader->NumberOfSymbols);
    printf("\n    Size of optional header : %08x",pFileHeader->SizeOfOptionalHeader);
	if (pFileHeader->SizeOfOptionalHeader == 0)
		printf("..........ERROR!!");
    printf("\n    Characteristics : %08x\n",pFileHeader->Characteristics);
	if (pFileHeader->Characteristics & IMAGE_FILE_RELOCS_STRIPPED)
		printf("\n      Relocations stripped..........ERROR!!");
	if (pFileHeader->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)
		printf("\n      Executable image.");
	else
		printf("\n      Not executable image..........ERROR!!");
	if (pFileHeader->Characteristics & IMAGE_FILE_CHAR_REVERSED_LO)
		printf("\n      Bytes reversed lo..........ERROR!!");
 	if (pFileHeader->Characteristics & IMAGE_FILE_32BIT_MACHINE)
		printf("\n      32bit image.");
	else
		printf("\n      Not 32bit image..........ERROR!!");
 	if (pFileHeader->Characteristics & IMAGE_FILE_SYSTEM)
		printf("\n      System file.");
 	if (pFileHeader->Characteristics & IMAGE_FILE_DLL)
		printf("\n      Dll file.");
	if (pFileHeader->Characteristics & IMAGE_FILE_CHAR_REVERSED_HI)
		printf("\n      Bytes reversed hi..........ERROR!!");


  	printf ("\n\n\nOptional Header\n");
  	printf ("------------------");

	printf("\n    Magic = %04x", pOptionalHeader->Magic);
	printf("\n    Major Linker Version = %02x", pOptionalHeader->MajorLinkerVersion);
	printf("\n    Minor Linker Version = %02x", pOptionalHeader->MinorLinkerVersion);
	printf("\n    Size of code (bytes) = %08lx", pOptionalHeader->SizeOfCode);
	printf("\n    Size of initialized data (bytes) = %08lx", pOptionalHeader->SizeOfInitializedData);
	printf("\n    Size of uninitialized data (bytes) = %08lx", pOptionalHeader->SizeOfUninitializedData);
	printf("\n    Entrypoint RVA = %08lx", pOptionalHeader->AddressOfEntryPoint);
	if (pOptionalHeader->AddressOfEntryPoint & 0x80000000)
		printf("..........ERROR!!");
	printf("\n    Base of code = %08lx", pOptionalHeader->BaseOfCode);
	if (pOptionalHeader->BaseOfCode & 0x80000000)
		printf("..........ERROR!!");
	printf("\n    Base of data = %08lx", pOptionalHeader->BaseOfData);
	if (pOptionalHeader->BaseOfData & 0x80000000)
		printf("..........ERROR!!");
	printf("\n    Image base = %08lx", pOptionalHeader->ImageBase);
	if (pOptionalHeader->ImageBase & 0x80000000)
		printf("..........ERROR!!");
	printf("\n    Section alignment (bytes) = %08lx",pOptionalHeader->SectionAlignment);
	if (pOptionalHeader->SectionAlignment & 0x80000000)
		printf("..........ERROR!!\n");
	printf("\n    File alignment (bytes) = %08lx", pOptionalHeader->FileAlignment);
	if (pOptionalHeader->FileAlignment & 0x80000000)
		printf("..........ERROR!!");
	printf("\n    Major Operating System Version = %04x", pOptionalHeader->MajorOperatingSystemVersion);
	printf("\n    Minor Operating System Version = %04x", pOptionalHeader->MinorOperatingSystemVersion);
	printf("\n    Major Image Version = %04x", pOptionalHeader->MajorImageVersion);
	printf("\n    Minor Image Version = %04x", pOptionalHeader->MinorImageVersion);
	printf("\n    Major Subsystem Version = %04x", pOptionalHeader->MajorSubsystemVersion);
	printf("\n    Minor Subsystem Version = %04x", pOptionalHeader->MinorSubsystemVersion);

	printf("\n    Size of image (bytes) = %08lx", pOptionalHeader->SizeOfImage);
	if (pOptionalHeader->SizeOfImage & 0x80000000)
		printf("..........ERROR!!");
	printf("\n    Size of headers (bytes) = %08lx",pOptionalHeader->SizeOfHeaders);
	if (pOptionalHeader->SizeOfHeaders & 0x80000000)
		printf("..........ERROR!!");
	printf("\n    CheckSum = %04lx", pOptionalHeader->CheckSum);
	printf("\n    Subsystem = %04x", pOptionalHeader->Subsystem);
	printf("\n    Dll Characteristics = %04x", pOptionalHeader->DllCharacteristics);
	printf("\n    Size Of Stack Reserve = %04lx", pOptionalHeader->SizeOfStackReserve);
	printf("\n    Size Of Stack Commit = %04lx", pOptionalHeader->SizeOfStackCommit);
	printf("\n    Size Of Heap Reserve = %04lx", pOptionalHeader->SizeOfHeapReserve);
	printf("\n    Size Of Heap Commit = %04lx", pOptionalHeader->SizeOfHeapCommit);
	printf("\n    Loader Flags = %04lx", pOptionalHeader->LoaderFlags);
	printf("\n    Number Of Rva and Sizes = %04lx", pOptionalHeader->NumberOfRvaAndSizes);

	printf("\n\n\nSection Headers\n");
 	printf("---------------\n\n");

 	for (TUint i=0;i<iNumSections;i++)
		{
		  flipIMAGE_SECTION_HEADER(*pSectionHeader);
		  DumpNextSectionInFile(pSectionHeader);
		  pSectionHeader++;
		}

	if (!hadText)
 		printf("\nERROR - missing code section.");
	if (!hadReloc)
 		printf("\nERROR - missing reloc section. (All images must be relocatable.)");
	HMem::Free(iHeader);
 	}


void PEFile::DumpNextSectionInFile(PIMAGE_SECTION_HEADER pSectionHeader)
//
// Print out loads of stuff from the section header
//
	{
	printf("\nSection name %-8.8s\n",pSectionHeader->Name);
	printf("\n    Virtual size            : %08lx", pSectionHeader->Misc.VirtualSize);
	printf("\n    RVA of section data     : %08lx", pSectionHeader->VirtualAddress);
	if (pSectionHeader->VirtualAddress & 0x80000000)
		printf("..........ERROR!!");
	printf("\n    Size of raw data        : %08lx", pSectionHeader->SizeOfRawData);
	printf("\n    Pointer to raw data     : %08lx", pSectionHeader->PointerToRawData);
	printf("\n    Characteristics: %08lx\n", pSectionHeader->Characteristics);
 	if (pSectionHeader->Characteristics & IMAGE_SCN_LNK_REMOVE)
		printf("\nERROR - Section should have been removed by linker.\n");

	// read the section in
	TUint32 filePos = pSectionHeader->PointerToRawData;
 	TUint32 fileLength = pSectionHeader->SizeOfRawData;
//	TUint32 memLength = pSectionHeader->Misc.VirtualSize;
	TAny *sectionFile = HMem::Alloc((TAny *)0, fileLength); // get a buffer
	HFile::Seek(iFileHandle, filePos);
	HFile::Read(iFileHandle, sectionFile, fileLength); // and read the file into the buffer
//	TAny *sectionMem = (TAny *)((TUint32)iMemBase + pSectionHeader->VirtualAddress);

	if (strncasecmp((const char *)pSectionHeader->Name, ".text", IMAGE_SIZEOF_SHORT_NAME) == 0)
		{
		hadText = ETrue;
		if (!(pSectionHeader->Characteristics & IMAGE_SCN_CNT_CODE))
			printf("\nERROR - Code section has incorrect characteristics.\n");
		else if (!(pSectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE))
			printf("\nERROR - Code section has incorrect characteristics.\n");
		else if (!(pSectionHeader->Characteristics & IMAGE_SCN_MEM_READ))
			printf("\nERROR - Code section has incorrect characteristics.\n");
		}
	else if (strncasecmp((const char *)pSectionHeader->Name, ".data", IMAGE_SIZEOF_SHORT_NAME) == 0)
		{
		if (iImageIsDll)
			{
			printf ("\nERROR - DLL has data section.\n");
			}
		else
			{
			if (!(pSectionHeader->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA))
				printf("\nERROR - data section has incorrect characteristics.\n");
			else if (!(pSectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE))
				printf("\nERROR - data section has incorrect characteristics.\n");
			else if (!(pSectionHeader->Characteristics & IMAGE_SCN_MEM_READ))
				printf("\nERROR - data section has incorrect characteristics.\n");
			}
		}
	else if (strncasecmp((const char *)pSectionHeader->Name, ".rdata", IMAGE_SIZEOF_SHORT_NAME) == 0)
		{
		}							
	else if (strncasecmp((const char *)pSectionHeader->Name, ".bss", IMAGE_SIZEOF_SHORT_NAME) == 0)
		{
		if (iImageIsDll)
			{
			printf ("\nERROR - DLL has bss section.\n");
			}
		else
			{
			if (!(pSectionHeader->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA))
				printf("\nERROR - BSS section has incorrect characteristics.\n");
			else if (!(pSectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE))
				printf("\nERROR - BSS section has incorrect characteristics.\n");
			else if (!(pSectionHeader->Characteristics & IMAGE_SCN_MEM_READ))
				printf("\nERROR - BSS section has incorrect characteristics.\n");
			}
		}
 	else if (strncasecmp((const char *)pSectionHeader->Name, ".reloc", IMAGE_SIZEOF_SHORT_NAME) == 0)
		{
		hadReloc = ETrue;
		}
	else if (strncasecmp((const char *)pSectionHeader->Name, ".idata", IMAGE_SIZEOF_SHORT_NAME) == 0)
		{
		}
	else if (strncasecmp((const char *)pSectionHeader->Name, ".edata", IMAGE_SIZEOF_SHORT_NAME) == 0)
		{
		iExpDirectory = (PIMAGE_EXPORT_DIRECTORY)(sectionFile);
		if (iImageIsDll)
			{
			  IMAGE_EXPORT_DIRECTORY x = *iExpDirectory;
			  
			  flipIMAGE_EXPORT_DIRECTORY(x); // re order then print
			printf("\n      Ordinal base = %08lx", x.Base);
			printf("\n      Number of functions = %08lx", x.NumberOfFunctions);
			printf("\n      Number of names = %08lx", x.NumberOfNames);
			printf("\n      Export address table RVA = %08lx", (TUint32)x.AddressOfFunctions);
			printf("\n      Name pointer RVA = %08lx", (TUint32)x.AddressOfNames);
			printf("\n      Ordinal table RVA = %08lx", (TUint32)x.AddressOfNameOrdinals);
			}
		else
			{
			printf("\nERROR - non-DLL with export section.");
			}
 		}
	else
		{
		printf("\nERROR - unexpected section.");
		}

	HMem::Free(sectionFile);
	return;
	}


// The following static functions are passed an array of PE files to operate on
TBool PEFile::DoFixUps(PEFile aPeArray[], TUint aMaxFiles)
//
// Fix up the files' imports with each other
//
	{

	PEFile *exportingPEFile;
	TUint16 ordinal;
	TUint32 fixUpAddr;
	TUint32 *thunk;
	TText *nameOfExportingDll;
	TImportStat s;

	for (TUint i=0; i<aMaxFiles; i++)
		{
		while ((s=aPeArray[i].GetNextImport(nameOfExportingDll, ordinal, thunk))==EImpSuccess)
			{
				//			strupr((char *)nameOfExportingDll);  @todo (alfredh) is this really necessary ?
			if (FindByName(aPeArray, aMaxFiles, nameOfExportingDll, exportingPEFile))
				{
				fixUpAddr = exportingPEFile->GetFixUp(ordinal);
				*thunk = fixUpAddr;
				}
			else
				{
				printf("ERROR: can't fix up %s, as file %s is not in memory\n",
					aPeArray[i].GetImageName(), nameOfExportingDll);
				return EFalse;
				}
			}
		 if (s==EImpError)
		 	return EFalse;
		}
	return ETrue;
	}

 
TBool PEFile::FindByName(PEFile aPeArray[], TUint aMaxFiles,const TText * const aDllName, PEFile * &aExportPEFile)
//
// Find the PE file in the array with a given name, or return false
//
	{

	for (TUint i =0; i<aMaxFiles; i++)
		{
		TText *thisName = aPeArray[i].GetImageName();
		if ((thisName) && (strcmp((const char *)aDllName,(const char *)thisName) == 0))
			{
			aExportPEFile = &aPeArray[i];
			return ETrue;
			}
		}
	return EFalse;
	}

void PEFile::WriteDllRefTables(PEFile aPeArray[], TUint aMaxFiles)
//
// Writes the dll reference tables (list of entry points) for all the dlls called
// by each PE file.
// Note that for RAM exes this will have to be a list of names, as RAM dlls can move
//
	{

	for (TUint i=0;i<aMaxFiles;i++)
		{
		if (!aPeArray[i].iImageIsDll)
			{
			aPeArray[i].WriteDllRefTable(aPeArray, aMaxFiles);
//			if (numDlls!=aPeArray[i].iRomExeHeader->iDllRefTableCount)
//				printf("ERROR in Dll table\n");
			}
		}
	}

TUint32 PEFile::WriteDllRefTable(PEFile aPeArray[], TUint aMaxFiles)
//
// Wander through the import section of a file, writing the entrypoints of DLLs
// it calls to a table. Return the number of entries written
//
	{

	PIMAGE_IMPORT_DESCRIPTOR impDesc = iImpDescriptor;
	PEFile *exportingPEFile=0;
	TText *nameOfExportingDll=0;
	TUint numDlls=0;
	TUint32 *dllRef=NULL; //(TUint32 *)iRomExeHeader->iDllRefTable;
	// now make the dll ref table correct for when the kernel references it in the ROM.
	//iRomExeHeader->iDllRefTable+=iRomLinearBase-iRomMemBase;
	if (impDesc == 0)
		return 0; // This file imports nothing
 	while ((impDesc->TimeDateStamp!=0 ) || (impDesc->Name!=0))
		{
		  TUint32 nm = impDesc->Name;
		  flipi(nm); // re order
		nameOfExportingDll = (TText *)(iMemBase + nm);
		if (FindByName(aPeArray, aMaxFiles, nameOfExportingDll, exportingPEFile))
			{
			*dllRef++ = exportingPEFile->iEntryPoint;
			numDlls++;
			}			
		else
			{
			Print(EPeError,"Can't write DLL table for %s, as file %s is not in memory\n",
					GetImageName(), nameOfExportingDll);
			}
		impDesc++;
		}
	return numDlls;
	}

void PEFile::RelocateExportTables(PEFile aPeArray[], TUint aMaxFiles)
//
// The dll header points to an export table which (on the 486 at least) is just RVAs.
// After all the files are fixed up, this must be replaced by real entry points.
//
	{

	for (TUint i=0;i<aMaxFiles;i++)
		{
		if (aPeArray[i].iImageIsDll && aPeArray[i].iExpDirectory)
			{
			aPeArray[i].RelocateExportTable();
			}
		}
	}

void PEFile::RelocateExportTable()
//
// Relocate the export table
//
	{
	  TUint32 **af = iExpDirectory->AddressOfFunctions;
	  flipi(af);
	  TUint32 nf = iExpDirectory->NumberOfFunctions;
	  TUint32 *expFns=(TUint32 *)((TUint32)af+iMemBase);

	for (TUint i=0;i<nf;i++)
		{
		*expFns+=iMemBase-iRomMemBase+iRomLinearBase;
		expFns++;
		}
	}
