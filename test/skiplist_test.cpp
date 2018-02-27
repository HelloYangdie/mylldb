#include <iostream>
#include "db/skiplist.h"

void SkipListTest()
{
	leveldb::SkipList list;

	list.Insert(100);
	list.Insert(10);
	list.Insert(9);
	list.Insert(20);
	list.Insert(700);
	list.Insert(110);
	list.Insert(19);
	list.Insert(300);
	list.Insert(200);
	list.Insert(111);

	leveldb::SkipList::Iterator it(&list);

	for (it.SeekToFirst(); it.Valid(); it.Next())
	{
		std::cout << it.key() << ";";
	}

	std::cout << std::endl;
	
}
