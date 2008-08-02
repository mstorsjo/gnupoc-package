// STRUCTST.H
//
// Copyright (c) 1997-1999 Symbian Ltd.  All rights reserved.
//

#ifndef __STRUCTST_H__
#define __STRUCTST_H__

#include <iostream>
#include "stringar.h"
#include "datatype.h"

using namespace std;

class ResourceItem;

// StructItem

class StructItem : public ArrayItem
	{
public:
	virtual ResourceItem * NewResourceItem() = 0;
	virtual ostream& StreamOut ( ostream & os) = 0;
protected:
	StructItem( String LabelToSet);
	virtual ~StructItem();
	StructItem( StructItem & Source);
public:
	String	iLabel;
	};

// SimpleStructItem

class SimpleStructItem : public StructItem	// e.g. WORD a = 5
	{
	friend ostream& operator<< ( ostream & os, SimpleStructItem & o);
public:
	SimpleStructItem(DataType aItemTypeToSet,String aLabelToSet);
	SimpleStructItem(DataType aItemTypeToSet,String aLabelToSet,String aMaxLength);
	ResourceItem * NewResourceItem();
	ostream& StreamOut ( ostream & os);
	virtual ~SimpleStructItem();
	SimpleStructItem( SimpleStructItem & Source);
	SimpleStructItem& operator=( SimpleStructItem & Source);
public:
	DataType iItemType;
	String iDefault;
	String iLengthLimit;
	};

ostream& operator<< ( ostream & os, SimpleStructItem & o);

// ArrayStructItem

class ArrayStructItem : public StructItem	// e.g. WORD a[] = {1,2,3}
	{
	friend ostream& operator<< ( ostream & os, ArrayStructItem & o);
public:
	ArrayStructItem( DataType ItemTypeToSet, String LabelToSet);
	virtual ~ArrayStructItem();
	ResourceItem * NewResourceItem();
	ostream& StreamOut ( ostream & os);
public:
	DataType iItemType;
	StringArray iDefaults;
	DataType iLenType;
	String iSize;
	};

ostream& operator<< ( ostream & os, ArrayStructItem & o);

// StructTypeStructItem

class StructTypeStructItem : public StructItem	// e.g. STRUCT a
	{
	friend ostream& operator<< ( ostream & os, StructTypeStructItem & o);
public:
	StructTypeStructItem( String LabelToSet);
	ResourceItem * NewResourceItem();
	ostream& StreamOut ( ostream & os);
	};

ostream& operator<< ( ostream & os, StructTypeStructItem & o);

// StructArrayStructItem

class StructArrayStructItem : public StructItem // e.g. STRUCT a[]
	{
	friend ostream& operator<< ( ostream & os, StructArrayStructItem & o);
public:
	StructArrayStructItem( String LabelToSet);
	StructArrayStructItem( String LabelToSet, String SizeToSet);
	ResourceItem * NewResourceItem();
	ostream& StreamOut ( ostream & os);
	virtual ~StructArrayStructItem();
public:
	DataType iLenType;
	String iSize;
	};

ostream& operator<< ( ostream & os, StructArrayStructItem & o);

// StructItemArray

class StructItemArray : public Array
	{
	friend ostream& operator<< ( ostream & os, StructItemArray & o);
public:
	StructItemArray();
	~StructItemArray();
	void Add( StructItem * pNewItem);
	};

ostream& operator<< ( ostream & os, StructItemArray & o);

// StructItemArrayIterator

class StructItemArrayIterator : public ArrayIterator
	{
public:
	StructItemArrayIterator( const StructItemArray & c);
	StructItem * operator() ();
	};

// StructHeader

class StructHeader : public ArrayItem
	{
	friend ostream& operator<< ( ostream & os, StructHeader & o);
public:
	StructHeader( String LabelToSet);
	StructHeader( String LabelToSet, DataType LenTypeToSet);
public:
	String iLabel;
	DataType iLenType;
	StructItemArray iSIA;
	};

ostream& operator<< ( ostream & os, StructHeader & o);

// StructHeaderArray

class StructHeaderArray : public Array
	{
	friend ostream& operator<< ( ostream & os, StructHeaderArray & o);
public:
	StructHeaderArray();
	~StructHeaderArray();
	void Add( StructHeader * pNewItem);
	StructHeader * Find( const String & LabelSought);
private:
	static int iInUse;	// Only one instance of this class may exist at a time.
	};

ostream& operator<< ( ostream & os, StructHeaderArray & o);

// StructHeaderArrayIterator

class StructHeaderArrayIterator : public ArrayIterator
	{
public:
	StructHeaderArrayIterator( const StructHeaderArray& c);
	StructHeader * operator() ();
	};

#endif

