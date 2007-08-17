// PE_RELOC.CPP
//
// Copyright (c) 1996-1999 Symbian Ltd.  All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <e32std.h>
#include "e32image.h"
#include <h_utl.h>
#include "h_endian.h"

TInt sizeOfCodeRelocs(TUint *relocs, TUint *relocsection, TInt nrelocs)
	{

	TInt bytecount=0;
	TInt page=-1;
	TInt i;
	for (i=0; i<nrelocs; i++)
		{
		if (relocsection[i]==KTextSection || relocsection[i]==KConstSection || relocsection[i]==KCrtSection)
			{
			TInt p=relocs[i]&0xfffff000;
			if (page!=p)
				{
				if (bytecount%4!=0)
					bytecount+=2;
				bytecount+=8; // page, block size
				page=p;
				}
			bytecount+=2;
			}
		}
	if (bytecount%4!=0)
		bytecount+=2;
	return bytecount;
	}

TInt sizeOfDataRelocs(TUint *relocs, TUint *relocsection, TInt nrelocs)
	{

	TInt bytecount=0;
	TInt page=-1;
	TInt i;
	for (i=0; i<nrelocs; i++)
		{
		if (relocsection[i]==KDataSection)
			{
			TInt p=relocs[i]&0xfffff000;
			if (page!=p)
				{
				if (bytecount%4!=0)
					bytecount+=2;
				bytecount+=8; // page, block size
				page=p;
				}
			bytecount+=2;
			}
		}
	if (bytecount%4!=0)
		bytecount+=2;
	return bytecount;
	}

void reorderRelocs(TUint aReloc[], TUint aRelocSection[], TInt aNumberOfRelocs)
//
// sort the relocations in section order
//
	{
	TUint *temp=new TUint [aNumberOfRelocs];
	TUint *tempsection=new TUint [aNumberOfRelocs];
	TInt idx=0;
	TUint section=0;
	while (idx<aNumberOfRelocs)
		{
		for (TInt i=0; i<aNumberOfRelocs; i++)
			{
			if (aRelocSection[i]==section)
				{
				temp[idx]=aReloc[i];
				tempsection[idx]=aRelocSection[i];
				idx++;
				}
			}
		section++;
		}
	memcpy((char *)aReloc, (char *)temp, aNumberOfRelocs*sizeof(TUint));
	memcpy((char *)aRelocSection, (char *)tempsection, aNumberOfRelocs*sizeof(TUint));
	delete [] temp;
	delete [] tempsection;
	}

char *E32ImageFile::CreateCodeRelocs(TUint *relocs, TUint *relocsection, TInt nrelocs, TInt &aSize)
	{

	TInt bytecount=sizeOfCodeRelocs(relocs, relocsection, nrelocs);
	aSize=0;
	if (bytecount==0)
		return NULL;
	aSize=bytecount+sizeof(E32RelocSection);

	char *section=new char [bytecount+sizeof(E32RelocSection)];
	char *data=section+sizeof(E32RelocSection);
	char *startofblock=data;

	TInt ncoderelocs=0;
	TInt page=-1;
	TInt pagesize=8;
	TInt i;
	for (i=0; i<nrelocs; i++)
		{
		if (relocsection[i]==KTextSection || relocsection[i]==KConstSection || relocsection[i]==KCrtSection)
			{
			TInt p=relocs[i]&0xfffff000;
			if (page!=p)
				{
				if (pagesize%4!=0)
					{
					*(TUint16 *)data=0;
					data+=2;
					pagesize+=2;
					}
				*(TUint *)startofblock=vflipi(page); // re order
				*(TUint *)(startofblock+4)=vflipi(pagesize);
				pagesize=8;
				page=p;
				startofblock=data;
				data+=8;
				}
			*(TUint16 *)data=(TUint16)vflips((relocs[i]&0xfff)|0x3000);
			data+=2;
			pagesize+=2;
			ncoderelocs++;
			}
		}
	if (pagesize%4!=0)
		{
		*(TUint16 *)data=0;
		data+=2;
		pagesize+=2;
		}
	*(TUint *)startofblock=vflipi(page); // re order
	*(TUint *)(startofblock+4)=vflipi(pagesize);
	((E32RelocSection *)section)->iNumberOfRelocs=ncoderelocs;
	((E32RelocSection *)section)->iSize=bytecount;
	
	flipi(((E32RelocSection *)section)->iNumberOfRelocs);
	flipi(((E32RelocSection *)section)->iSize);

	return section;
	}

