#include "util/coding.h"

namespace leveldb 
{

int VarintLength(uint64_t v) 
{
	int len = 1;
	while (v >= 128) {
		v >>= 7;
		len++;
	}
	return len;
}

void PutFixed32(std::string* dst, uint32_t value) {
	char buf[sizeof(value)];
	EncodeFixed32(buf, value);
	dst->append(buf, sizeof(buf));
}

void PutFixed64(std::string* dst, uint64_t value) {
	char buf[sizeof(value)];
	EncodeFixed64(buf, value);
	dst->append(buf, sizeof(buf));
}

void PutVarint32(std::string* dst, uint32_t v) {
	char buf[5];
	char* ptr = EncodeVarint32(buf, v);
	dst->append(buf, ptr - buf);
}

void PutVarint64(std::string* dst, uint64_t v) {
	char buf[10];
	char* ptr = EncodeVarint64(buf, v);
	dst->append(buf, ptr - buf);
}

void PutLengthPrefixedSlice(std::string* dst, const Slice& value) {
	PutVarint32(dst, value.size());
	dst->append(value.data(), value.size());
}

char* EncodeVarint32(char* dst, uint32_t v)
{
	unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
	static const int B = 128;
	if (v < (1<<7)) {
		*(ptr++) = v;
	} else if (v < (1<<14)) {
		*(ptr++) = v | B;
		*(ptr++) = v>>7;
	} else if (v < (1<<21)) {
		*(ptr++) = v | B;
		*(ptr++) = (v>>7) | B;
		*(ptr++) = v>>14;
	} else if (v < (1<<28)) {
		*(ptr++) = v | B;
		*(ptr++) = (v>>7) | B;
		*(ptr++) = (v>>14) | B;
		*(ptr++) = v>>21;
	} else {
		*(ptr++) = v | B;
		*(ptr++) = (v>>7) | B;
		*(ptr++) = (v>>14) | B;
		*(ptr++) = (v>>21) | B;
		*(ptr++) = v>>28;
	}
	return reinterpret_cast<char*>(ptr);
}

/*
static_cast
	a) 用于类层次结构中基类和派生类之间指针或者引用的转换。
	up-casting (把派生类的指针或引用转换成基类的指针或者引用表示)是安全的；
	down-casting(把基类指针或引用转换成子类的指针或者引用)是不安全的。
	b) 用于基本数据类型之间的转换，如把int转换成char，这种转换的安全性也要由开发人员来保证。
	c) 可以把空指针转换成目标类型的空指针(null pointer)。
	d) 把任何类型的表达式转换成void类型。
reinterpret_cast:
	转换一个指针为其他类型的指针，也允许将一个指针转换为整数类型，反之亦然。
	这个操作符能够在非相关的类型之间进行转换。
	操作结果只是简单的从一个指针到别的指针的值的二进制拷贝，
	在类型之间指向的内容不做任何类型的检查和转换。这是一个强制转换。
*/
char* EncodeVarint64(char* dst, uint64_t v)
{
	static const int B = 128;
	unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
	while (v >= B) {
		*(ptr++) = (v & (B-1)) | B;
		v >>= 7;
	}
	*(ptr++) = static_cast<unsigned char>(v);
	return reinterpret_cast<char*>(ptr);
}

void EncodeFixed32(char* dst, uint32_t value)
{
	memcpy(dst, &value, sizeof(value));
}

void EncodeFixed64(char* dst, uint64_t value)
{
	memcpy(dst, &value, sizeof(value));
}

const char* GetVarint32PtrFallback(const char* p, const char* limit, uint32_t* value)
{
	uint32_t result = 0;
	uint32_t byte = 0;
	for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7)
	{
		byte = *(reinterpret_cast<const unsigned char*>(p));
		p++;
		if (byte & 128)
		{
			result |= ((byte & 0x7F) << shift);
		}
		else
		{
			 result |= (byte << shift);
			 *value = result;
			 return reinterpret_cast<const char*>(p);
		}
	}

	return NULL;
}

bool GetVarint32(Slice* input, uint32_t* value) 
{
	const char* p = input->data();
	const char* limit = p + input->size();
	const char* q = GetVarint32Ptr(p, limit, value);
	if (q == NULL) {
		return false;
	} else {
		*input = Slice(q, limit - q);
		return true;
	}
}

extern bool GetVarint64(Slice* input, uint64_t* value);
extern bool GetLengthPrefixedSlice(Slice* input, Slice* result);

}