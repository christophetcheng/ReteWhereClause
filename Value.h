#ifndef _RETE_VALUE_H_
#define _RETE_VALUE_H_

#include <iostream>

namespace Rete
{
class Value
{
private:
	friend class GateInput;
	// we should never have _intValue==0 && _strValue==0
	mutable int _intValue;
	mutable char* _strValue;
	static char* dupStr(const char* s)
	{
		#if _DEBUG
		std::cout << "d" << std::endl;
		#endif
		size_t len = strlen(s);
		void* p = malloc(len+1);
		memcpy(p,s,len+1);
		return (char*)p;
	}
public:
	Value() : _intValue(0),_strValue(0) {} // initialize to int 0
	Value(int i) : _intValue(i),_strValue(0) {}
	Value(const char* s) : _intValue(std::numeric_limits<int>::max()), _strValue(dupStr(s)) {}
	bool hasInt() const { return _intValue!=std::numeric_limits<int>::max(); }
	bool hasStr() const { return _strValue!=0; }
	int getInt() const { return _intValue; }
	const char* getStr() const { return _strValue; }
	// don't like too much operator overloading, but useful for templates in GateInput<Value> where Value=int
	bool operator==(int i) const { return i==_intValue; }
	bool operator==(const char* s) const { return _strValue!=0 && (strcmp(_strValue,s)==0); }
	bool operator==(const Value& other) const { return (*this == other._intValue) || (*this == other._strValue);}

	/*
	void set(const Value& v)
	{
		_intValue=v._intValue;
		_strValue=v._strValue;
	}
	*/
	void set(int i)
	{
		_intValue=i;
		_strValue=0;
	}
	void set(const char* s)
	{
		_intValue=std::numeric_limits<int>::max();
		_strValue=dupStr(s);
	}

	inline int op_equals(const Value& other) const
	{
		return ( *this == other );
	}
	inline int op_not_equals(const Value& other) const
	{
		return ( !(*this == other) );
	}
	inline int op_smaller(const Value& other) const
	{
		return ( hasInt() ? (_intValue < other._intValue) : (strcmp(_strValue,other._strValue)<0) );
	}
	inline int op_smallerOrEqual(const Value& other) const
	{
		return ( hasInt() ? (_intValue <= other._intValue) : (strcmp(_strValue,other._strValue)<=0) );
	}
	inline int op_greater(const Value& other) const
	{
		return ( hasInt() ? (_intValue > other._intValue) : (strcmp(_strValue,other._strValue)>0) );
	}
	inline int op_greaterOrEqual(const Value& other) const
	{
		return ( hasInt() ? (_intValue >= other._intValue) : (strcmp(_strValue,other._strValue)>=0) );
	}
	inline int op_plus(const Value& other) const
	{
		return ((hasInt() ? _intValue : 0) + (other.hasInt()?other._intValue:0));
	}
	inline int op_minus(const Value& other) const
	{
		return ((hasInt() ? _intValue : 0) - (other.hasInt()?other._intValue:0));
	}
	inline int op_multiply(const Value& other) const
	{
		return ((hasInt() ? _intValue : 0) * (other.hasInt()?other._intValue:0));
	}
	inline int op_divide(const Value& other) const
	{
		return ( (hasInt()&&other.hasInt()&&other.getInt()!=0)? _intValue/other._intValue : 0 );
	}
	inline int op_or(const Value& other) const
	{
		return ((hasInt() ? _intValue : 0) || (other.hasInt()?other._intValue:0));
	}
	inline int op_and(const Value& other) const
	{
		return ((hasInt() ? _intValue : 0) && (other.hasInt()?other._intValue:0));
	}
};
};
// implementation in ReteNetwork.cpp
std::ostream & operator<<(std::ostream& o, const Rete::Value& v);
#endif // _RETE_VALUE_H_