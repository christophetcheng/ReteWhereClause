#ifndef _RETE2_H_
#define _RETE2_H_
#include <ostream>
#include <algorithm>
#include <string>
#include <typeinfo>
#include <list>
#include <boost/call_traits.hpp>
#include <cassert>
#include "Rete2String.h"

// A little bit a templates
// to optimize, the only virtual function is onChange();
// every implementation drives the template types that will be used.
namespace Rete2
{
	// AbstractGate :  the root of all gates
	struct EventReceptor
	{
		virtual ~EventReceptor() {};
		virtual void onChange() = 0;
		EventReceptor* me() { return this; } // just to avoid warnings later on
	};
	typedef EventReceptor AbstractGate;

	// AbstractGateInput: the root of all gate inputs (set method is NOT virtual)
	class EventSender
	{
	protected:
		EventReceptor* const _receptor;
	public:
		EventSender(EventReceptor* p) : _receptor(p) {};
	};
	typedef EventSender AbstractGateInput;

	// Forward declarations
	template <typename T> class Gate;
	template <typename T> class Variable;
	// The gateinput, templated by the type of value
	template<typename T>
	class GateInput : public EventSender
	{
		T _value;
	public:
		typedef typename boost::call_traits<T>::param_type       param_type; // best way to pass a param of type T
		typedef typename boost::call_traits<T>::const_reference  const_reference;
		typedef GateInput<T> MyType;
		GateInput(EventReceptor* p): EventSender(p), _value() {}
		// if set to virtual, high performance degradation: drop from 34 M/s to 18 M/s
		void set(param_type t)
		{
			if(_value == t) return;
			else
			{
				#if _DEBUG
				std::cout << "Changing INPUT from "<<_value<<" to " << t << " for "<<this<<":" << typeid(MyType).name() << endl;
				#endif
				_value = t;
				_receptor->onChange();
			}
		}
		const_reference get() { return _value; }
		void chain(Gate<T>& g);
		void chain(Variable<T>& v);
	};

	// The root gate having no "next gate", so onChange does nothing
	template <typename Tinput>
	class FinalGate : public EventReceptor
	{
	public:
		typedef Tinput input_type;
		FinalGate() : _input(me()) {} // normal warning
		virtual ~FinalGate() {}

		virtual void onChange() {} // do nothing

		input_type getValue() { return _input.get(); }
		GateInput<input_type>& getInput() { return _input; }

		// singleton for temporary creation as gates need an output
		static FinalGate& defaultGate() { static FinalGate g;  return g; } ;
	private:
		GateInput<input_type> _input;
	};

	// The template base class for gates, templated by output
	// still pure virtual as onChange is not implemented
	// The output template param can be different from the result of the operation function
	template<typename Toutput>
	class Gate: public AbstractGate
	{
	public:
		typedef Toutput output_type;
		typedef GateInput<output_type> output_gate_type;

		Gate(output_gate_type& next = defaultGateOutput())	: _output(&next)	{}
		virtual ~Gate() {}
		virtual const type_info& getOutputType() { return typeid(output_type); }

		static output_gate_type& defaultGateOutput() { return FinalGate<output_type>::defaultGate().getInput(); }
	protected:
		friend class output_gate_type;
		output_gate_type* _output;
	};


	// UnaryGates, functions passed are coming from <functional>: logical_not and negate
	//template < template <typename> class Func, typename Tinput > // use UnaryGate<logical_not,bool>
	template <typename Func, typename Toutput = typename Func::result_type> // use UnaryGate<logical_not<bool> >
	class UnaryGate : public Func, public Gate<Toutput> // Will find functions in <functional>
	{
	public:
		typedef Gate<Toutput> Base;
		typedef typename Func::argument_type argument_type;

		UnaryGate(output_gate_type& next = defaultGateOutput())
			: Base(next), _input(me()) {}

		GateInput<argument_type>& getInput() { return _input; }

		virtual void onChange()
		{
			_output->set((*this)(_input.get()));
		}
	private:
		GateInput<argument_type> _input;
	};

