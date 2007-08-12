// PE_TRAN.CPP
//
// Copyright (c) 1996-1999 Symbian Ltd.  All rights reserved.
//

#include <time.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <string.h>
#include <e32image.h>
#include <e32std.h>
#include "pe_defs.h"
#include "pe_file.h"
#include "h_ver.h"
#include "h_utl.h"
#include "h_endian.h"

int gAlignConstSection=FALSE;
TUint gConstSectionAddressMask=0;
static TUint gRequiredConstPadding;

class TE32ImageUids : public TCheckedUid
	{
public:
	TE32ImageUids(TUid aUid1, TUid aUid2, TUid aUid3) : TCheckedUid(TUidType(aUid1, aUid2, aUid3)) {}
	TUint Check() { return TCheckedUid::Check(); }
	};

void E32ImageFile::SetUids(TUid aUid1, TUid aUid2, TUid aUid3)
	{
	iHeader->iUid1=aUid1.iUid;
	iHeader->iUid2=aUid2.iUid;
	iHeader->iUid3=aUid3.iUid;
	TE32ImageUids u(aUid1, aUid2, aUid3);
	iHeader->iCheck=u.Check();
	}

void E32ImageFile::SetCallEntryPoints(TInt aBool)
	{

	if (aBool)
		iHeader->iFlags&=~KImageNoCallEntryPoint;
	else
		iHeader->iFlags|=KImageNoCallEntryPoint;
	}

void E32ImageFile::SetFixedAddress(TInt aBool)
	{

	if (aBool)
		iHeader->iFlags|=KImageFixedAddressExe;
	else
		iHeader->iFlags&=~KImageFixedAddressExe;
	}

void E32ImageFile::SetPriority(TProcessPriority aPri)
	{

	iHeader->iPriority=aPri;
	}

void E32ImageFile::CreateExportDirectory(char *aPtr, PEFile &aPeFile)
//
// create a new format export directory
//
	{
	
	if (iHeader->iExportDirCount==0)
		return;
	TUint *src=(TUint *)aPeFile.iSectionData[KExportSection];
	TUint *dst=(TUint *)aPtr;
	PIMAGE_EXPORT_DIRECTORY dir=(PIMAGE_EXPORT_DIRECTORY)src;
	TUint32 **af = dir->AddressOfFunctions;
	
	flipi(af); // re order
	
	src+=(((TInt)af)-((TInt)aPeFile.iSectionHeader[KExportSection]->VirtualAddress))/4;
	TUint i;
	// @todo andreh - flipi for the number of exported functions
	flipi(dir->NumberOfFunctions);
    Print(EAlways,"NumberOfFunctions[%d]\n",dir->NumberOfFunctions,i);
	for (i=0; i<dir->NumberOfFunctions; i++)
		{
		TUint va=*src++;
		// No need for byte re order here, since this is going to machine directly...
		dst[i]=va;
		}
	FixExportDirectory(dst, aPeFile);
	}

void E32ImageFile::FixExportDirectory(TUint *aExportDir, PEFile &aPeFile)
//
// Fix the export directory
//
	{

	TUint *exportdir=aExportDir;
	TInt n;
	for (n=0; n<(TInt)iHeader->iExportDirCount; n++)
		{
		TUint va=*exportdir;

		flipi(va); // re order (to get it into machine order so we can use it below)

		TInt i=PEFile::FindSectionByVa(va, aPeFile.iSectionHeader);
		if (i==KTextSection)
			va=va-aPeFile.iSectionHeader[i]->VirtualAddress;
		else if (i==KConstSection)
			va=va-aPeFile.iSectionHeader[i]->VirtualAddress+ConstOffset();
		else if (i==KDataSection)
			va=va-aPeFile.iSectionHeader[i]->VirtualAddress+DataOffset();
		else if (i==KBssSection)
			va=va-aPeFile.iSectionHeader[i]->VirtualAddress+BssOffset();
		else
			{
			if (va == 0)
				Print(EWarning, "No export specified for ordinal %d\n", n+1, va);
			else
				Print(EError, "Export %d (address %08x) is not from .text, .rdata, or data sections\n", n+1, va);
			}
		flipi(va); // ...re order again (to little endian) before entering
		*exportdir++=va;
		}
	}

