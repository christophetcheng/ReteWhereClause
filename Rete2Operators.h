#ifndef _RETE2_OPERATORS_H_
#define _RETE2_OPERATORS_H_

#include <string>

namespace std {

#define UNKNOWN_OPERATOR(arg,op,out) \
	template<>	struct op<arg> : public binary_function<arg, arg, out> {\
	out operator()(arg,arg){ throw string("Operator " #op " no suported for type " #arg); } \
	};

	UNKNOWN_OPERATOR(string,multiplies,string);
	UNKNOWN_OPERATOR(string,divides,string);
	UNKNOWN_OPERATOR(string,minus,string);
	UNKNOWN_OPERATOR(string,logical_and,bool);
	UNKNOWN_OPERATOR(string,logical_or,bool);
	UNKNOWN_OPERATOR(bool,multiplies,bool);
	UNKNOWN_OPERATOR(bool,divides,bool);
	UNKNOWN_OPERATOR(bool,minus,bool);
	UNKNOWN_OPERATOR(bool,plus,bool);

#define REDEFINE_DIVIDES(arg) \
	template<>	struct divides<arg> : public binary_function<arg, arg, arg> { \
		arg operator()(arg i,arg j){ return j==0 ? std::numeric_limits<arg>::quiet_NaN() : i/j; } \
	};

	REDEFINE_DIVIDES(int);
	REDEFINE_DIVIDES(double);

#undef UNKNOWN_OPERATOR
};

namespace Rete2 { namespace Operators {



}; }; //namespace Rete2 { namespace Operators {

#endif //_RETE2_OPERATORS_H_
