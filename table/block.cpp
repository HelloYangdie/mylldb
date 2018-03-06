#include "block.h"
#include "table/format.h"
#include "util/coding.h"
#include "include/leveldb/comparator.h"

namespace leveldb {

Block::Block(const struct BlockContents& contents)
	: data_(contents.data.data()),
	  size_(contents.data.size()),
	  owned_(contents.heap_allocated)
{
	if (size_ < sizeof(uint32_t))
	{
		size_ = 0;
	}
	else
	{
		size_t max_restarts_allowd = (size_ - sizeof(uint32_t)) / sizeof(uint32_t);
		if (NumRestarts() > max_restarts_allowd)
		{
			size_ = 0;
		}
		else
		{
			restart_offset_ = size_ - (1 + NumRestarts()) * sizeof(uint32_t);
		}
	}
}

inline uint32_t Block::NumRestarts() const {
	assert(size_ >= sizeof(uint32_t));
	return DecodeFixed32(data_ + size_ - sizeof(uint32_t));
}

static inline const char* DecodeEntry(const char* p, 
	const char* limit, uint32_t* shared, uint32_t* non_shared, uint32_t* value_length)
{
	if (limit - p < 3) return NULL;
	*shared       = (reinterpret_cast<const unsigned char*>(p))[0];
	*non_shared   = (reinterpret_cast<const unsigned char*>(p))[1];
	*value_length = (reinterpret_cast<const unsigned char*>(p))[2];
	if ((*shared | *non_shared | *value_length) < 128)
	{
		p += 3;
	}
	else
	{
		if ((p = GetVarint32Ptr(p, limit, shared)) == NULL) return NULL;
		if ((p = GetVarint32Ptr(p, limit, non_shared)) == NULL) return NULL;
		if ((p = GetVarint32Ptr(p, limit, value_length)) == NULL) return NULL;
	}

	if (static_cast<uint32_t>(limit - p) < (*non_shared + *value_length)) {
		return NULL;
	}
	return p;
}

class Block::Iter : public Iterator
{
private:
	const Comparator* const comparator_;
	const char* const data_;
	uint32_t const restarts_; //重启点偏移
	uint32_t const num_restarts_; //重启点的个数

	uint32_t current_; //当前重启点的偏移位置
	uint32_t restart_index_; //重启点的索引
	std::string key_;
	Slice value_;
	Status status_;

	inline int Compare(const Slice& a, const Slice& b) const 
	{
		return comparator_->Compare(a, b);
	}

	inline uint32_t NextEntryOffset() const 
	{
		return (value_.data() + value_.size() - data_);
	}

	uint32_t GetRestartPoint(uint32_t index)
	{
		assert(index < num_restarts_);
		return DecodeFixed32(data_ + restarts_ + index*sizeof(uint32_t));
	}

	void SeekToRestartPoint(uint32_t index)
	{
		key_.clear();
		restart_index_ = index;

		uint32_t offset = GetRestartPoint(index);
		value_ = Slice(data_ + offset, 0);
	}

public:
	Iter(const Comparator* cmp, const char* data, uint32_t restarts, uint32_t num_restarts)
		: comparator_(cmp), data_(data), restarts_(restarts), 
		num_restarts_(num_restarts), current_(restarts),restart_index_(num_restarts)
	{
		assert(num_restarts_ > 0);
	}

	virtual bool Valid() const { return current_ < restarts_;}
	
	virtual Status status() const { return status_;}

	virtual Slice key() const { assert(Valid()); return key_;}

	virtual Slice value() const { assert(Valid()); return value_;}

	virtual void Next() { ParseNextKey(); }

	virtual void Prev() 
	{
		assert(Valid());
		const uint32_t original = current_;
		while (GetRestartPoint(restart_index_) >= current_)
		{
			if (restart_index_ == 0)
			{
				current_ = restarts_;
				restart_index_ = num_restarts_;
				return;
			}
			restart_index_--;
		}

		SeekToRestartPoint(restart_index_);
		do {
			// Loop until end of current entry hits the start of original entry
		} while (ParseNextKey() && NextEntryOffset() < original);
	}

	virtual void Seek(const Slice& target)
	{
		// Binary search in restart array to find the last restart point
		// with a key < target
		uint32_t left = 0;
		uint32_t right = num_restarts_ - 1;
		while (left < right) {
			uint32_t mid = (left + right + 1) / 2;
			uint32_t region_offset = GetRestartPoint(mid);
			uint32_t shared, non_shared, value_length;
			const char* key_ptr = DecodeEntry(data_ + region_offset,
				data_ + restarts_,
				&shared, &non_shared, &value_length);
			if (key_ptr == NULL || (shared != 0)) {
				CorruptionError();
				return;
			}
			Slice mid_key(key_ptr, non_shared);
			if (Compare(mid_key, target) < 0) {
				// Key at "mid" is smaller than "target".  Therefore all
				// blocks before "mid" are uninteresting.
				left = mid;
			} else {
				// Key at "mid" is >= "target".  Therefore all blocks at or
				// after "mid" are uninteresting.
				right = mid - 1;
			}
		}

		// Linear search (within restart block) for first key >= target
		SeekToRestartPoint(left);
		while (true) {
			if (!ParseNextKey()) {
				return;
			}
			if (Compare(key_, target) >= 0) {
				return;
			}
		}
	}

	virtual void SeekToFirst() 
	{
		SeekToRestartPoint(0);
		ParseNextKey();
	}

	virtual void SeekToLast()
	{
		SeekToRestartPoint(num_restarts_ - 1);
		while(ParseNextKey() && NextEntryOffset() < restarts_);
	}

private:
	void CorruptionError() 
	{
		current_ = restarts_;
		restart_index_ = num_restarts_;
		status_ = Status::Corruption("bad entry in block");
		key_.clear();
		value_.clear();
	}

	bool ParseNextKey()
	{
		current_ = NextEntryOffset();
		const char* p = data_ + current_;
		const char* limit = data_ + restarts_;
		if (p >= limit)
		{
			current_ = restarts_;
			restart_index_ = num_restarts_;
			return false;
		}

		uint32_t shared, non_shared, value_length;
		p = DecodeEntry(p, limit, &shared, &non_shared, &value_length);
		if (p == NULL || key_.size() < shared)
		{
			CorruptionError();
			return false;
		}
		else
		{
			key_.resize(shared);
			key_.append(p, non_shared);
			value_ = Slice(p + non_shared, value_length);
			while (restart_index_ + 1 < num_restarts_ &&
				GetRestartPoint(restart_index_ + 1) < current_) {
					++restart_index_;
			}
			return true;
		}
	}
};

Iterator* Block::NewIterator(const Comparator* comparator)
{
	if (size_ < sizeof(uint32_t))
	{
		return NewErrorIterator(Status::Corruption("bad block contents"));
	}
	const uint32_t num_restarts = NumRestarts();
	if (num_restarts == 0)
	{
		return NewEmptyIterator();
	}
	else
	{
		return new Iter(comparator, data_, restart_offset_, num_restarts);
	}
}

}
