#include <iostream>
#include "Simulation/Simulation.h"
int main() {
	try {
		Simulation mySim;

		/*Gets first data from GUI.*/
		mySim.getFirstData();

		/*Generates event from GUI.*/
		auto ev = mySim.eventGenerator();

		/*While user hasn't asked to exit...*/
		while (mySim.isRunning()) {
			/*Dispatches event.*/
			mySim.dispatch(ev);

			/*Gets new event from GUI.*/
			ev = mySim.eventGenerator();
		}
	}

	/*Exception handler.*/
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}