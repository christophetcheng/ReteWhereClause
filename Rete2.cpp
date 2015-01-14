#include "Rete2.h"
#include "Rete2String.h"
#include <functional>
#include <iostream>
#include <time.h>
#include <string>

using namespace Rete2;
using namespace std;

// string   => 14.53 million queries per second.
// mystring => 19.4 million queries per second.

int test_rete2()
{
	// mystring=="c"

	// mystring=="c"

	// int

    #if _DEBUG
	const int nbQuery = (int)(0.1 * 1000000);
	#else
	const int nbQuery = (int)(20 * 1000000);
	#endif

	clock_t start,finish;
	double clocks;

	const int kCpu = 1600000000; // 1.6 Ghz CPU

// Results for 1.6 Ghz cpu are: 
//36.84 clocks per query for Rete2::string MATCHING. // not really better than std::string
//215.92 clocks per query for Rete2::string NON MATCHING.
//55.28 clocks per query for std::string MATCHING.
//268.44 clocks per query for std::string NON MATCHING.
//5.6 clocks per query for int MATCHING.
//15.64 clocks per query for int NON MATCHING.

	////////////////////////////////////
	// compute clocks for each item
	{
		Variable<mystring> aMyStr("mystring");
		BinaryGate<equal_to<mystring> > aEqMyStr;
		aEqMyStr.getFirstInput().set("a");
		aEqMyStr.getSecondInput().chain(aMyStr);

		start = clock();
		for(int iter=0;iter<nbQuery;++iter)
			aMyStr.set("z"); 
		finish = clock();
		clocks = finish - start;
		cout << clocks/CLOCKS_PER_SEC/nbQuery*kCpu << " clocks per query for Rete2::string MATCHING."<< endl;

		start = clock();
		char buf[2] = "z";
		for(int iter=0;iter<nbQuery;++iter)
		{
			buf[0]='z'-(iter&1);
			aMyStr.set(buf); 
		}
		finish = clock();
		clocks = finish - start;
		cout << clocks/CLOCKS_PER_SEC/nbQuery*kCpu << " clocks per query for Rete2::string NON MATCHING."<< endl;
	}
	{
		Variable<string> aStr("string");
		BinaryGate<equal_to<string> > aEqStr;
		aEqStr.getFirstInput().set("a");
		aEqStr.getSecondInput().chain(aStr);
		string s("z");

		start = clock();
		for(int iter=0;iter<nbQuery;++iter)
			aStr.set(s);
		finish = clock();
		clocks = finish - start;
		cout << clocks/CLOCKS_PER_SEC/nbQuery*kCpu << " clocks per query for std::string MATCHING."<< endl;

		start = clock();
		for(int iter=0;iter<nbQuery;++iter)
		{
			s[0]='z'-(iter&1);
			aStr.set(s);
		}
		finish = clock();
		clocks = finish - start;
		cout << clocks/CLOCKS_PER_SEC/nbQuery*kCpu << " clocks per query for std::string NON MATCHING."<< endl;
	}
	{
		Variable<int> aInt("int");
		BinaryGate<equal_to<int> > aEqInt;
		aEqInt.getFirstInput().chain(aInt);
		aEqInt.getSecondInput().set(-1);

		start = clock();
		for(int iter=0;iter<nbQuery;++iter)
			aInt.set(10); 
		finish = clock();
		clocks = finish - start;
		cout << clocks/CLOCKS_PER_SEC/nbQuery*kCpu << " clocks per query for int MATCHING."<< endl;

		start = clock();
		for(int iter=0;iter<nbQuery;++iter)
			aInt.set(iter); 
		finish = clock();
		clocks = finish - start;
		cout << clocks/CLOCKS_PER_SEC/nbQuery*kCpu << " clocks per query for int NON MATCHING."<< endl;
	}

	cout << endl;
	return nbQuery*6;

	//Variable<int> aIntI("i");
	//Variable<int> aIntJ("j");

	//// i==2
	//BinaryGate<equal_to<int> > aEqIntI;
	//aEqIntI.getFirstInput().chain(aIntI);
	//aEqIntI.getSecondInput().set(2);

	//// j==3
	//BinaryGate<equal_to<int> > aEqIntJ;
	//aEqIntJ.getFirstInput().set(3);
	//aEqIntJ.getSecondInput().chain(aIntJ);

	//const int C1=10;
	//const int C2=10;
	//const int nbLoops = (int)(N * 1000000 / (C1*C2));
	//cout << "Will Now run " << nbLoops << " loops of " << (C1*C2) << " queries." << endl;
	//static const bool test_str = true;
	//typedef string mystr;
	//mystr buf("a");
	//bool error_found = false;
	// i==2 || s=="c"
	//FinalGate<bool> theFinalGate;
	//BinaryGate<logical_or<bool> > aOr(theFinalGate.getInput());
	//aOr.getFirstInput().chain(aEqIntI);
	//if(test_str)
	//	aOr.getSecondInput().chain(aEqStr);
	//else
	//	aOr.getSecondInput().chain(aEqIntJ);
	//printf("(%d == %d) || (%d == %d) => %d\n",
	//	aEqIntI.getFirstInput().get(),aEqIntI.getSecondInput().get(),
	//	aEqIntJ.getFirstInput().get(),aEqIntJ.getSecondInput().get(),
	//	theFinalGate.getValue());
	//for(int iter=0;iter<nbLoops;++iter)
	//{
	//	//if(iter%100000==0) cout << "Iter #" << iter << " out of " << nbLoops << endl;
	//	for(int j=-5;j<-5+C1;++j)
	//	{
	//		for(int i=-5 ; i< -5+C2 ; ++i)
	//		{
	//			#if _DEBUG
	//			cout << endl;
	//			#endif
	//			if(test_str)
	//			{
	//				buf[0]='f'+j;
	//				aStr.set(buf); // => 36 clocks
	//			}
	//			else
	//				aIntJ.set(j);
	//			
	//			aIntI.set(i); // => 15 clocks

	//			//bool res = test_str ? (i==2 || buf=="c") : (i==2 || j==3);
	//			//if(theFinalGate.getInput().get() != res)
	//			//	error_found = true;
	//			/*printf("(%d == %d) || (%s == %s) => %d\n",
	//				aEqInt.getFirstInput().get(),aEqInt.getSecondInput().get(),
	//				aEqStr.getFirstInput().get().c_str(),aEqStr.getSecondInput().get().c_str(),
	//				theFinalGate.getFirstInput().get());*/
	//		}
	//	}
	//}
	//cout << (error_found ? "An incorrect output has been found." : "No incorrect output found.") << endl;
	//return nbQuery;
}
