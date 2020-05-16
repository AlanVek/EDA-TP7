#include "Simulation.h"
#include <exception>

//Simulation constructor.
Simulation::Simulation(void) : running(true)
{
	gui = new GUI;
	qt = nullptr;
}

//Gets first data input from GUI.
void Simulation::getFirstData(void) {
	/*If user asked to leave, running will be False
	and program will exit.*/
	running = gui->firstRun();

	/*If running is true, then user loaded
	a format in which to compress files.*/
	if (running) {
		qt = new QuadTree(gui->getFormat());
	}
}

//Simulation destructor. Deletes used resources.
Simulation::~Simulation() {
	if (gui)
		delete gui;
	if (qt)
		delete qt;
}

//Polls GUI and dispatches according to button code.
void Simulation::dispatch(const codes& code) {
	switch (code) {
	case codes::END:
		running = false;
		break;
	case codes::COMPRESS:
		//compressFiles();
		break;
	case codes::DECOMPRESS:
		//decompressFiles();
		break;

	default:
		break;
	}
}

//Getter.
bool Simulation::isRunning(void) { return running; }

const codes Simulation::eventGenerator() { return gui->checkStatus(); }