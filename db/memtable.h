#ifndef STORAGE_LEVELDB_DB_MEMTABLE_H_
#define STORAGE_LEVELDB_DB_MEMTABLE_H_

#include <string.h>
#include "db/dbformat.h"
#include "db/skiplist.h"
#include "include/leveldb/db.h"

namespace leveldb
{

class MemTableIterator;

class MemTable
{
public:
	explicit MemTable(const InternalKeyComparator& cmp);
	~MemTable(){}

	size_t ApproximateMemoryUsage();

	void Add(SequenceNumber s, ValueType t, const Slice& key, const Slice& value);

	bool Get(const LookupKey& key, std::string* value, Status* s);

	Iterator* NewIterator();

private:
	struct KeyComparator
	{
		const InternalKeyComparator comparator;
		explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) { }
		int operator()(const char* a, const char* b) const;
	};

	friend class MemTableIterator;

	typedef SkipList<const char*, KeyComparator> Table;

	KeyComparator comparator_;
	Arena arena_;
	Table table_;
};

}

#endif