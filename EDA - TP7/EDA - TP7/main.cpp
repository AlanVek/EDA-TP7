#include <iostream>
#include "Simulation/Simulation.h"
int main() {
	int result = -1;

	QuadTree qt("EDA");
	try {
		qt.compressAndSave("Simulation/QuadTree/Images/dice.png", "Simulation/QuadTree/Images/dice.EDA", 0.01);
		//qt.decompressAndSave("Simulation/QuadTree/Images/dice.EDA", "Simulation/QuadTree/Images/dice2.png");
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;

	try {
		Simulation mySim;

		/*Creates a GUI event*/
		Events ev;

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