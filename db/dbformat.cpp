
#include <stdio.h>
#include "db/dbformat.h"
#include "util/coding.h"

namespace leveldb
{

static uint64_t PackSequenceAndType(uint64_t seq, ValueType t) {
	assert(seq <= kMaxSequenceNumber);
	assert(t <= kValueTypeForSeek);
	return (seq << 8) | t;
}

LookupKey::LookupKey(const Slice& user_key, SequenceNumber s) 
{
	size_t usize = user_key.size();
	size_t needed = usize + 8 + 5;
	char* dst;
	if (needed <= sizeof(space_)) 
	{
		dst = space_;
	} else 
	{
		dst = new char[needed];
	}

	start_ = dst;
	dst = EncodeVarint32(dst, usize + 8);
	kstart_ = dst;
	memcpy(dst, user_key.data(), usize);
	dst += usize;
	EncodeFixed64(dst, PackSequenceAndType(s, kValueTypeForSeek));
	dst += 8;
	end_ = dst;
}

const char* InternalKeyComparator::Name() const
{
	return "leveldb.InternalKeyComparator";
}

int InternalKeyComparator::Compare(const Slice& a, const Slice& b) const
{
	int r = user_comparator_->Compare(ExtractUserKey(a), ExtractUserKey(b));
	if (r == 0)
	{
		const uint64_t anum = DecodeFixed64(a.data() + a.size() - 8);
		const uint64_t bnum = DecodeFixed64(b.data() + b.size() - 8);
		if (anum >bnum)
		{
			r = -1;
		}
		else if (anum < bnum)
		{
			r = 1;
		}
	}
	return r;
}

void InternalKeyComparator::FindShortestSeparator(
	std::string* start, const Slice& limit) const
{
	Slice user_start = ExtractUserKey(*start);
	Slice user_limit = ExtractUserKey(limit);
	std::string tmp(user_start.data(), user_start.size());
	user_comparator_->FindShortestSeparator(&tmp, user_limit);
	if (tmp.size() < user_start.size() &&
		user_comparator_->Compare(user_start, tmp) < 0) {
			// User key has become shorter physically, but larger logically.
			// Tack on the earliest possible number to the shortened user key.
			PutFixed64(&tmp, PackSequenceAndType(kMaxSequenceNumber,kValueTypeForSeek));
			assert(this->Compare(*start, tmp) < 0);
			assert(this->Compare(tmp, limit) < 0);
			start->swap(tmp);
	}
}

void InternalKeyComparator::FindShortSuccessor(std::string* key) const
{
	// Find first character that can be incremented
	size_t n = key->size();
	for (size_t i = 0; i < n; i++) 
	{
		const uint8_t byte = (*key)[i];
		if (byte != static_cast<uint8_t>(0xff)) 
		{
			(*key)[i] = byte + 1;
			key->resize(i+1);
			return;
		}
	}
	// *key is a run of 0xffs.  Leave it alone.
}

}