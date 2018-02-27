#include <iostream>
#include "db/skiplist.h"

struct IntComparator
{
	int operator()(int a, int b) const
	{
		if (a > b) return 1;
		else if (a< b) return -1;
		else return 0;
	}
};

void SkipListTest()
{
	IntComparator cmp;
	leveldb::SkipList<int, IntComparator> list(cmp);

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

	leveldb::SkipList<int, IntComparator>::Iterator it(&list);

	for (it.SeekToFirst(); it.Valid(); it.Next())
	{
		std::cout << it.key() << ";";
	}

	std::cout << std::endl;
	
}
