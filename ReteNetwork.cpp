#include "ReteNetwork.h"
#include <exception>
#include <locale>
#include <iostream>

using namespace Rete;
using namespace std;

int nbFctCalls_SetValue;
int nbFctCalls_SetOutput;


ostream & operator<<(ostream& o, const Value& v)
{
	if(v.hasInt()) return o << v.getInt();
	else return o << v.getStr();
}

struct Operand
{
	enum Type
	{
		kConstantInt,
		kConstantStr,
		kVariable,
		kExpression
	};
	Operand(Type t,string s) : type(t), value(s) {}
	Type type;
	string value;
};

enum Operator
{
	kPlus,
	kMinus,
	kMultiply,
	kDivide,
	kEqual,
	kNotEqual,
	kOr,
	kAnd,
	kSmaller,
	kSmallerOrEqual,
	kGreater,
	kGreaterOrEqual
};

struct Expression
{
	Expression(Operand l, Operator o, Operand r) : left(l),op(o),right(r) {}
	Operand left;
	Operator op;
	Operand right;
};

map<string,Operator> initOperatorMap()
{
	map<string,Operator> m;
	m["=="] = kEqual;
	m["="] = kEqual;
	m["+"] = kPlus;
	m["-"] = kMinus;
	m["*"] = kMultiply;
	m["/"] = kDivide;
	m["!="] = kNotEqual;
	m["||"] = kOr;
	m["OR"] = kOr;
	m["or"] = kOr;
	m["&&"] = kAnd;
	m["AND"] = kAnd;
	m["and"] = kAnd;
	m["<"] = kSmaller;
	m["<="] = kSmallerOrEqual;
	m[">"] = kGreater;
	m[">="] = kGreaterOrEqual;
	return m;
}

static map<string,Operator> kOperatorMap = initOperatorMap();

Operator parseOperator(const string& s, size_t start, size_t& end)
{
	size_t cursor = start;
	while(cursor<s.length() && s[cursor]==' ') ++cursor;
	start = cursor;
	const char* operator_chars = "=+-*/><&|!";
	while(cursor<s.length() && strchr(operator_chars,s[cursor])!=NULL) ++cursor;
	string op = s.substr(start,cursor-start);
	end = cursor;
	map<string,Operator>::const_iterator i = kOperatorMap.find(op);
	if(i==kOperatorMap.end())
		throw string("Unknown operator: "+op);
	else
		return i->second;
}

Operand parseOperand(const string& s,size_t start,size_t& end)
{
	size_t cursor = start;
	while(cursor<s.length() && s[cursor]==' ') ++cursor;
	start = cursor;

	if(s[cursor] == '(')
	{
		int level = 1;
		++cursor;
		while(level != 0)
		{
			if(s[cursor]=='(') ++level;
			else if(s[cursor]==')') --level;
			++cursor;
		}
		end = cursor;
		return Operand(Operand::kExpression,s.substr(start+1,end-start-2));
	}
	else if(s[cursor] == '\'')
	{
		++cursor;
		while(cursor<s.length() && s[cursor] != '\'') ++cursor;
		end = cursor + 1;
		// todo?: output to a string
		return Operand(Operand::kConstantStr,s.substr(start+1,end-start-2));
	}
	else if(isdigit(s[cursor]))
	{
		++cursor;
		while(cursor<s.length() && isdigit(s[cursor])) ++cursor;
		end = cursor;
		return Operand(Operand::kConstantInt,s.substr(start,end-start));
	}
	else if(isalpha(s[cursor]))
	{
		++cursor;
		while(cursor<s.length() && isalnum(s[cursor])) ++cursor;
		end = cursor;
		return Operand(Operand::kVariable,s.substr(start,end-start));
	}
	else
	{
		throw string("Unknow token"+s.substr(start));
	}
}

Expression split(const string& s)
{
	size_t left=0,right=0;
	Operand l = parseOperand(s,left,right);
	left = right;
	Operator op = parseOperator(s,left,right);
	left = right;
	Operand r = parseOperand(s,left,right);
	// there should be only spaces now
	return Expression(l,op,r);
}

Gate* Rete::prepare_query(string s,InputMap& aInput)
{
	static int indent_level=0;
	++indent_level;
	string indent(indent_level,' ');
	Expression expr = split(s);

	Gate* root;
	switch(expr.op)
	{ // todo: delete all objects allocated by prepare
	case kPlus:		root = new BinaryGate<Plus>(); break;
	case kMinus:	root = new BinaryGate<Minus>(); break;
	case kMultiply:	root = new BinaryGate<Multiply>(); break;
	case kDivide:	root = new BinaryGate<Divide>(); break;
	case kEqual:	root = new BinaryGate<Equal>(); break;
	case kNotEqual:	root = new BinaryGate<NotEqual>(); break;
	case kOr:		root = new BinaryGate<Or>(); break;
	case kAnd:		root = new BinaryGate<And>(); break;
	case kSmaller:	root = new BinaryGate<Smaller>(); break;
	case kSmallerOrEqual:	root = new BinaryGate<SmallerOrEqual>(); break;
	case kGreater:	root = new BinaryGate<Greater>(); break;
	case kGreaterOrEqual:	root = new BinaryGate<GreaterOrEqual>(); break;
	default: throw string("operator not part of enum");
	}
#if _DEBUG
	root->expr=s;
#endif
	cout << indent << "->Op=" << expr.op << endl;
	cout << indent << "->Left=" << expr.left.value << endl;
	switch(expr.left.type)
	{
	case Operand::kConstantInt:
		root->getLeftGateInput().setValue(atoi(expr.left.value.c_str()));
		break;
	case Operand::kConstantStr:
		root->getLeftGateInput().setValue(expr.left.value.c_str());
		break;
	case Operand::kVariable:
		aInput[expr.left.value].push_back( & root->getLeftGateInput() );
		break;
	case Operand::kExpression:
		{
			Gate* e = prepare_query(expr.left.value,aInput);
			e->pipeOutput(&root->getLeftGateInput());
		}
		break;
	}

	cout << indent << "->Right=" << expr.right.value << endl;
	switch(expr.right.type)
	{
	case Operand::kConstantInt:
		root->getRightGateInput().setValue(atoi(expr.right.value.c_str()));
		break;
	case Operand::kConstantStr:
		root->getRightGateInput().setValue(expr.right.value.c_str());
		break;
	case Operand::kVariable:
		aInput[expr.right.value].push_back(&root->getRightGateInput());
		break;
	case Operand::kExpression:
		{
			Gate* e = prepare_query(expr.right.value,aInput);
			e->pipeOutput(&root->getRightGateInput());
		}
		break;
	}

	--indent_level;
	return root;
}
