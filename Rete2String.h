#ifndef _RETE2_STRING_H_
#define _RETE2_STRING_H_
#include <boost/call_traits.hpp>
#include <string>
#if _DEBUG
#include <iostream>
#endif

namespace Rete2 { class mystring; };
namespace boost {
	template <> struct call_traits<Rete2::mystring>
	{
		typedef Rete2::mystring value_type;
		typedef Rete2::mystring& reference;
		typedef const Rete2::mystring& const_reference;
		//typedef const Rete2::mystring& param_type;
		//typedef const char* const_reference;
		typedef const char* param_type;
	};
};

namespace Rete2
{
	// need a lot of optims !
	class mystring
	{
		char* _str;
		static char* dupStr(const char* s)
		{
			if(s==0) return 0;
			//#if _DEBUG
			//std::cout << "d";
			//#endif
			size_t len = strlen(s);
			void* p = malloc(len+1);
			memcpy(p,s,len+1);
			return static_cast<char*>(p);
		}
		// no memory management
		explicit mystring(void* p) : _str(static_cast<char*>(p)) {}
	public:
		mystring() : _str(0) {}
		explicit mystring(const char* s) : _str(dupStr(s)) {}
		mystring(const mystring& o) : _str(dupStr(o._str)) {}
		~mystring() { if(_str) free(_str); }

		// behave as a const char*
		operator const char*() { return _str; }
		inline bool operator==(const char* ostr) const
		{
			return _str && ostr ? 0==strcmp(_str,ostr) : (_str==0 && ostr==0);
		}
		mystring& operator=(const char* ostr)
		{
			if(ostr==0)
			{
				if(_str)
				{
					free(_str);
					_str=0;
				}
			}
			else
			{
				//#if _DEBUG
				//std::cout << "=";
				//#endif
				size_t len = strlen(ostr);
				_str = static_cast<char*>(realloc(_str,len+1));
				memcpy(_str,ostr,len+1);
			}
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& o, const mystring& s) { return o<<s._str; }

		mystring& operator=(const mystring& o)	{ return *this=o._str;	}
		inline char& operator[](int i) { return _str[i]; } // non checked
			// now all operators
		inline bool operator==(const mystring& o) const
		{
			return _str && o._str ? 0==strcmp(_str,o._str) : (_str==0 && o._str==0);
		}
		inline bool operator!=(const mystring& o) const
		{
			return !(*this == o);
		}
		inline bool operator<(const mystring& o) const
		{
			return _str && o._str ? 0<strcmp(_str,o._str) : _str<o._str;
		}
		inline bool operator<=(const mystring& o) const
		{
			return _str && o._str ? 0<=strcmp(_str,o._str) : _str<=o._str;
		}
		inline bool operator>(const mystring& o) const
		{
			return !(*this <= o);
		}
		inline bool operator>=(const mystring& o) const
		{
			return !(*this < o);
		}
		inline mystring operator+(const mystring& o) const
		{
			size_t len1 = _str==0 ? 0 : strlen(_str);
			size_t len2 = o._str==0 ? 0 : strlen(o._str);
			void* p = malloc(len1+len2+1);
			memcpy(p,_str,len1);
			memcpy(static_cast<char*>(p)+len1,o._str,len2+1);
			return mystring(p);
		}
		inline mystring operator-(const mystring& o) const
		{
			throw std::string("operator - not supported for std::strings");
		}
		inline mystring operator*(const mystring& o) const
		{
			throw std::string("operator * not supported for std::strings");
		}
		inline mystring operator/(const mystring& o) const
		{
			throw std::string("operator - not supported for std::strings");
		}
		inline bool operator||(const mystring& o) const
		{
			return _str || o._str;
		}
		inline bool operator&&(const mystring& o) const
		{
			return _str && o._str;
		}
	}; // class mystring
}; // namespace Rete2

#endif //_RETE2_STRING_H_
