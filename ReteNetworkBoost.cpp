#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <map>
#include <functional>

#include "Rete2.h"
#include "ReteNetworkBoost.h"
#include "Rete2Operators.h"

// #define BOOST_SPIRIT_DEBUG       // define this for debug output#include <boost/spirit/core.hpp>
#include <boost/spirit/core.hpp>
#include <boost/assign/list_of.hpp>

///////////////////////////////////////////////////////////////////////////////
using namespace std;
using namespace boost::spirit;

////////////////////////////////////////////////////////////////////////////
//
//  Semantic actions
//
////////////////////////////////////////////////////////////////////////////

namespace Rete2
{
	void    do_NEGATE(char const*, char const*)    { throw string("negate not supported yet"); }
	void    do_IF(char const*, char const*)    { throw string("if then else not supported yet"); }
	////////////////////////////////////////////////////////////////////////////
	//
	//  Our WhereClause grammar
	//
	////////////////////////////////////////////////////////////////////////////
	struct WhereClause : public grammar<WhereClause>
	{
		ParsingContext& _ctx;
		WhereClause(ParsingContext& ctx) : _ctx(ctx) {}

		template <typename ScannerT>
		struct definition
		{
#define PUSH_OP(func) push_binary_op<func>(self._ctx)
			definition(WhereClause const& self)
			{
				identifier	= (lexeme_d[ ((ch_p('_') | alpha_p) >> *(ch_p('_') | alnum_p)) ]
							- str_p("if") // except keywords
							- str_p("then")
							- str_p("else"))[push_variable(self._ctx)];
				group       = ch_p('(') >> expression >> ')'; // back to the lowest priority
				literal		= lexeme_d[ ch_p('\'') >> (*~ch_p('\''))[push_value<string>(self._ctx)] >> '\'' ]
					| lexeme_d[ ch_p('"') >> (*~ch_p('"'))[push_value<string>(self._ctx)] >> '"' ];
				factor      = strict_real_p[push_value<double>(self._ctx)]
					| int_p[push_value<int>(self._ctx)]
					| literal
					| identifier
					| group
					| (ch_p('-') >> factor)[&do_NEGATE];
				term        = factor >> *(
					(ch_p('*') >> factor)[PUSH_OP(std::multiplies)]
					| (ch_p('/') >> factor)[PUSH_OP(std::divides)]);
				arithm		= term >> *(
					(ch_p('+') >> term)[PUSH_OP(std::plus)]
					| (ch_p('-') >> term)[PUSH_OP(std::minus)]);
				relational	= arithm >> *(
					(ch_p('<') >> arithm )[PUSH_OP(std::less)]
					| (str_p("<=") >> arithm)[PUSH_OP(std::less_equal)]
					| (ch_p('>') >> arithm)[PUSH_OP(std::greater)]
					| (str_p(">=") >> arithm)[PUSH_OP(std::greater_equal)]);
				equality	= relational >> *(
					(str_p("==") >> relational)[PUSH_OP(std::equal_to)]
					| (str_p("!=") >> relational)[PUSH_OP(std::not_equal_to)]);
				logical_and	= equality >> *(
					(str_p("&&") >> equality)[PUSH_OP(std::logical_and)]);
				logical_or	= logical_and >> *(
					(str_p("||") >> logical_and)[PUSH_OP(std::logical_or)]);
				logical_if = logical_or |
					(str_p("if") >> logical_or
						>> str_p("then") >> logical_or
						>> str_p("else") >> logical_or) [&do_IF];// todo

				expression = logical_if;
			}

			rule<ScannerT> expression, identifier, literal, group, factor, term, arithm,
				relational,	equality, logical_and, logical_or, logical_if;

			rule<ScannerT> const& start() const { return expression; }
		};
	};
} // namespace Rete2
////////////////////////////////////////////////////////////////////////////
//
//  Main program
//
////////////////////////////////////////////////////////////////////////////
using namespace Rete2;
int test_parse()
{
	cout << "/////////////////////////////////////////////////////////\n\n";
	cout << "\t\tExpression parser...\n\n";
	cout << "/////////////////////////////////////////////////////////\n\n";
	cout << "Type an expression...or [q or Q] to quit\n\n";


	string str;
	while (getline(cin, str))
	{
		cout << "-------------------------\n";
		Rete2::ParsingContext aCtx;
		Variable<int> a("a");
		Variable<string> s("s");
		aCtx.addVariable(a);
		aCtx.addVariable(s);
		WhereClause cl(aCtx);    //  Our parser
		//BOOST_SPIRIT_DEBUG_NODE(cl);

		if (str.empty() || str[0] == 'q' || str[0] == 'Q')
			break;
		cout << "Parsing gives:" << endl;
		parse_info<> info = parse(str.c_str(), cl, space_p);

		if (info.full)
		{
			Item& i = aCtx.top();
			FinalGate<int> theGate; // assume our result is int, could be bool or string
			a.set(2);
			s.set("toto");
			AbstractGate** g = boost::get<AbstractGate*>(&i.var);
			theGate.getInput().chain(*static_cast<Gate<int>*>(*g));
			cout << "Rule: " << str.c_str() <<endl
				<< "Result:" << theGate.getValue() << endl;
			cout << "-------------------------\n";
		}
		else
		{
			cout << "-------------------------\n";
			cout << "Parsing failed\n";
			cout << "stopped at: \": " << info.stop << "\"\n";
			cout << "-------------------------\n";
		}
	}

	cout << "Bye... :-) \n\n";
	return 0;
}

