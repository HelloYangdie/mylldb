
#include <algorithm>
#include <iostream>
#include "db/memtable.h"
#include "include/leveldb/slice.h"
#include "include/leveldb/comparator.h"
#include "include/leveldb/status.h"

using namespace leveldb;

void MemTableTest()
{
	InternalKeyComparator comparator(BytewiseComparator());

	MemTable table(comparator);

	std::string key1 = "key11";
	std::string val1 = "value11";
	table.Add(1, kTypeValue, key1, val1);

	std::string key12 = "key12";
	std::string val12 = "value12";
	table.Add(12, kTypeValue, key12, val12);

	std::string key20 = "key20";
	std::string val20 = "value20";
	table.Add(20, kTypeValue, key20, val20);

	LookupKey look(key1, 13);
	std::string value;
	Status s;
	table.Get(look, &value, &s);
	std::cout << "get key:" << key1 << " value is " << value << std::endl;

	Iterator* it = table.NewIterator();
	for (it->SeekToFirst(); it->Valid(); it->Next())
	{
		std::cout << it->value().ToString() << ",";
	}
	std::cout << std::endl;
}