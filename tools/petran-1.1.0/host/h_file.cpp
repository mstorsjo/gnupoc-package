// H_FILE.CPP
//
// Copyright (c) 1995-1999 Symbian Ltd.  All rights reserved.
//


#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <h_utl.h>
#include <unistd.h>
#include <string.h>

TInt HFile::Open(const TText * const aFileName, TInt32 * const aFileHandle)
	{
	TInt32 hFile = open((const char *)aFileName, O_RDONLY);
	if (hFile == -1)
		{
		switch (errno)
			{
		case EACCES:
			Print(EError,"Can't open file %s - access violation.\n",aFileName);
			break;
		case EEXIST:
			Print(EAlways,"Tried to create existing file %s\n",aFileName);
			break;
		case EINVAL:
			Print(EError,"Can't open file %s - invalid open flags.\n",aFileName);
			break;
		case EMFILE:
			Print(EError,"Can't open file %s - too many open files.\n",aFileName);
			break;
		case ENOENT:
			Print(EError,"Can't open file %s - file or path not found.\n",aFileName);
			break;
		default:
			Print(EError,"Can't open file %s, error %d (decimal).\n",aFileName,errno);
			break;
			}
		return errno;
		}
 	*aFileHandle = hFile;
	return 0;
	}	

/******************************************************************************/
TBool HFile::Read(const TInt32 aFileHandle, TAny * const aBuffer, const TUint32 aCount)
	{
	TInt32 bytesRead =  read(aFileHandle, aBuffer, aCount);
	if (bytesRead != (TInt32)aCount)
		return EFalse;
	else
		return ETrue;
	}  

/******************************************************************************/
TBool HFile::Seek(const TInt32 aFileHandle, const TUint32 aOffset)
	{
	TInt32 newPos = lseek(aFileHandle, aOffset, SEEK_SET);
	if (newPos == -1)
		return(EFalse);
	else
		return ETrue;
	}

/******************************************************************************/
TUint32 HFile::GetPos(const TInt32 aFileHandle)
	{
	TUint32 pos = lseek(aFileHandle, 0, SEEK_CUR);
	return pos;
	}

/******************************************************************************/
TUint32 HFile::GetLength(TText *aName)
	{
#if 0
	TInt32 handle;
	if (HFile::Open(aName, &handle)==0)
		{
		TUint32 size = _filelength(handle);
		HFile::Close(handle);
		return size;
		}
	else
		return 0;
#else
	struct stat s;
	const int ret = stat((char *)aName, &s);
	if (ret==-1) 
		{
		Print(EError,"Cannot get length of %s (%s).\n",aName, strerror(errno));
		return 0;
		}
	return s.st_size;
#endif
	}


/******************************************************************************/
void HFile::Close(const TInt32 aFileHandle)
	{
	close(aFileHandle);
	}


/******************************************************************************/
TUint32 HFile::Read(TText *aName, TAny *someMem)
 	{
	TInt32 handle;
	const TUint32 size = HFile::GetLength(aName);
	if (HFile::Open(aName, &handle)==0)
		{
		if (HFile::Read(handle, someMem, size))
			{
			HFile::Close(handle);
			return size;
			}
		else 
			{
			HFile::Close(handle);
			return 0;
			}
		}
	else
		return 0;
	}


/******************************************************************************/
