// RCSCAN.CPP
//
// Copyright (c) 1997-1999 Symbian Ltd.  All rights reserved.
//

#include <assert.h>
#include "rcompl.hpp"
#include "rcscan.h"
#include "errorhan.h"
#include "mem.h"
#include <stdarg.h>  // for va_... used by yyerror
#include <stdio.h>   // for vsprintf... used by yerror


rcscan::rcscan(const FileLineManager& aFileLineHandlerToSet,FILE* aSourceFile):
	yy_scan(300),
	iErrorFound(0),
	iFileLineHandler(aFileLineHandlerToSet)
	{
	setinput(aSourceFile);
	}

rcscan& rcscan::operator=(const rcscan& /*scan*/)
	{
	assert(0);
	return *this;
	}

void rcscan::yyerror(const char* aCharPtr,...)
	{
	if(yylineno)
		{
		MOFF;
		cerr << iFileLineHandler.GetCurrentFile() << "(";
		cerr << iFileLineHandler.GetErrorLine(yylineno) << ") : ";
		MON;
		}

    // Format a string that can be sent via the iostream mechanism
    va_list va;
    char buffer[128];
    va_start(va,aCharPtr);
    vsprintf(buffer, aCharPtr, va );
    va_end(va);

	cerr << buffer << endl;
	iErrorFound = 1;
	}

int rcscan::ErrorWasFound() const
	{
	return iErrorFound;
	}
