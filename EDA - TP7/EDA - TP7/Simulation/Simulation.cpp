#include "Simulation.h"
#include <iostream>
#include <functional>

using namespace std::placeholders;

//Simulation constructor.
Simulation::Simulation(void) : running(true)
{
	gui = new GUI;
	qt = new QuadTree;
}

//Polls GUI and dispatches according to button code.
void Simulation::dispatch(const Events& code) {
	switch (code) {
		/*User asked to exit.*/
	case Events::END:
		running = false;
		break;

		/*User asked to compress.*/
	case Events::COMPRESS:
		perform(std::bind(&QuadTree::compressAndSave, qt, _1, _2, gui->getThreshold()), Events::COMPRESS);
		break;

		/*User asked to decompress.*/
	case Events::DECOMPRESS:
		perform(std::bind(&QuadTree::decompressAndSave, qt, _1, _2), Events::DECOMPRESS);
		break;

		/*User changed target file format.*/
	case Events::FORMAT:
		setFormat();
		break;
	default:
		break;
	}
}

/*Generates event from GUI.*/
const Events Simulation::eventGenerator() { return gui->checkStatus(); }

/*Applies a function that takes two strings to
every file in gui->getFiles() marked with 'ev'.*/
template <class T>
void Simulation::perform(const T& apply, const Events& ev) {
	try {
		/*Gets files from GUI.*/
		const auto& files = gui->getFiles();
		int pos;

		/*Loops through every file and applies function to
		the ones marked with 'ev'.*/
		for (const auto& file : files) {
			if (file.second == ev) {
				pos = file.first.find_last_of(".");
				apply(file.first, file.first.substr(0, pos));
			}
		}
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

/*Sets new QuadTree target format.*/
void Simulation::setFormat() {
	qt->setFormat(gui->getFormat());
}

/*Getter.*/
bool Simulation::isRunning(void) { return running; }

/*Simulation destructor. Deletes used resources.*/
Simulation::~Simulation() {
	if (gui)
		delete gui;
	if (qt)
		delete qt;
}