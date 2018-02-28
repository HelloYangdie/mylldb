#include <iostream>
#include "db/skiplist.h"
#include "include/leveldb/slice.h"

struct StringComparator
{
	int operator()(const leveldb::Slice* a, const leveldb::Slice* b) const
	{
		return a->compare(*b);
	}
};

void SkipListTest()
{
	StringComparator cmp;
	leveldb::SkipList<leveldb::Slice*, StringComparator> list(cmp);

	char* str = "hell100";
	list.Insert(new leveldb::Slice(str));
	str = "hell10";
	list.Insert(new leveldb::Slice(str));
	str = "hell9";
	list.Insert(new leveldb::Slice(str));
	str = "hell20";
	list.Insert(new leveldb::Slice(str));
	str = "hell700";
	list.Insert(new leveldb::Slice(str));
	str = "hell110";
	list.Insert(new leveldb::Slice(str));
	str = "hell19";
	list.Insert(new leveldb::Slice(str));
	str = "hell300";
	list.Insert(new leveldb::Slice(str));
	str = "hell200";
	list.Insert(new leveldb::Slice(str));
	str = "hell111";
	list.Insert(new leveldb::Slice(str));

	leveldb::SkipList<leveldb::Slice*, StringComparator>::Iterator it(&list);

	for (it.SeekToFirst(); it.Valid(); it.Next())
	{
		std::cout << it.key()->ToString() << ";";
	}

	std::cout << std::endl;
	
}