char *E32ImageFile::CreateDataRelocs(TUint *relocs, TUint *relocsection, TInt nrelocs, TInt &aSize)
	{

	TInt bytecount=sizeOfDataRelocs(relocs, relocsection, nrelocs);
	aSize=0;
	if (bytecount==0)
		return NULL;
	aSize=bytecount+sizeof(E32RelocSection);

	char *section=new char [bytecount+sizeof(E32RelocSection)];
	char *data=section+sizeof(E32RelocSection);
	char *startofblock=data;

	TInt ndatarelocs=0;
	TInt page=-1;
	TInt pagesize=8;
	TInt i;
	for (i=0; i<nrelocs; i++)
		{
		if (relocsection[i]==KDataSection)
			{
			TInt p=relocs[i]&0xfffff000;
			if (page!=p)
				{
				if (pagesize%4!=0)
					{
					*(TUint16 *)data=0;
					data+=2;
					pagesize+=2;
					}
				*(TUint *)startofblock=vflipi(page); // re order
				*(TUint *)(startofblock+4)=vflipi(pagesize);
				pagesize=8;
				page=p;
				startofblock=data;
				data+=8;
				}
			*(TUint16 *)data=(TUint16)vflips(((relocs[i]&0xfff)|0x3000)); // and re order
			data+=2;
			pagesize+=2;
			ndatarelocs++;
			}
		}
	if (pagesize%4!=0)
		{
		*(TUint16 *)data=0;
		data+=2;
		pagesize+=2;
		}
	*(TUint *)startofblock=vflipi(page); // re order
	*(TUint *)(startofblock+4)=vflipi(pagesize);

	((E32RelocSection *)section)->iNumberOfRelocs=ndatarelocs;
	((E32RelocSection *)section)->iSize=bytecount;

	flipi(((E32RelocSection *)section)->iNumberOfRelocs); // re order
	flipi(((E32RelocSection *)section)->iSize);

	return section;
	}

void checkreloc(PIMAGE_SECTION_HEADER *sectionheader, TUint va, TUint reloc)
	{

	if (PEFile::FindSectionByVa(va, sectionheader)==-1)
		Print(EError, "bad relocation:  [%08x] = %08x\n", reloc, va);
	}

void E32ImageFile::FixRelocs(PEFile &aPeFile, TUint *relocation, TUint *relocsection, TInt aNumberOfRelocs)
	{

	TUint linkedbase=aPeFile.iLinkedBase;
	TUint *data;
	TInt i;
	for (i=0; i<aNumberOfRelocs; i++)
		{
		switch (relocsection[i])
			{
		case KTextSection:
			relocation[i]-=aPeFile.iSectionHeader[KTextSection]->VirtualAddress;
			data=(TUint *)(aPeFile.iSectionData[KTextSection]+relocation[i]);
						
			flipi(*data); // re order for below check...

			checkreloc(aPeFile.iSectionHeader, *data-linkedbase, relocation[i]+aPeFile.iSectionHeader[KTextSection]->VirtualAddress);

			*data=FixAddress(aPeFile, *data);

			flipi(*data); // re order again...

			break;
		case KConstSection:
			relocation[i]-=aPeFile.iSectionHeader[KConstSection]->VirtualAddress;
			data=(TUint *)(aPeFile.iSectionData[KConstSection]+relocation[i]);


			flipi(*data); // re order for below check...
			
			checkreloc(aPeFile.iSectionHeader, *data-linkedbase, relocation[i]+aPeFile.iSectionHeader[KConstSection]->VirtualAddress);
			relocation[i]+=ConstOffset();
			*data=FixAddress(aPeFile, *data);

			flipi(*data); // re order again...
			
			break;
		case KCrtSection:
			relocation[i]-=aPeFile.iSectionHeader[KCrtSection]->VirtualAddress;
			data=(TUint *)(aPeFile.iSectionData[KCrtSection]+relocation[i]);

			flipi(*data); // re order for below check...
			
			checkreloc(aPeFile.iSectionHeader, *data-linkedbase, relocation[i]+aPeFile.iSectionHeader[KCrtSection]->VirtualAddress);
			relocation[i]+=iCrtOffset;
			*data=FixAddress(aPeFile, *data);

			flipi(*data); // re order again...

			break;
		case KDataSection:
			relocation[i]-=aPeFile.iSectionHeader[KDataSection]->VirtualAddress;
			data=(TUint *)(aPeFile.iSectionData[KDataSection]+relocation[i]);

			flipi(*data); // re order for below check...

			checkreloc(aPeFile.iSectionHeader, *data-linkedbase, relocation[i]+aPeFile.iSectionHeader[KDataSection]->VirtualAddress);
			*data=FixAddress(aPeFile, *data);

			flipi(*data); // re order again...
			break;
		default:
			Print(EWarning, "Relocation in invalid section.\n");
			break;
			}
		}
	reorderRelocs(relocation, relocsection, aNumberOfRelocs);
	}

