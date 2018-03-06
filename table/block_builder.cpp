
#include "table/block_builder.h"

#include <algorithm>
#include <assert.h>
#include "include/leveldb/comparator.h"
#include "include/leveldb/options.h"
#include "util/coding.h"

namespace leveldb
{

BlockBuilder::BlockBuilder(const struct Options* options)
	: options_(options),
	counter_(0),
	finished_(false)
{
	assert(options->block_restart_interval >= 1);
	restarts_.push_back(0);
}

void BlockBuilder::Reset()
{
	buffer_.clear();
	restarts_.clear();
	restarts_.push_back(0);
	counter_ = 0;
	finished_ = false;
	last_key_.clear();
}

void BlockBuilder::Add(const Slice& key, const Slice& value)
{
	Slice last_key_piece(last_key_);
	size_t shared = 0;
	if (counter_ < options_->block_restart_interval)
	{
		const size_t min_length = std::min(last_key_piece.size(), key.size());
		while ((shared < min_length) && (last_key_piece[shared] == key[shared]))
		{
			shared++;
		}
	}
	else
	{
		restarts_.push_back(buffer_.size());
		counter_ = 0;
	}

	const size_t non_shared = key.size() - shared;
	PutVarint32(&buffer_, shared);
	PutVarint32(&buffer_, non_shared);
	PutVarint32(&buffer_, value.size());
	buffer_.append(key.data() + shared, non_shared);
	buffer_.append(value.data(), value.size());

	last_key_.resize(shared);
	last_key_.append(key.data() + shared, non_shared);
	assert(Slice(last_key_) == key);
	counter_++;
}

Slice BlockBuilder::Finish() 
{
	// Append restart array
	for (size_t i = 0; i < restarts_.size(); i++) 
	{
		PutFixed32(&buffer_, restarts_[i]);
	}

	PutFixed32(&buffer_, restarts_.size());
	finished_ = true;
	return Slice(buffer_);
}

size_t BlockBuilder::CurrentSizeEstimate() const 
{
	return (buffer_.size() +                        // Raw data buffer
		restarts_.size() * sizeof(uint32_t) +   // Restart array
		sizeof(uint32_t));                      // Restart array length
}

}