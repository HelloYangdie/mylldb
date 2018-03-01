#ifndef STORAGE_LEVELDB_DB_DBFORMAT_H_
#define STORAGE_LEVELDB_DB_DBFORMAT_H_

#include <string>
#include <stdio.h>
#include "include/leveldb/comparator.h"
#include "include/leveldb/db.h"
#include "include/leveldb/slice.h"

namespace leveldb
{

typedef uint64_t SequenceNumber;

enum ValueType 
{
	kTypeDeletion = 0x0,
	kTypeValue = 0x1
};

static const ValueType kValueTypeForSeek = kTypeValue;

//序列号低8为保留给ValueType用
static const SequenceNumber kMaxSequenceNumber = ((0x1ull << 56) - 1);

inline Slice ExtractUserKey(const Slice& internal_key) 
{
	assert(internal_key.size() >= 8);
	return Slice(internal_key.data(), internal_key.size() - 8);
}

class InternalKeyComparator : public Comparator
{
private:
	const Comparator* user_comparator_;
public:
	explicit InternalKeyComparator(const Comparator* c) : user_comparator_(c) { }

	virtual const char* Name() const;

	virtual int Compare(const Slice& a, const Slice& b) const;

	virtual void FindShortestSeparator(
		std::string* start,
		const Slice& limit) const;

	virtual void FindShortSuccessor(std::string* key) const;

	const Comparator* user_comparator() const { return user_comparator_; }

	//int Compare(const InternalKey& a, const InternalKey& b) const;
};

class LookupKey {
public:
	LookupKey(const Slice& user_key, SequenceNumber sequence);

	~LookupKey();

	// 返回一个在memtable使用的internalkey (internalkey长度 + internalkey)
	Slice memtable_key() const { return Slice(start_, end_ - start_); }

	// 返回internalkey(key + sequence/type)
	Slice internal_key() const { return Slice(kstart_, end_ - kstart_); }

	// 返回userkey
	Slice user_key() const { return Slice(kstart_, end_ - kstart_ - 8); }

private:
	const char* start_;
	const char* kstart_;
	const char* end_;
	char space_[200]; 

	// No copying allowed
	LookupKey(const LookupKey&);
	void operator=(const LookupKey&);
};

inline LookupKey::~LookupKey() {
	if (start_ != space_) delete[] start_;
}


}

#endif