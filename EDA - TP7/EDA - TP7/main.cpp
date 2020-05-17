#include <iostream>
#include "Simulation/Simulation.h"
int main() {
	int result = -1;

	try {
		Simulation mySim;

		/*Creates a GUI event*/
		Codes ev;

		/*While user hasn't asked to exit...*/
		while (mySim.isRunning()) {
			/*Gets new event from GUI.*/
			ev = mySim.eventGenerator();

			/*Dispatches event.*/
			mySim.dispatch(ev);
		}

		result = 0;
	}

	/*Exception handler.*/
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return result;
}