	// BinaryGates, functions passed are coming from <functional>: multiplies, divides, logical_and, etc.
	template < typename Func, typename Toutput = typename Func::result_type >
	class BinaryGate : public Func, public Gate<Toutput>
	{
	public:
		typedef typename Func::first_argument_type	first_argument_type;
		typedef typename Func::second_argument_type second_argument_type;
		typedef Gate<Toutput> Base;
		typedef BinaryGate<Func,Toutput> MyType;

		BinaryGate(output_gate_type& next = defaultGateOutput())
			: Base(next), _first_input(me()), _second_input(me()) {}

		GateInput<first_argument_type>& getFirstInput() { return _first_input; }
		GateInput<second_argument_type>& getSecondInput() { return _second_input; }

		virtual void onChange()
		{
			#if _DEBUG
			std::cout << "Changing GATE from "<<_output->get()<< " to ("<<_first_input.get()<<","<<_second_input.get()<<")=" 
				<< (*this)(_first_input.get(),_second_input.get()) << " for "<<this<<":" << typeid(MyType).name() << endl; 
			#endif
			_output->set((*this)(_first_input.get(),_second_input.get()));
		}
	private:
		GateInput<first_argument_type> _first_input;
		GateInput<second_argument_type> _second_input;
	};

	// Ternary Gates (the only case known is if_then_else)
	template < typename Func, typename Toutput = typename Func::result_type >
	class TernaryGate : public Func, public Gate<Toutput>
	{
	public:
		typedef typename Func::first_argument_type	first_argument_type;
		typedef typename Func::second_argument_type second_argument_type;
		typedef typename Func::third_argument_type	third_argument_type;
		typedef Gate<Toutput> Base;

		TernaryGate(output_gate_type& next = defaultGateInput())
			: Base(next), _first_input(me()), _second_input(me()), _third_input(me()) {}

		virtual void onChange()
		{
			_output->set((*this)(_first_input.get(),_second_input.get(),_third_input.get()));
		}
	private:
		GateInput<first_argument_type> _first_input;
		GateInput<second_argument_type> _second_input;
		GateInput<third_argument_type> _third_input;
	};

	// Operator if_then_else
	template<typename Tinput> struct if_then_else {
		typedef bool	first_argument_type;
		typedef Tinput	second_argument_type;
		typedef Tinput	third_argument_type;
		typedef Tinput	result_type;
		typedef typename boost::call_traits<Tinput>::param_type arg_param_type;
		result_type operator()(bool iIf, arg_param_type iThen, arg_param_type iElse)
		{ return iIf ? iThen : iElse; }
	};

	//////////////////////////////////////////
	// Variable and TypedVariable
	//////////////////////////////////////////
	class AbstractVariable
	{
		const std::string _name;
		const std::type_info& _type;
	public:
		AbstractVariable(const std::string& n, const std::type_info& t) : _name(n),_type(t) {}
		const std::string& getName() { return _name; }
		const std::type_info& getType() { return _type; }
	};

	template <typename T>
	class Variable : public AbstractVariable
	{
	public:
		typedef typename boost::call_traits<T>::param_type param_type;
		Variable(const std::string& name) : AbstractVariable(name,typeid(T)) {}
		void set(param_type v)
		{
			GateInputList::iterator i	= _gateInputs.begin();
			GateInputList::iterator end = _gateInputs.end();
			for(;i!=end;++i)
				(*i)->set(v);
		}
	private:
		void push(GateInput<T>* p) { _gateInputs.push_front(p); }
		friend class GateInput<T>;
		typedef std::list<GateInput<T>*> GateInputList;
		GateInputList _gateInputs;
	};

	// Implement the chains of Gates
	template <typename T> void GateInput<T>::chain(Gate<T>& g)
	{
		g._output=this;
		g.onChange(); //virtual call to recompute the output value
	}
	template <typename T> void GateInput<T>::chain(Variable<T>& v)
	{
		v.push(this);
	}
};

#endif //_RETE2_H_
