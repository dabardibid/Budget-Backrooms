//! @file typedefs.h
//! @author Antoine Gonzalez - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_TYPEDEFS_H
#define _SUBSTANCE_AIR_FRAMEWORK_TYPEDEFS_H

#include <assert.h>
#include <cstddef>
#include <cinttypes>
#include <cstdio>

// Detect Modern C++ Features
#if (__cplusplus >= 201103L) || (_MSC_VER >= 1900)	//MSVC 2015
#	define AIR_CPP_MODERN_MEMORY
#	define AIR_CONSTEXPR constexpr
#else
#	define AIR_CONSTEXPR
#endif

#include <atomic>
#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#include <stdlib.h>

#ifdef __linux__
#	include <string.h>
#endif

#include "memory.h"

namespace SubstanceAir
{

//! @brief basic types used in the integration framework
typedef unsigned int UInt;
typedef unsigned long long UInt64;

//! @brief STL shared_ptr adjusted to use our allocators
template<typename T> using shared_ptr = std::shared_ptr<T>;

//! @brief STL make_shared, construct a shared_ptr using this helper. This is preferred to allocating 
//! by yourself because make_shared should make one allocation for both the object and the control block
template<typename T, class... Args> inline shared_ptr<T> make_shared(Args&& ... args)
{
	return std::allocate_shared<T>(aligned_allocator<T, AIR_DEFAULT_ALIGNMENT>(), std::forward<Args>(args)...);
}

//! @brief STL alias for unique_ptr
template<typename T> using unique_ptr = std::unique_ptr<T, deleter<T>>;

//! @brief create a unique ptr via template
template<typename T, typename... Args> unique_ptr<T> make_unique(Args&&... args)
{
	return unique_ptr<T>(AIR_NEW(T)(std::forward<Args>(args)...));
}

//! @brief safely cast a unique ptr between types. This will invalidate the caster unique_ptr
template<typename U, typename T> unique_ptr<U> static_unique_ptr_cast(unique_ptr<T>&& base)
{
	return unique_ptr<U>(static_cast<U*>(base.release()));
}

//! @brief make_shared and make_unique can only construct classes from public constructors, 
//! so you can use this type to convert a protected construct to public
//!	e.g. std::make_shared<make_constructible<SubstanceAir::InputImage>>(args)
template<typename T> struct make_constructible : public T
{
	template<typename... Args> make_constructible(Args... args)
		: T(std::forward<Args>(args)...)
	{
	}
};

//! @brief STL atomics mapped to our namespace
template<typename T>
using atomic = std::atomic<T>;
//! @brief STL atomic specialization mapped to our namespace
using atomic_bool = std::atomic_bool;
//! @brief STL atomic specialization mapped to our namespace
using atomic_uint = std::atomic_uint;

//! @brief STL weak_ptr mapped to our namespace
template<typename T>
using weak_ptr = std::weak_ptr<T>;

//! @brief STL containers mapped to substance air allocators
template<typename T, std::size_t ALIGNMENT> 
using aligned_vector = std::vector<T, aligned_allocator<T, ALIGNMENT>>;

template<typename T> 
using deque = std::deque<T, aligned_allocator<T, AIR_DEFAULT_ALIGNMENT>>;

template<typename K, typename V> 
using map = std::map<K, V, std::less<K>, aligned_allocator<std::pair<const K, V>, AIR_DEFAULT_ALIGNMENT>>;

template<typename K>
using set = std::set<K, std::less<K>, aligned_allocator<K, AIR_DEFAULT_ALIGNMENT>>;

template<typename... Types>
using tuple = std::tuple<Types...>;

template<typename T> 
using vector = aligned_vector<T, AIR_DEFAULT_ALIGNMENT>;

// Note:: Windows has really odd behavior with using char16_t, and wchar_t is
// not always going to be 16-bits, so it must be aliased properly to work with
// the UTF-16 string pointers returned by the linker.
#ifdef _MSC_VER
using Utf16Char = wchar_t;
#else
using Utf16Char = char16_t;
#endif

//! @brief Type for a UCS-4 string
using ucs4string = vector<unsigned int>;

using u16string = std::basic_string<Utf16Char, std::char_traits<Utf16Char>, aligned_allocator<Utf16Char, AIR_DEFAULT_ALIGNMENT>>;

using string = std::basic_string<char, std::char_traits<char>, aligned_allocator<char, AIR_DEFAULT_ALIGNMENT>>;
using stringstream = std::basic_stringstream<char, std::char_traits<char>, aligned_allocator<char, AIR_DEFAULT_ALIGNMENT>>;

static inline string to_string(std::string& str)
{
	return string(str.c_str());
}

static inline string to_string(const std::string& str)
{
	return string(str.c_str());
}

static inline string to_string(const char* str)
{
	return string(str);
}

static inline std::string to_std_string(string& str)
{
	return std::string(str.c_str());
}

static inline std::string to_std_string(const string& str)
{
	return std::string(str.c_str());
}

//! @brief std::index_sequence implementation (C++14)

template<class T, T... Nums> struct integer_sequence {};
template<class S> struct next_integer_sequence;

template<class T, T... Nums> struct next_integer_sequence<integer_sequence<T, Nums...>>
{
	using type = integer_sequence<T, Nums..., sizeof...(Nums)>;
};

template<class T, T I, T N> struct _generate_integer_sequence
{
	using type = typename next_integer_sequence<
		typename _generate_integer_sequence<T, I + 1, N>::type
	>::type;
};

// finalized specialization
template<class T, T N> struct _generate_integer_sequence<T, N, N>
{
	using type = integer_sequence<T>;
};

template<class T, T N>
using make_integer_sequence = typename _generate_integer_sequence<T, 0, N>::type;

template<std::size_t... Nums> using index_sequence = integer_sequence<std::size_t, Nums...>;
template<std::size_t N> using make_index_sequence = make_integer_sequence<std::size_t, N>;

//! @brief std::apply implementation (C++17) (Return values not handled yet!)
template<class F, class Tuple, size_t ...S>
int applyExpanded(F&& f, Tuple&& tuple, index_sequence<S...>)
{
	f(std::get<S>(std::forward<Tuple>(tuple))...);
	return 0;
}

template <class F, class Tuple>
AIR_CONSTEXPR int apply(F&& f, Tuple&& t)
{
	return applyExpanded(
		std::forward<F>(f),
		std::forward<Tuple>(t),
		make_index_sequence<
			std::tuple_size<typename std::remove_reference<Tuple>::type>::value
		>());
}

}  // namespace SubstanceAir

#include "vector.h"

namespace SubstanceAir
{
//! @brief containers used in the integration framework
typedef vector<unsigned char> BinaryData;
typedef vector<UInt> Uids;

// Vectors
typedef Math::Vector2<float> Vec2Float;
typedef Math::Vector3<float> Vec3Float;
typedef Math::Vector4<float> Vec4Float;
typedef Math::Vector2<int> Vec2Int;
typedef Math::Vector3<int> Vec3Int;
typedef Math::Vector4<int> Vec4Int;
}

#endif //_SUBSTANCE_AIR_FRAMEWORK_TYPEDEFS_H