TInt E32ImageFile::DoCodeHeader(PEFile &aPeFile)
//
// Calculate the code parts of the pefile
//
	{

	TInt size=ALIGN4(aPeFile.iSectionHeader[KTextSection]->Misc.VirtualSize);
	iHeader->iTextSize=size;
	TInt nimports=aPeFile.NumberOfImports();
	if (nimports!=0)
		size+=nimports*4+4; // null terminated
	iConstOffset=0;
	if (gAlignConstSection)
		{
	    // Compute the amount of padding to put before the
	    // const section to align it correctly
	    TUint   oldAddressBits = aPeFile.iSectionHeader[KConstSection]->VirtualAddress & gConstSectionAddressMask;
	    TUint   oldConstAddress = size;
	    TUint   newConstAddress = oldConstAddress;
	    // slow but sure
	    while ((newConstAddress & gConstSectionAddressMask) != oldAddressBits)
	    	{
			newConstAddress++;
			}
	    gRequiredConstPadding = newConstAddress - oldConstAddress;
	    size += gRequiredConstPadding;
		}

	if (aPeFile.iSectionHeader[KConstSection])
		{
		iConstOffset=size;
		size+=ALIGN4(aPeFile.iSectionHeader[KConstSection]->Misc.VirtualSize);
		}
	iCrtOffset=0;
	if (aPeFile.iSectionHeader[KCrtSection])
		{
		iCrtOffset=size;
		size+=ALIGN4(aPeFile.iSectionHeader[KCrtSection]->Misc.VirtualSize);
		}
	if (iHeader->iExportDirCount)
		{
		iHeader->iExportDirOffset=iHeader->iCodeOffset+size;
		size+=ALIGN4(iHeader->iExportDirCount*4);
		}
	iHeader->iCodeSize=size;
	return size;
	}

TInt E32ImageFile::DoDataHeader(PEFile &aPeFile, TUint aDataBase)
//
//
//
	{

	if (aDataBase==0)
		aDataBase=iHeader->iCodeBase+iHeader->iCodeSize;
	TInt size=0;
	if (PEFile::HasInitialisedData(aPeFile.iSectionHeader[KDataSection]))
		{
		size=ALIGN4(aPeFile.iSectionHeader[KDataSection]->Misc.VirtualSize);
		iHeader->iDataBase=aDataBase;
		iHeader->iDataOffset=iHeader->iCodeOffset+iHeader->iCodeSize;
		TInt bsssize=aPeFile.iSectionHeader[KDataSection]->Misc.VirtualSize-aPeFile.iSectionHeader[KDataSection]->SizeOfRawData;
		// drop any uninitialised data
		if (bsssize>0)
			{
			iHeader->iBssSize+=bsssize;
			size=ALIGN4(aPeFile.iSectionHeader[KDataSection]->SizeOfRawData);
			}
		iHeader->iDataSize=size;
		}
	else if (aPeFile.iSectionHeader[KDataSection])
		{ // just .bss
		iHeader->iDataBase=aDataBase;
		TInt bsssize=aPeFile.iSectionHeader[KDataSection]->Misc.VirtualSize;
		iHeader->iBssSize+=bsssize;
		}
	if (aPeFile.iSectionHeader[KBssSection])
		{
		iHeader->iBssSize+=ALIGN4(aPeFile.iSectionHeader[KBssSection]->Misc.VirtualSize);
		if (iHeader->iDataBase==0) // .bss but no .data
			iHeader->iDataBase=aDataBase;
		}
	return size;
	}

