#ifndef _RETE_NETWORK_H_
#define _RETE_NETWORK_H_

#include <map>
#include <string>
#include <list>
#include <limits>
#include "Value.h"

#if _DEBUG
extern int nbFctCalls_SetValue;
extern int nbFctCalls_SetOutput;
#define INCR(x) ++nbFctCalls_##x
#else
#define INCR(x)
#endif

namespace Rete
{
class Gate;

class GateInput
{
private:
	Value _value;
	Gate* _gate;
public:
	GateInput() : _gate(0),_value(0) {}
	void init(Gate* g) {_gate=g;}
	template <typename T> inline void setValue(const T& v);
	template <typename T> inline void setValueNoCheck(const T& v);
	const Value& getValue() const { return _value; }
};

class MultipleGateInput : public std::list<GateInput*>
{
public:
	template<typename T> void setValue(T v) // int or const char*
	{
		std::list<GateInput*>::iterator i;
		for(i=begin();i!=end();++i)
			(*i)->setValue(v);
	}
};

typedef std::map<std::string,MultipleGateInput> InputMap;
extern Gate* prepare_query(std::string s,InputMap& aInput);


class Gate
{
private:
	Value _output;
	class GateInput* _nextGate;
protected:
	GateInput _leftGateInput;
	GateInput _rightGateInput;
	void propagateOutput() { if(_nextGate) _nextGate->setValueNoCheck(_output.getInt()); }
public:
#if _DEBUG
	std::string expr;
#endif
	Gate() : _output(0),_nextGate(0)
	{
		_leftGateInput.init(this);
		_rightGateInput.init(this);
	}
	GateInput& getLeftGateInput()  { return _leftGateInput; }
	GateInput& getRightGateInput() { return _rightGateInput; }

	const Value& getOutput() const { return _output; }
	template <typename T> void setOutput(const T& v)
	{
		INCR(SetOutput);
		if(_output == v)
			return;
		else
		{
			#if _DEBUG
			//std::cout << "Changing expr " << expr << " from " << _output << " to " << v << endl;
			#endif
			_output.set(v);
			propagateOutput();
		}
	}
	void pipeOutput(GateInput* gi)
	{
		_nextGate = gi;
		propagateOutput();
	}
	virtual void onChange() =0;
};

template <typename T> inline void GateInput::setValueNoCheck(const T& v)
{
	_value.set(v);
	_gate->onChange();
}

template <typename T> inline void GateInput::setValue(const T& v)
{
	INCR(SetValue);
	if(_value==v)
		return;
	else
		setValueNoCheck(v);
}

template <class BinaryFunc>
struct BinaryGate : public Gate
{
	BinaryGate() { onChange(); }
	void onChange()
	{
		setOutput(BinaryFunc::compute(_leftGateInput,_rightGateInput));
	}
};

template <class BinaryFunc>
struct IntBinaryGate : public Gate
{
	IntBinaryGate() { onChange(); }
	void onChange()
	{
		setOutput(BinaryFunc::compute(_leftGateInput.getValue().getInt(),_rightGateInput.getValue().getInt()));
	}
};


/////////////////////////////////////////////////////////////
// Now all the operators
/////////////////////////////////////////////////////////////
/*

struct Equal
{
	static int compute(int left,int right) { return left==right; }
};

struct NotEqual
{
	static int compute(int left,int right) { return left!=right; }
};

struct Or
{
	static int compute(int left,int right) { return left||right; }
};

struct And
{
	static int compute(int left,int right) { return left&&right; }
};

struct Smaller
{
	static int compute(int left,int right) { return left<right; }
};
struct SmallerOrEqual
{
	static int compute(int left,int right) { return left<=right; }
};

struct Greater
{
	static int compute(int left,int right) { return left>right; }
};
struct GreaterOrEqual
{
	static int compute(int left,int right) { return left>=right; }
};
struct Plus
{
	static int compute(int left,int right) { return left+right; }
};
struct Minus
{
	static int compute(int left,int right) { return left-right; }
};
struct Multiply
{
	static int compute(int left,int right) { return left*right; }
};
struct Divide
{
	static int compute(int left,int right) { return left/right; }
};
*/


struct Equal
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_equals(right.getValue()); }
};
struct Plus
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_plus(right.getValue()); }
};
struct Minus
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_minus(right.getValue()); }
};
struct Multiply
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_multiply(right.getValue()); }
};
struct Divide
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_divide(right.getValue()); }
};
struct NotEqual
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_not_equals(right.getValue()); }
};
struct Or
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_or(right.getValue()); }
};
struct And
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_and(right.getValue()); }
};
struct Smaller
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_smaller(right.getValue()); }
};
struct SmallerOrEqual
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_smallerOrEqual(right.getValue()); }
};
struct Greater
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_greater(right.getValue()); }
};
struct GreaterOrEqual
{
	static int compute(const GateInput& left, const GateInput& right) { return left.getValue().op_greaterOrEqual(right.getValue()); }
};


} // namespace Rete

#endif //_RETE_NETWORK_H_
