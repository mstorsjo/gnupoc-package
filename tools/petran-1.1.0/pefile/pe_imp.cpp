// PE_IMP.CPP
//
// Copyright (c) 1996-1999 Symbian Ltd.  All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <e32std.h>
#include <h_utl.h>
#include "e32image.h"
#include "h_endian.h"

TInt E32ImportBlock::Size()
//
// return the size of this block
//
	{
	return sizeof(E32ImportBlock)+iNumberOfImports*sizeof(TUint);
	}

TInt E32ImageFile::CopyImportAddrTable(char *aPtr, PEFile &aPeFile)
//
// Copy the import address table entries
//
	{

	TUint *ptr=(TUint *)aPtr;
	char *importsection=aPeFile.iSectionData[KImportSection];
	TUint *src=(TUint *)importsection;
	while (*src)
		{
		TUint vaoffset=src[4];

		flipi(vaoffset); // re order.
		
		TUint offset=vaoffset-aPeFile.iSectionHeader[KImportSection]->VirtualAddress; // find the offset into the section of import addr table
		vaoffset=src[3];

		flipi(vaoffset); // re order.

		TUint exportername=vaoffset-aPeFile.iSectionHeader[KImportSection]->VirtualAddress;
		TUint *p=(TUint *)(importsection+offset);
		while (*p)
			{
			  // Note the byte re order of the masks below...
			  if ((*p&vflipi(0x80000000))==0)
				{
				Print(EError, "%s exporting symbol by name\n", importsection+exportername);
				return KErrGeneral;
				}
			*ptr++=(*p++)&vflipi(0x7fffffff); // mask out the high bit (- indicates export by ordinal)
			}
		src+=5;
		}
	*ptr++=0;
	return KErrNone;
	}

char *E32ImageFile::CreateImportSection(const PEFile &aPeFile, TInt &aSize)
//
// Create a new format import section
//
	{

	PIMAGE_SECTION_HEADER aHeader=aPeFile.iSectionHeader[KImportSection];
	TUint *aSrc=(TUint *)aPeFile.iSectionData[KImportSection];

	TInt nimportdlls=aPeFile.NumberOfImportDlls();
	if (nimportdlls==0)
		{
		aSize=0;
		return NULL;
		}
	E32ImportBlock *block=new E32ImportBlock [nimportdlls];
	char **name=new char* [nimportdlls];
	TUint **import=new TUint* [nimportdlls];

	TInt bytecount=sizeof(E32ImportSection)+sizeof(E32ImportBlock)*nimportdlls;
	TUint *src=aSrc;
	TInt i;
	for (i=0; i<nimportdlls; i++)
		{
		TUint vaoffset=src[4];
		
		flipi(vaoffset); // re order it
		
		TUint offset=vaoffset-aHeader->VirtualAddress; // find the offset into the section of import addr table
		TUint *p=aSrc+offset/4;
		block[i].iNumberOfImports=0;
		while (*p++) // skip re order, since 0 is unchanged
			block[i].iNumberOfImports++;
		import[i]=new TUint [block[i].iNumberOfImports];
		TInt j;
		p=aSrc+offset/4;
		for (j=0; j<block[i].iNumberOfImports; j++)
			{
			import[i][j]=(*p++)&vflipi(0x7fffffffu);
			bytecount+=4;
			}
		// name
		vaoffset=src[3];
		flipi(vaoffset); // re order it
		
		offset=vaoffset-aHeader->VirtualAddress;
		name[i]=((char *)aSrc)+offset;
		bytecount+=strlen(name[i])+1;
		src+=5;
		}

	bytecount=ALIGN4(bytecount);
	char *section=new char [bytecount];
	char *s=section+sizeof(E32ImportSection);
	for (i=0; i<nimportdlls; i++)
		{
		  E32ImportBlock x = block[i];
		  
		  flipE32ImportBlock(x); 
		  memcpy(s, (char *)&x, sizeof(E32ImportBlock));
		  s+=sizeof(E32ImportBlock);
		  memcpy(s, (char *)import[i], block[i].iNumberOfImports*4);
		  s+=block[i].iNumberOfImports*4;
		}
	char *t=section+sizeof(E32ImportSection);
	for (i=0; i<nimportdlls; i++)
		{
		((E32ImportBlock *)t)->iOffsetOfDllName=vflipi(s-section); // Re order before putting in
		strcpy(s, name[i]);
		s+=strlen(name[i])+1;
		t+= sizeof(E32ImportBlock) + block[i].iNumberOfImports*4; // skip to next
		}
	while ((s-section)<bytecount)
		*s++=0;

	// free mem
	for (i=0; i<nimportdlls; i++)
		delete import[i];
	delete block;
	delete import;
	delete name;

	aSize=bytecount;
	*(TUint *)section=vflipi(bytecount);
	return section;
	}

TUint E32ImageFile::FixImportThunk(PEFile &aPeFile, TUint va)
//
// Fix an access to the import address table
//
	{

	TUint *imports=(TUint *)aPeFile.iSectionData[KImportSection];
	TUint n=0;
	TUint importoffset=imports[4];

	flipi(importoffset); // re order
	TUint iat=importoffset-aPeFile.iSectionHeader[KImportSection]->VirtualAddress;

	while (iat!=va)
		{
		if (*((TUint *)(aPeFile.iSectionData[KImportSection]+iat))==0)
			{
			imports+=5;
			importoffset=imports[4];
			flipi(importoffset); // re order
			iat=importoffset-aPeFile.iSectionHeader[KImportSection]->VirtualAddress;
			}
		else
			{
			n++;
			iat+=4;
			}
		}
	return iHeader->iTextSize+n*sizeof(TUint);
	}

TInt E32ImageFile::NumberOfImports()
//
// Return the number of imports made by this image
//
	{

	if (iHeader->iDllRefTableCount==0)
		return 0;
	TUint *imports=(TUint *)(iData+iHeader->iCodeOffset+iHeader->iTextSize);
	TInt i=0;
	while (*imports++) // No need for re order
		i++;
	return i;
	}