TInt E32ImageFile::CopyCode(char *p, PEFile &aPeFile)
//
// Copies the files code sections to p
// returns the number of bytes copied or KErrGeneral
//
	{

	// text
	TInt size=aPeFile.iSectionHeader[KTextSection]->Misc.VirtualSize;
	memcpy(p, aPeFile.iSectionData[KTextSection], size);
	p+=ALIGN4(size);
	// iat
	
	TInt nimports=aPeFile.NumberOfImports();
	if (nimports)
		{
		TInt r=CopyImportAddrTable(p, aPeFile);
		p+=ALIGN4(nimports*4+4);
		if (r!=KErrNone)
			return Print(EError, "%s is importing symbols by name.\n", iFileName);
		}
	// rdata
	if (aPeFile.iSectionData[KConstSection])
		{
		if (gAlignConstSection)
			{
			// add padding ahead of const section
			p += gRequiredConstPadding;
			}
		TInt size=ALIGN4(aPeFile.iSectionHeader[KConstSection]->Misc.VirtualSize);
		memcpy(p, aPeFile.iSectionData[KConstSection], size);
		p+=size;
		}
	if (aPeFile.iSectionData[KCrtSection])
		{
		TInt size=ALIGN4(aPeFile.iSectionHeader[KCrtSection]->Misc.VirtualSize);
		memcpy(p, aPeFile.iSectionData[KCrtSection], size);
		p+=size;
		}
	// export dir
	CreateExportDirectory(p, aPeFile);
	p+=iHeader->iExportDirCount*4;
	return iHeader->iCodeSize;
	}

TInt E32ImageFile::CopyData(char *p, PEFile &aPeFile)
	{
	
	if (iHeader->iDataSize)
		memcpy(p, aPeFile.iSectionData[KDataSection], iHeader->iDataSize);
	return iHeader->iDataSize;
	}

TInt64 timeToTInt64(TInt aTime)
	{
	aTime-=(30*365*24*60*60+7*24*60*60);	// seconds since midnight Jan 1st, 2000
	TInt64 daysTo2000AD=730497;
	TInt64 t=daysTo2000AD*24*3600+aTime;	// seconds since 0000
	t=t+3600;								// BST (?)
	return t*1000000;						// milliseconds
	}

