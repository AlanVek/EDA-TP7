#include <iostream>
#include "Simulation/Simulation.h"
int main() {
	int result = -1;

	QuadTree qt("EDA");
	try {
		qt.compressAndSave("Simulation/QuadTree/Images/imagenfuego.png", "Simulation/QuadTree/Images/imagenfuego.EDA", 0.01);
		qt.decompressAndSave("Simulation/QuadTree/Images/imagenfuego.EDA", "Simulation/QuadTree/Images/imagenfuego2.png");
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