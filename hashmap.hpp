#pragma once

#include "string.hpp"
#include <string>
#include "xtypes.hpp"

#ifdef _MSC_VER
	#include <unordered_map>
	#include <unordered_set>
	#include <functional>
	#include<limits>
#elif (__GNUC__ <= 2)
	#include <hash_map>
  #include <stl_hash_fun.h>
	#define __gnu_cxx std
#else
/*
	#include <ext/hash_map>
	#include <ext/hash_fun.h>
	#include <ext/hash_set>
*/
	#include <tr1/unordered_map>
	#include <tr1/unordered_set>
	#include <tr1/functional>
	#include <limits>
#endif

/*using __gnu_cxx::hash;
using __gnu_cxx::hashtable;
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_multimap;
using __gnu_cxx::hash_set;*/

using std::tr1::unordered_set;
using std::tr1::unordered_map;
using std::tr1::hash;
using std::string;
using namespace lb;

namespace std
{
	namespace tr1
	{
#if (__GNUC__ <= 4 && __GNUC_MINOR__ < 4 && !BSD)
		template<> struct hash<unsigned long long> {
			size_t operator()(unsigned long long val) const 
			{ 
				const int size_t_bits = std::numeric_limits<std::size_t>::digits;
				// ceiling(std::numeric_limits<T>::digits / size_t_bits) - 1
				const int length = (std::numeric_limits<unsigned long long>::digits - 1)
						/ size_t_bits;

				std::size_t seed = 0;

				// Hopefully, this loop can be unrolled.
				for(unsigned int i = length * size_t_bits; i > 0; i -= size_t_bits)
				{
						seed ^= (std::size_t) (val >> i) + (seed<<6) + (seed>>2);
				}
				seed ^= (std::size_t) val + (seed<<6) + (seed>>2);

				return seed;
			}
		};
		template<> struct hash<const unsigned long long> {
			size_t operator()(const unsigned long long val) const 
			{ 
				const int size_t_bits = std::numeric_limits<std::size_t>::digits;
				// ceiling(std::numeric_limits<T>::digits / size_t_bits) - 1
				const int length = (std::numeric_limits<const unsigned long long>::digits - 1)
						/ size_t_bits;

				std::size_t seed = 0;

				// Hopefully, this loop can be unrolled.
				for(unsigned int i = length * size_t_bits; i > 0; i -= size_t_bits)
				{
						seed ^= (std::size_t) (val >> i) + (seed<<6) + (seed>>2);
				}
				seed ^= (std::size_t) val + (seed<<6) + (seed>>2);

				return seed;
			};
		};
#endif
	}
}
/*
namespace std
{
namespace tr1
{

namespace __gnu_cxx
{
	template<> struct hash<string>
	{
		size_t  operator()(const string& __s) const
		{	
			return __stl_hash_string(__s.c_str());
									        
		}
	};

	struct eqstr
	{
		bool operator()(const String &s1, const String &s2) const
		{
			return s1.Cmp(s2);
		}
	};

	template<> struct hash<lb::String&>
	{
		size_t operator()(const lb::String &str) const 
		{ 
			if (str) 
				return __stl_hash_string(str.c_str()); 
						else return 0; 
		}
	};
	
	template<> struct hash<long long> {
		size_t operator()(long long __x) const { return __x; }
	};
	template<> struct hash<const long long> {
		size_t operator()(const long long __x) const { return __x; }
	};

 	template<> struct hash<unsigned long long> {
		size_t operator()(unsigned long long __x) const { return static_cast<size_t>(__x); }
	};
	template<> struct hash<const unsigned long long> {
		size_t operator()(const unsigned long long __x) const { return static_cast<size_t>(__x); }
	};
};*/

