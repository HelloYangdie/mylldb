#include "include/leveldb/iterator.h"

namespace leveldb
{
Iterator::Iterator()
{
	cleanup_.function = NULL;
	cleanup_.next = NULL;
}

Iterator::~Iterator()
{
	if (cleanup_.function != NULL) {
		(*cleanup_.function)(cleanup_.arg1, cleanup_.arg2);
		for (Cleanup* c = cleanup_.next; c != NULL; ) {
			(*c->function)(c->arg1, c->arg2);
			Cleanup* next = c->next;
			delete c;
			c = next;
		}
	}
}

void Iterator::RegisterCleanup(CleanupFunction func, void* arg1, void* arg2) {
	assert(func != NULL);
	Cleanup* c;
	if (cleanup_.function == NULL) {
		c = &cleanup_;
	} else {
		c = new Cleanup;
		c->next = cleanup_.next;
		cleanup_.next = c;
	}
	c->function = func;
	c->arg1 = arg1;
	c->arg2 = arg2;
}

}