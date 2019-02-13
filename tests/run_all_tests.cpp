#include <iostream>
#include <gtest/gtest.h>
#include <systemc>
#include <qmg2sc.h>

int sc_main(int argc, char* argv[]) {
	::hv::cfg::Broker myBroker("MyGlobalBroker");
	srand(time(NULL));
	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();
	return ret;
}
