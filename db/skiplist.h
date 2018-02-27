
#ifndef STORAGE_LEVELDB_DB_SKIPLIST_H_
#define STORAGE_LEVELDB_DB_SKIPLIST_H_

#include <assert.h>
#include <string>
#include "util/random.h"

namespace leveldb {

class SkipList
{
private:
	struct Node
	{
		int const key;

		explicit Node(int k) : key(k){}

		Node* Next(int n) { return next_[n];}

		void SetNext(int n, Node* node) { next_[n] = node;}

	private:
		Node* next_[1];
	};

public:
	explicit SkipList() : head_( NewNode( 0, kMaxHeight)),max_height_(1), rnd_(0xdeadbeef){}

	void Insert(int key)
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
	
	Node* const head_;
	int max_height_;
	Random rnd_;

	int GetMaxHeight() const { return max_height_;}
	
	Node* NewNode(int key, int height)
	{
		int size = sizeof(Node) + sizeof(Node*) * (height - 1);
		char* mem = new char[size];
		memset(mem, 0x00, size);

		return new (mem) Node(key);
	}

	int RandomHeight()
	{
		static const uint32_t kBranching = 4;
		int height = 1;

		while (height < kMaxHeight && ((rnd_.Next() % kBranching) == 0))
		{
			height++;
		}

		return height;
	}

	bool KeyIsAfterNode(int key, Node* node) const
	{
		return (node != NULL && (node->key < key));
	}

	Node* FindGreaterOrEqual(int key, Node** prev) const
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

	Node* FindLessThan(int key) const
	{
		Node* x= head_;
		int level = GetMaxHeight() - 1;

		while (true)
		{
			Node* next = x->Next(level);
			if (next == NULL || next->key >= key)
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

	Node* FindLast() const
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


	bool Equal(int a, int b) const { return a == b;}

	

	SkipList(const SkipList&);
	void operator=(const SkipList&);
};


}

#endif