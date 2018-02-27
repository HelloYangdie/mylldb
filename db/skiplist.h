
#ifndef STORAGE_LEVELDB_DB_SKIPLIST_H_
#define STORAGE_LEVELDB_DB_SKIPLIST_H_

#include <assert.h>
#include <string>
#include "util/random.h"

namespace leveldb {

template<typename Key, class Comparator>
class SkipList
{
private:
	struct Node;

public:
	explicit SkipList(Comparator cmp);

	void Insert(Key key);

	bool Contains(int key) const
	{
		Node* x = FindGreaterOrEqual(key, NULL);
		if (x != NULL && Equal(key, x->key))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	class Iterator {
	public:
		explicit Iterator(const SkipList* list)
		{
			list_ = list;
			node_ = NULL;
		}

		bool Valid() const { return node_ != NULL;}

		const int key() const { assert(Valid()); return node_->key;}

		void Next() { assert(Valid()); node_ = node_->Next(0);}

		void Prev() 
		{
			assert(Valid());
			node_ = list_->FindLessThan(node_->key);
			if (node_ == list_->head_)
			{
				node_ = NULL;
			}
		}

		void Seek(int target) {node_ = list_->FindGreaterOrEqual(target, NULL);}

		void SeekToFirst() {node_ = list_->head_->Next(0);}

		void SeekToLast() 
		{ 
			node_ = list_->FindLast();
			if (node_ == list_->head_)
			{
				node_ = NULL;
			}
		}

	private:
		const SkipList* list_;
		Node* node_;
	};

private:
	enum {kMaxHeight = 12};
	
	Comparator const compare_;
	Node* const head_;
	int max_height_;
	Random rnd_;

	int GetMaxHeight() const { return max_height_;}
	
	Node* NewNode(Key key, int height);

	int RandomHeight();

	bool KeyIsAfterNode(Key key, Node* node) const
	{
		return (node != NULL && (compare_(node->key, key) < 0));
	}

	Node* FindGreaterOrEqual(Key key, Node** prev) const;


	Node* FindLessThan(Key key) const;

	Node* FindLast() const;

	bool Equal(int a, int b) const { return compare_(a, b) == 0;}

	SkipList(const SkipList&);
	void operator=(const SkipList&);
};

template<typename Key, class Comparator>
struct SkipList<Key, Comparator>::Node
{
	Key const key;

	explicit Node(Key k) : key(k){}

	Node* Next(int n) { return next_[n];}

	void SetNext(int n, Node* node) { next_[n] = node;}
private:
	Node* next_[1];
};

template<typename Key, class Comparator>
SkipList<Key, Comparator>::SkipList(Comparator cmp) 
	 : head_(NewNode( 0, kMaxHeight)),
	 compare_(cmp),
	 max_height_(1), 
	 rnd_(0xdeadbeef)
{
	for (int i = 0; i < kMaxHeight; i++)
	{
		head_->SetNext(i, NULL);
	}
}

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert(Key key)
{
	Node* prev[kMaxHeight];
	Node* x = FindGreaterOrEqual(key, prev);
	int height = RandomHeight();
	if (height > GetMaxHeight())
	{
		for (int i = GetMaxHeight(); i < height; i++)
		{
			prev[i] = head_;
		}

		max_height_ = height;
	}

	x = NewNode(key, height);
	for (int i = 0; i < height; i++)
	{
		x->SetNext(i, prev[i]->Next(i));
		prev[i]->SetNext(i, x);
	}
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key, Comparator>::NewNode(Key key, int height)
{
	int size = sizeof(Node) + sizeof(Node*) * (height - 1);
	char* mem = new char[size];

	return new (mem) Node(key);
}

template<typename Key, class Comparator>
int SkipList<Key, Comparator>::RandomHeight()
{
	static const uint32_t kBranching = 4;
	int height = 1;

	while (height < kMaxHeight && ((rnd_.Next() % kBranching) == 0))
	{
		height++;
	}

	return height;
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key, Comparator>::FindGreaterOrEqual(Key key, Node** prev) const
{
	Node* x = head_;
	int level = GetMaxHeight() - 1;

	while (true)
	{
		Node* next = x->Next(level);
		if (KeyIsAfterNode(key, next))
		{
			x = next;
		}
		else
		{
			if (prev != NULL) prev[level] = x;
			if (level == 0) 
			{
				return next;
			}
			else
			{
				level--;
			}
		}
	}
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key, Comparator>::FindLessThan(Key key) const
{
	Node* x= head_;
	int level = GetMaxHeight() - 1;

	while (true)
	{
		Node* next = x->Next(level);
		if (next == NULL || compare_(next->key, key) >= 0)
		{
			if (level == 0)
			{
				return x;
			}
			else
			{
				level--;
			}
		}
		else
		{
			x = next;
		}
	}
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key, Comparator>::FindLast() const
{
	Node* x = head_;
	int level = GetMaxHeight() - 1;

	while (true)
	{
		Node* next = x->Next(level);
		if (next == NULL)
		{
			if (level == 0)
			{
				return x;
			}
			else
			{
				level--;
			}
		}
		else
		{
			x = next;
		}
	}
}

}

#endif