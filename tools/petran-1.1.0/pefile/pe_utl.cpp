// PE_UTL.CPP
//
// Copyright (c) 1996-1999 Symbian Ltd.  All rights reserved.
//

#include <e32std.h>
#include <pe_file.h>
#include <string.h>
#include <e32image.h>

TInt PEFile::CmpSectionName(PIMAGE_SECTION_HEADER apSectionHeader, char *aName)
//
// Returns true if the name of the pe section is the same as aName
//
	{

	return (strncasecmp((const char *)apSectionHeader->Name, aName, IMAGE_SIZEOF_SHORT_NAME)==0);
	}

TInt PEFile::VirtualAddressInSection(TUint aVA, PIMAGE_SECTION_HEADER aHeader)
//
// Returns true if the virtual address is in the section
//
	{

	TUint start=aHeader->VirtualAddress;
	TUint finish=start+aHeader->Misc.VirtualSize;
	if ((aVA>=start) && (aVA<finish))
		return TRUE;
	return FALSE;
	}

TInt PEFile::FindSectionByVa(TUint aVa, PIMAGE_SECTION_HEADER *peSectionHeader)
	{

	TInt i,s=-1;
	for (i=0; i<KNumberOfSections; i++)
		if (peSectionHeader[i])
			if (VirtualAddressInSection(aVa, peSectionHeader[i]))
				s=i;
	return s;
	}
