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
	leveldb::Arena arena;
	StringComparator cmp;
	leveldb::SkipList<leveldb::Slice*, StringComparator> list(cmp, &arena);

	int size = 8;
	char* str = arena.Allocate(size);
	memset(str, 0x00, size);
	memcpy(str,"hell100",size);
	leveldb::Slice s1(str);
	list.Insert(&s1);

	str = arena.Allocate(size);
	memset(str, 0x00, size);
	memcpy(str,"hell10",size);
	leveldb::Slice s2(str);
	list.Insert(&s2);

	str = arena.Allocate(size);
	memset(str, 0x00, size);
	memcpy(str,"hell9",size);
	leveldb::Slice s3(str);
	list.Insert(&s3);

	str = arena.Allocate(size);
	memset(str, 0x00, size);
	memcpy(str,"hell20",size);
	leveldb::Slice s4(str);
	list.Insert(&s4);

	str = arena.Allocate(size);
	memset(str, 0x00, size);
	memcpy(str,"hell700",size);
	leveldb::Slice s5(str);
	list.Insert(&s5);

	str = arena.Allocate(size);
	memset(str, 0x00, size);
	memcpy(str,"hell110",size);
	leveldb::Slice s6(str);
	list.Insert(&s6);

	leveldb::SkipList<leveldb::Slice*, StringComparator>::Iterator it(&list);

	for (it.SeekToFirst(); it.Valid(); it.Next())
	{
		std::cout << it.key()->ToString() << ";";
	}

	std::cout << std::endl;
	
}
