#ifndef PORT_ATOMIC_POINTER_H_
#define PORT_ATOMIC_POINTER_H_

#include <stdint.h>

#ifdef _MSC_VER
#include <windows.h>
#endif

#if defined(_M_X64) || defined(__x86_64__)
#define ARCH_CPU_X86_FAMILY 1
#elif defined(_M_IX86) || defined(__i386__) || defined(__i386)
#define ARCH_CPU_X86_FAMILY 1
#endif

namespace leveldb{
namespace port{

#if defined(_MSC_VER) && defined(ARCH_CPU_X86_FAMILY)
#define LEVELDB_HAVE_MEMORY_BARRIER
#elif defined(ARCH_CPU_X86_FAMILY) && defined (__GNUC__)
inline void MemoryBarrier() {
	__asm__ __volatile__("" : : : "memory");
}
#define LEVELDB_HAVE_MEMORY_BARRIER
#endif

#if defined(LEVELDB_HAVE_MEMORY_BARRIER)
class AtomicPointer
{
private:
	void* rep_;
public:
	AtomicPointer() : rep_(NULL) {}

	explicit AtomicPointer(void* p) : rep_(p) {}

	void* NoBarrierLoad() const { return rep_;}

	void NoBarrierStore(void* v) { rep_ = v;}

	void* AcquireLoad() const 
	{
		void* result = rep_;
		MemoryBarrier();
		return result;
	}
};

#else
#error Please implement AtomicPointer for this platform
#endif

#undef LEVELDB_HAVE_MEMORY_BARRIER
#undef ARCH_CPU_X86_FAMILY

}

}

#endif