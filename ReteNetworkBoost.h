#ifndef _RETE_NETWORK_BOOST_H_
#define _RETE_NETWORK_BOOST_H_

#include <boost/variant.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>
#include <Loki/Typelist.h>
#include <string>
#include <stack>
#include <map>
#include <ostream>

namespace Rete2
{
	// Variable name
	struct VariableName
	{
		const std::string name;
		VariableName(const std::string& n) : name(n) {}
		std::ostream& print(std::ostream& o) const {	return o << "var " << name; }
		friend std::ostream& operator<<(std::ostream& o,const VariableName& i) { return o << i.name; }
	};

	// all items MUST have value semantics, other copy ctor of Item does not work
	typedef boost::variant<int,double,bool,std::string,VariableName,AbstractGate*> MyVariant;
	typedef LOKI_TYPELIST_5(int,double,bool,std::string,Rete2::mystring) MyTypeList;

	// An item in the stack of the context
	struct Item
	{
		MyVariant var; 
		const std::type_info& type;
		const std::string dbg_info;
		//template <typename T> explicit Item(T i) : var(i),type(t) {}
		template <typename T> explicit Item(T i, const std::type_info& t, const std::string dbg)
			: var(i),type(t),dbg_info(dbg) {}
		friend std::ostream& operator<<(std::ostream& o,const Item& i)
		{
			return o << i.type.name() << " " << i.var << "; (" << i.dbg_info << ")";
		}
	};

	class ParsingContext
	{
		std::map<std::string,AbstractVariable*> _vars;
		std::stack<Item> _stack;
	public:
		Item& top() { return _stack.top(); }
		void pop() { _stack.pop(); }
		void push(const Item& i) { _stack.push(i); }

		void addVariable(AbstractVariable& v) { _vars[v.getName()] = &v; }
		AbstractVariable* getVariable(const std::string& name)
		{
			std::map<std::string,AbstractVariable*>::iterator i = _vars.find(name);
			return i==_vars.end() ? 0 : i->second;
		}
	};

	// Push a value 
	template <typename T> struct push_value
	{
		ParsingContext& _ctx;
		push_value(ParsingContext& ctx) : _ctx(ctx) {}

		void operator()(const T& d) const
		{
			_ctx.push(Item(d,typeid(T),"value"));
			cout << _ctx.top() << endl;
		}
		void operator()(char const* str, char const* end) const
		{
			_ctx.push(Item(T(str,end),typeid(T),"value"));
			cout << _ctx.top() << endl;
		}
	};

	// Push a variable
	// Push a value 
	struct push_variable
	{
		ParsingContext& _ctx;
		push_variable(ParsingContext& ctx) : _ctx(ctx) {}

		void operator()(char const* str, char const* end) const
		{
			std::string n(str,end);
			VariableName v(n);
			AbstractVariable* var = _ctx.getVariable(n);
			if(var==0) throw (std::string("Variable ")+n+" not found\n");
			Item i(v,var->getType(),"variable");
			_ctx.push(i);
			std::cout << _ctx.top() << std::endl;
		}
	};

	// Factory to build templated derived classes, to be used with type list is from Loki
	// this class will build instantiate TGate< TFunc<int> >
	// So for instance it will build BinaryGate< equal_to<int> >
	template<
		template <typename> class TFunc, // the function, e.g. equal_to
		template <typename,typename> class TGate = BinaryGate, // the gate
		class TBaseGate = AbstractGate>
	struct Factory
	{
		ParsingContext& _ctx;
		Item& _i0;
		Item& _i1;
		Factory(ParsingContext& ctx,Item& i0, Item& i1) : _ctx(ctx),_i0(i0), _i1(i1) {}
		template<class TTypeList> // the list of types to find
			void make(const type_info& in_type)
		{
			if(typeid(TTypeList::Head) == in_type)
			{
				// we have found the templte functor to instantiate
				typedef TTypeList::Head						input_type;
				typedef TFunc<TTypeList::Head>				functor_type;
				typedef TFunc<TTypeList::Head>::result_type result_type;
				typedef TGate<functor_type,result_type>		gate_type;
				gate_type* g = new gate_type();
				// todo: set the first and second gates (or chains) from the context
				// need to cast i0 to either input_type, Gate<input_type> or Variable<input_type>
				chain(g->getFirstInput(),_i0.var); // template types are deduced
				chain(g->getSecondInput(),_i1.var);// template types are deduced

				Item i(g,typeid(result_type),typeid(gate_type).name());
				_ctx.push(i);
			}
			else
				return make<TTypeList::Tail>(in_type); // call the next in the chain
		}
		// return 0 if type not found
		template<>
			void make<Loki::NullType>(const type_info& in_type)
		{ throw (string(in_type.name())+" type not found.");}

		// chain a gate input
		template<template<typename> class gate_input,class input_type>
			void chain(gate_input<input_type>& g, MyVariant& v)
		{
			if(AbstractGate** i = boost::get<AbstractGate*>(&v))
				g.chain(*static_cast<Gate<input_type>*>(*i));
			else if(VariableName* i = boost::get<VariableName>(&v))
				g.chain(*static_cast<Variable<input_type>*>( _ctx.getVariable(i->name)));
			else if(input_type* i = boost::get<input_type>(&v))
				g.set(*i);
			else
				throw string("Cannot chain\n");
		}
	};

	template < template<typename> class TFunc > // TFunc = multiplies, plus, etc.
	struct push_binary_op
	{
		ParsingContext& _ctx;
		push_binary_op(ParsingContext& ctx) :  _ctx(ctx) {}

		void operator()(char const* str, char const* end) const
		{
			// we get them in reversed order, so pop first the right operand
			Item i1 = _ctx.top();
			_ctx.pop();
			Item i0 = _ctx.top();
			_ctx.pop();

			// todo: need to compute the operator type, for the moment just assume
			// would need to rewrite all <functional> operators for more flexibility in functors
			// as <functional> does not support equal_to(int,char)
			// for the moment just assume they are equal
			assert(i0.type==i1.type);
			const type_info& t = i0.type;
			Factory<TFunc> factory(_ctx,i0,i1);
			factory.make<MyTypeList>(t);
			cout << _ctx.top() << endl;
			return;
		}
	};
}


#endif // _RETE_NETWORK_BOOST_H_