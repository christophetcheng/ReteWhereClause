#include "ReteNetwork.h"
#include <iostream>
#include <time.h>
#include <iomanip>

using namespace Rete;
using namespace std;
extern int test_parse();
extern int test_rete2();

static int test_rete1();

int main(int argc, char** argv)
{
	try {
	//string rule = "((a == b) || (c < 10)) && ((a > c) || (a < 0))";

	string str;
    #if _DEBUG
	cout << "Enter parse, rete1 or rete2: ";
	getline(cin, str);
	#else
	str = argc>1 ? argv[1] : "rete1";
	#endif

	clock_t start = clock();

	int nbQuery;

	if(str=="parse")
		nbQuery = test_parse();
	else if(str=="rete2")
		nbQuery = test_rete2();
	else
		nbQuery = test_rete1();

	clock_t finish = clock();

	double clocks = finish - start;
	double duration = clocks/CLOCKS_PER_SEC;
	double million_per_sec = (double)nbQuery/1000000 / duration ;

	cout << setprecision(4) << million_per_sec
		<< " million queries per second."<< endl;
	cout << clocks/nbQuery*1000000
		<< " clocks per query."<< endl;
	} catch(const string& s) {
		cout << "Exception caught: " << s << endl;
	}
	return 0;
};

static int test_rete1()
{
	string rule =
		"( ((i==((3*2)/3)) && ((2*h)==(5-1)))"
			"&& ((i==h) && (dd==13)) )"
		"&&"
		"(((i==2)||(dd<=ii)) && ((dd>=ii) && ((2*i)==4)))";

	InputMap aGateInputs;
	Gate* aGate;
	try {
		clock_t start, finish;
		start = clock();
		aGate = Rete::prepare_query(rule,aGateInputs);
		finish = clock();
		double duration = (double)(finish-start)/CLOCKS_PER_SEC;
		cout << "Prepare duration is " << duration << endl;
	}
	catch (string& e) {
		cout << "Cannot parse :" << e << endl;
		exit(-1);
	}

	MultipleGateInput& varI = aGateInputs["i"];
	MultipleGateInput& varH = aGateInputs["h"];
	MultipleGateInput& varDD = aGateInputs["dd"];
	MultipleGateInput& varII = aGateInputs["ii"];

    #if _DEBUG
	const int N = 1;
	#else
	const int N = 20; // number of million queries to run
	#endif
	const int C1=10;
	const int C2=10;
	const int C3=10;
	const int C4=10;
	const int nbQuery = N * 1000000 / (C1*C2*C3*C4);
	cout << "Will Now run " << nbQuery << " loops of " << (C1*C2*C3*C4) << " queries." << endl;
	bool error_found = false;
	for(int iter=0;iter<nbQuery;++iter)
	{
		//cout << "Iteration #" << iter << endl;
		for(int i  = -5 ; i <(-5+C1); ++i  )
		{
		for(int h  = -5 ; h <(-5+C2); ++h  )
		{
		for(int dd = 5 ;  dd<( 5+C3); ++dd )
		{
		for(int ii = 5 ;  ii<( 5+C4); ++ii )
		{
			#if _DEBUG
			//cout << "Setting i=" << i << endl;
			#endif
			varI.setValue(i);
			#if _DEBUG
			//cout << "Setting h=" << h << endl;
			#endif
			varH.setValue(h);
			#if _DEBUG
			//cout << "Setting dd=" << dd << endl;
			#endif
			varDD.setValue(dd);
			#if _DEBUG
			//cout << "Setting ii=" << ii << endl;
			#endif
			varII.setValue(ii);
			
			//cout << "i="<<i<<",h="<<h<<",dd="<<dd<<",ii="<<ii<< endl;
			int correct_output = ( (( ( i == ((3*2)/3)) && ((2*h) == (5-1)) ) && ((i==h) && (dd==13)))
				&& (((i==2)||(dd<=ii)) && ((dd>=ii) && ((2*i)==4))) );
			if(aGate->getOutput().getInt() != correct_output)
			{
				error_found = true;
				#if _DEBUG
				cout << "Error --> i="<<i<<",h="<<h<<",dd="<<dd<<",ii="<<ii<< endl;
				cout << "aGate->getOutput()="<< aGate->getOutput().getInt() << endl;
				cout << "Correct Output=" << correct_output << endl;
				#endif
			}
		}
		}
		}
		}
	}
	cout << "Rule: " << rule << endl;
	#ifdef _DEBUG
	cout << "SetOuput called " << nbFctCalls_SetOutput << " times." << endl;
	cout << "SetValue called " << nbFctCalls_SetValue << " times." << endl;
	#endif
	cout << (error_found ? "An incorrect output has been found." : "No incorrect output found.") << endl;
	return N * 1000000;
}