TInt E32ImageFile::Translate(const TText * const aFileName, TUint aDataBase, TBool aAllowDllData)
//
// Translate a PE format file to a E32Image file
//
	{

	PEFile pefile;
	if (!pefile.Init(aFileName))
		return KErrGeneral;
	TInt r=pefile.ReadSectionHeaders();
	if (r!=KErrNone) return r;
	r=pefile.ReadData();
	if (r!=KErrNone) return r;
	pefile.Close();
	r=pefile.Normalise();
	if (r!=KErrNone) return r;
	iFileName=strdup((char *)aFileName);

	// Find the images export name
	if (pefile.iSectionData[KExportSection])
		{
		PIMAGE_EXPORT_DIRECTORY e=(PIMAGE_EXPORT_DIRECTORY)pefile.iSectionData[KExportSection];
		// re order of name below, since data section never normalised
		char *name=pefile.iSectionData[KExportSection]+vflipi(e->Name)-pefile.iSectionHeader[KExportSection]->VirtualAddress;
		//		strupr(name); @todo (alfredh) is this really necessary ?
		iExportName=new char [strlen(name)+1];
		strcpy(iExportName, name);
		}

	Adjust(ALIGN4(sizeof(E32ImageHeader)));
	iHeader->iDllRefTableCount=pefile.NumberOfImportDlls();
	iHeader->iExportDirCount=pefile.NumberOfExports();
	iHeader->iCodeBase=pefile.iLinkedBase;
	TInt nimports=pefile.NumberOfImports();
	TInt importSectionSize;
	char *newImportSection=CreateImportSection(pefile, importSectionSize);

	TInt size=ALIGN4(sizeof(E32ImageHeader));
	iHeader->iCodeOffset=size;
	size+=DoCodeHeader(pefile);
	TInt t=DoDataHeader(pefile, aDataBase);
	if (t>0)
		{
		iHeader->iDataOffset=size;
		size+=t;
		}
	if (importSectionSize!=0)
		{
		iHeader->iImportOffset=size;
		size+=importSectionSize;
		}

	char *newCodeRelocs=NULL;
	char *newDataRelocs=NULL;
	TInt codeRelocSize=0, dataRelocSize=0;
	TInt nrelocs=pefile.NumberOfRelocs();
	if (nrelocs)
		{
		TUint *relocs=new TUint [nrelocs];
		TUint *relocsection=new TUint [nrelocs];
		pefile.GetRelocs(relocs, relocsection, nrelocs);
		FixRelocs(pefile, relocs, relocsection, nrelocs);
		newCodeRelocs=CreateCodeRelocs(relocs, relocsection, nrelocs, codeRelocSize);
		newDataRelocs=CreateDataRelocs(relocs, relocsection, nrelocs, dataRelocSize);
		if (codeRelocSize)
			{
			iHeader->iCodeRelocOffset=size;
			size+=codeRelocSize;
			}
		if (dataRelocSize)
			{
			iHeader->iDataRelocOffset=size;
			size+=dataRelocSize;
			}
		delete [] relocs;
		delete [] relocsection;
		}

	Adjust(size);
	char *p=iData+ALIGN4(sizeof(E32ImageHeader));
	t=CopyCode(p, pefile);
	if (t<0)
		return KErrGeneral;
	p+=t;
	p+=CopyData(p, pefile);
	if (nimports)
		{
		memcpy(p, newImportSection, importSectionSize);
		p+=importSectionSize;
		}
	if (codeRelocSize)
		{
		memcpy(p, newCodeRelocs, codeRelocSize);
		p+=codeRelocSize;
		}
	if (dataRelocSize)
		{
		memcpy(p, newDataRelocs, dataRelocSize);
		p+=dataRelocSize;
		}

	// locate the entry point
	TInt entryPointSectionIndex=PEFile::FindSectionByVa(pefile.iEntryPoint, pefile.iSectionHeader);
	TUint entryPointOffset=pefile.iEntryPoint-pefile.iSectionHeader[entryPointSectionIndex]->VirtualAddress;
	if (entryPointSectionIndex!=KTextSection)
		return Print(EError, "Entry Point not in code section\n");

	// Arrange a header for this E32 Image
	strncpy((char*)&iHeader->iSignature, "EPOC", 4);
	switch (pefile.iCpu)
		{
	case IMAGE_FILE_MACHINE_I386:
		iHeader->iCpu=ECpuX86;
		break;
	case 0x0a00:
		iHeader->iCpu=ECpuArm;
		break;
	case 0x0b00:
		iHeader->iCpu=ECpuMCore;
		break;
	default:
		iHeader->iCpu=ECpuUnknown;
		break;
		}
	iHeader->iCheckSumCode=CheckSumCode();
	iHeader->iCheckSumData=CheckSumData();
	iHeader->iVersion=TVersion(MajorVersion, MinorVersion, Build);
	iHeader->iFlags=0;
	if (pefile.iImageIsDll)
		{
		iHeader->iFlags|=KImageDll;
		if (iHeader->iDataSize && !aAllowDllData)
			return Print(EError, "Dll '%s' has initialised data.\n", iExportName);
		if (iHeader->iBssSize  && !aAllowDllData)
			return Print(EError, "Dll '%s' has uninitialised data.\n", iExportName);
		}
	iHeader->iHeapSizeMin=pefile.iHeapCommittedSize;
	iHeader->iHeapSizeMax=pefile.iHeapReservedSize;
	iHeader->iStackSize=pefile.iStackCommittedSize;
	iHeader->iEntryPoint=entryPointOffset;
	iHeader->iTime=timeToTInt64(time(0));
	SetUids(KNullUid, KNullUid, KNullUid);
	SetPriority(EPriorityForeground);

	delete [] newImportSection;
	delete [] newCodeRelocs;
	delete [] newDataRelocs;
	return KErrNone;
	}

TBool E32ImageFile::Translate(PEFile &aPeFile)
//
//
//
	{

	return Translate(aPeFile.iFileName, 0, EFalse);
	}

TUint E32ImageFile::VaOfOrdinal(TUint aOrdinal)
//
// return the offset of the exported symbol
//
	{

	TUint *exportdir=(TUint *)(iData+iHeader->iExportDirOffset);
	TUint va = exportdir[aOrdinal-KOrdinalBase];
	return vflipi(va); // re order
	}