TUint E32ImageFile::FixAddress(PEFile &aPeFile, TUint va)
//
// Fix the given virtual address for the new headers
//
	{

	va-=aPeFile.iLinkedBase;
	TInt section=PEFile::FindSectionByVa(va, aPeFile.iSectionHeader);
	switch(section)
		{
		case KTextSection:
			va-=aPeFile.iSectionHeader[KTextSection]->VirtualAddress;
			va+=iHeader->iCodeBase;
			break;
		case KConstSection:
			va-=aPeFile.iSectionHeader[KConstSection]->VirtualAddress;
			va+=iHeader->iCodeBase+ConstOffset();
			break;
		case KDataSection:
			va-=aPeFile.iSectionHeader[KDataSection]->VirtualAddress;
			va+=iHeader->iDataBase; //DataOffset();
			break;
		case KCrtSection:
			va-=aPeFile.iSectionHeader[KCrtSection]->VirtualAddress;
			va+=iHeader->iCodeBase+iCrtOffset;
			break;
		case KBssSection:
			va-=aPeFile.iSectionHeader[KBssSection]->VirtualAddress;
			va+=iHeader->iDataBase+iHeader->iDataSize;
			break;
		case KImportSection:
			va=FixImportThunk(aPeFile, va-aPeFile.iSectionHeader[KImportSection]->VirtualAddress);
			va+=iHeader->iCodeBase;
			break;
		default:
			// The va doesn't point into any of the recognised sections
			// This could be because gcc is trying to be clever and pre-calculate
			// array indexes with constant offsets eg  array[x-100]...
			// and storing the address of array[-100] which may poke out of the
			// intended section.
			// We will try and guess where it is supposed to come from.

			// for the time being, pretend it comes from .data (or .bss if no .data)
			if (aPeFile.iSectionHeader[KDataSection])
				{
				va-=aPeFile.iSectionHeader[KDataSection]->VirtualAddress;
				va+=iHeader->iDataBase; //DataOffset();
				}
			else if (aPeFile.iSectionHeader[KBssSection])
				{
				va-=aPeFile.iSectionHeader[KBssSection]->VirtualAddress;
				va+=iHeader->iDataBase+iHeader->iDataSize;
				}
			else
				{
				Print(EWarning, "Address to relocate does not point at .text, .rdata, .idata or data sections\n");
				Print(EWarning, "Problem address = %08x (section %d)\n", va+aPeFile.iLinkedBase, section);
				}
			break;
		}
	// va is now an offset from the start of the text
	return va;
	}


void E32ImageFile::RelocateSection(char* aPtr, char *aRelocs, TUint aCodeDelta, TUint aDataDelta, TUint32 aImagePtr, TUint32* aIATRefs) // char* aImagePtr, TLinAddr** aIATRefs
//
// Relocates the section data at aPtr
//	
	{

	TUint codeStart=iHeader->iCodeBase;
	TUint codeFinish=codeStart+iHeader->iCodeSize;
	TUint iatStart=codeStart+iHeader->iTextSize;
	TUint iatFinish=iatStart+NumberOfImports()*sizeof(TUint);
	char *relocs=aRelocs;
	TUint page=0;
	TInt size=0;
	TInt i=vflipi(((E32RelocSection *)relocs)->iNumberOfRelocs); // re order -- we expect this in raw format
	relocs+=sizeof(E32RelocSection);
	while (i>0)
		{
		if (size>0)
			{
			TUint offset=*(TUint16 *)relocs;

			flipi(offset); // re order...

			relocs+=2;
			if (offset!=0)
				{ // its a reloc
				TUint va=page+(offset&0x0fff);
				TUint *dataptr=(TUint *)(aPtr+va);
				TUint data=*dataptr;

				flipi(data); // re order

				if (data>=iatStart && data<iatFinish)
					{
					TUint iatNum = (data-iatStart)/sizeof(TLinAddr);
					if ((TUint)aIATRefs[iatNum]>65535)
						Print(EWarning, "Multiple relocations for IAT entry %d (0x%x, 0x%x)\n",
								iatNum, aIATRefs[iatNum], dataptr);
					else
						aIATRefs[iatNum] = (TUint32) (aImagePtr+va);	// ROM image address of importing pointer
					// XXX Should we byte re order anything here???
					}
				else if (data>=codeStart && data<codeFinish)
					*dataptr=vflipi(data+aCodeDelta); // points to text/rdata section
				else
					*dataptr=vflipi(data+aDataDelta); // points to data section
				--i;
				}
			size-=2;
			}
		else
			{ // next page of relocs
			page=vflipi(*(TUint *)relocs);

			relocs+=4;
			size=vflipi(*(TUint *)relocs);
			relocs+=4;
			size-=8;
			}
		}
	}
