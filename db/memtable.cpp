
#include "db/memtable.h"
#include "util/coding.h"

namespace leveldb
{

static Slice GetLengthPrefixedSlice(const char* data) {
	uint32_t len;
	const char* p = data;
	p = GetVarint32Ptr(p, p + 5, &len);  // +5: we assume "p" is not corrupted
	return Slice(p, len);
}

static const char* EncodeKey(std::string* scratch, const Slice& target) {
	scratch->clear();
	PutVarint32(scratch, target.size());
	scratch->append(target.data(), target.size());
	return scratch->data();
}

class MemTableIterator : public Iterator
{
public:
	explicit MemTableIterator(MemTable::Table* table) : iter_(table) { }

	virtual bool Valid() const{ return iter_.Valid();}

	virtual void SeekToFirst() {iter_.SeekToFirst();}

	virtual void SeekToLast() { iter_.SeekToLast(); }

	virtual void Seek(const Slice& target) { iter_.Seek(EncodeKey(&tmp_, target));}

	virtual void Next() { iter_.Next(); }

	virtual void Prev() { iter_.Prev(); }

	virtual Slice key() const  { return GetLengthPrefixedSlice(iter_.key()); }

	virtual Slice value() const {
		Slice key_slice = GetLengthPrefixedSlice(iter_.key());
		return GetLengthPrefixedSlice(key_slice.data() + key_slice.size());
	}

	virtual Status status() const { return Status::OK(); }
private:
	MemTable::Table::Iterator iter_;
	std::string tmp_;
};

MemTable::MemTable(const InternalKeyComparator& cmp) 
	: table_(comparator_, &arena_),
	comparator_(cmp)
{

}

size_t MemTable::ApproximateMemoryUsage()
{
	return arena_.MemoryUsage();
}

/*
skiplist�洢���ݵĽṹΪ��internalkey���� + internalkey + value���� + value
internalkey = key + (SequenceNumber + valuetype)
(SequenceNumber + valuetype)ռ8���ֽ�
���ڳ���ȫ��ת�����ַ��������ʽ����7bit��Чλ�ָ�
*/
void MemTable::Add(SequenceNumber s, ValueType t, const Slice& key, const Slice& value)
{
	size_t key_size = key.size();
	size_t val_size = value.size();
	size_t internal_key_size = key_size + 8;

	size_t encoded_len = 
		VarintLength(internal_key_size) + internal_key_size + 
		VarintLength(val_size) + val_size;

	char* buf = arena_.Allocate(encoded_len);
	/* ����internal key */
	char* p = EncodeVarint32(buf, internal_key_size);
	memcpy(p, key.data(), key_size);
	p += key_size;
	EncodeFixed64(p, (s << 8) | t);

	/* ����value */
	p += 8;
	p = EncodeVarint32(p, val_size);
	memcpy(p, value.data(), val_size);
	assert((p + val_size) - buf == encoded_len);
	table_.Insert(buf);
}

int MemTable::KeyComparator::operator()(const char* aptr, const char* bptr)const 
{
	// Internal keys are encoded as length-prefixed strings.
	Slice a = GetLengthPrefixedSlice(aptr);
	Slice b = GetLengthPrefixedSlice(bptr);
	return comparator.Compare(a, b);
}

bool MemTable::Get(const LookupKey& key, std::string* value, Status* s)
{
	Slice memkey = key.memtable_key();
	Table::Iterator it(&table_);

	it.Seek(memkey.data());
	if (it.Valid())
	{
		const char* entry = it.key();
		uint32_t key_length;
		const char* key_ptr = GetVarint32Ptr(entry, entry+5, &key_length);
		if (comparator_.comparator.user_comparator()->Compare(
			Slice(key_ptr, key_length - 8), key.user_key()) == 0)
		{
			const uint64_t tag = DecodeFixed64(key_ptr + key_length - 8);
			switch (static_cast<ValueType>(tag & 0xff))
			{
			case kTypeValue:
				{
					Slice v = GetLengthPrefixedSlice(key_ptr + key_length);
					value->assign(v.data(), v.size());
					return true;
				}
			case kTypeDeletion:
				*s = Status::NotFound(Slice());
				return true;
			}
		}
	}

	return false;
}

Iterator* MemTable::NewIterator()
{
	return new MemTableIterator(&table_);
}

}