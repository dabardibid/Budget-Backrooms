//! @file memory.h
//! @author Josh Coyne - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
#ifndef _SUBSTANCE_AIR_FRAMEWORK_MEMORY_H
#define _SUBSTANCE_AIR_FRAMEWORK_MEMORY_H

#if !defined(AIR_DEFAULT_ALIGNMENT)
#define AIR_DEFAULT_ALIGNMENT 16
#endif

#include <assert.h>
#include <limits>
#include <new>

namespace SubstanceAir
{

void* alignedMalloc(size_t bytesCount, size_t alignment);
void alignedFree(void* bufferPtr);

//! @brief substance air C++ allocator
template<class T, std::size_t ALIGNMENT> struct aligned_allocator
{
	//! @brief typedefs required for custom allocator use with the STL
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template<class U> struct rebind
	{
		typedef aligned_allocator<U, ALIGNMENT> other;
	};

	//! @brief these constructors do nothing because the allocator is stateless
	aligned_allocator() {}
	aligned_allocator(const aligned_allocator<T, ALIGNMENT>&) {}
	template<class U> aligned_allocator(const aligned_allocator<U, ALIGNMENT>&) {}

	//! @brief allocates memory using our internal memory allocation
	inline pointer allocate(std::size_t n) const
	{
		return reinterpret_cast<pointer>(alignedMalloc(n * sizeof(T), ALIGNMENT));
	}

	//! @brief deallocate a pointer previously allocated by this allocator
	inline void deallocate(pointer p, std::size_t) const
	{
		alignedFree(p);
	}

	//! @brief construct object using placement new (Deprecated in C++17)
	template<class U, class... Args> inline void construct(U* p, Args&& ... args)
	{
		::new ((void*)p) U(std::forward<Args>(args)...);
	}

	//! @brief call destructor on pointer (Deprecated in C++17)
	template<class U> inline void destroy(U* p) const
	{
		// NOTE: This reference is required because MSVC reports `unreferenced formal variable`
		// if all you invoke is a destructor. I am not checking for nullptr here because the
		// standard guides I have read indicate you do not check for null here.
#ifdef _MSC_VER
		(p); // NOLINT
#endif

		p->~U();
	}

	//! @brief returns the maximum allocation we support (Deprecated in C++17)
	inline size_type max_size() const
	{
		return std::numeric_limits<size_type>::max() / sizeof(value_type);
	}
};

//! @brief compare two allocators (equality)
template<class T, std::size_t TA, class U, std::size_t UA> bool operator==(const aligned_allocator<T, TA>&, const aligned_allocator<U, UA>&)
{
	return (std::is_same<T, U>::value && TA == UA);
}

//! @brief compare two allocators (inequality)
template<class T, std::size_t TA, class U, std::size_t UA> bool operator!=(const aligned_allocator<T, TA>&, const aligned_allocator<U, UA>&)
{
	return !(std::is_same<T, U>::value && TA == UA);
}

//! @brief substance air new/delete macros
template<typename T> void deleteObject(T* ptr)
{
	ptr->~T();
	alignedFree(ptr);
}

template<typename T> T* newObjectArray(size_t count)
{
	size_t* ptr = (size_t*)SubstanceAir::alignedMalloc(sizeof(size_t) + (sizeof(T) * count), AIR_DEFAULT_ALIGNMENT);
	*ptr = count;
	T* objArray = reinterpret_cast<T*>(ptr + 1);
	for (size_t i = 0; i < count; i++)
		new (objArray + i) T;
	return objArray;
}

template<typename T> void deleteObjectArray(T* ptr)
{
	size_t* countPtr = reinterpret_cast<size_t*>(ptr) - 1;
	size_t count = *(countPtr);
	for (size_t i = 0; i < count; i++)
		(ptr + i)->~T();
	alignedFree(countPtr);
}

//! @brief deleter
template<typename T> struct deleter
{
	void operator()(T* ptr)
	{
		deleteObject(ptr);
	}
};

}

//used to ensure only our macros use our global new/delete operators
struct SubstanceAirMallocDummy
{
	int unused;
};

inline void* operator new (size_t size, SubstanceAirMallocDummy*, const std::nothrow_t&)
{
	return SubstanceAir::alignedMalloc(size, AIR_DEFAULT_ALIGNMENT);
}

//compiler complains if we don't have this, but do NOT use it!
inline void operator delete (void*, SubstanceAirMallocDummy*, const std::nothrow_t&)
{
	assert(false);
}

#define AIR_NEW(type) new ((SubstanceAirMallocDummy*)0, std::nothrow) type
#define AIR_DELETE(ptr) deleteObject(ptr)

#define AIR_NEW_ARRAY(type,count) newObjectArray<type>(count)
#define AIR_DELETE_ARRAY(ptr) deleteObjectArray(ptr)

//! @brief if AIR_OVERRIDE_GLOBAL_NEW is set
//!	we override the global new operators.
//! Be careful enabling this as it will override
//! operator new in any linked static libraries you compile against!
#if defined(AIR_OVERRIDE_GLOBAL_NEW)
void* operator	new (size_t);
void* operator	new (size_t, std::nothrow_t&);
void* operator	new [](size_t);
void* operator	new [](size_t, std::nothrow_t&);
void operator	delete (void*);
void operator	delete (void*, std::nothrow_t&);
void operator	delete [](void*);
void operator	delete [](void*, std::nothrow_t&);
#endif

#endif //_SUBSTANCE_AIR_FRAMEWORK_MEMORY_H
