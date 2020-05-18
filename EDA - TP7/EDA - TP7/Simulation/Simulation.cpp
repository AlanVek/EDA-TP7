#include "Simulation.h"
#include <iostream>

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
		compressFiles();
		break;

		/*User asked to decompress.*/
	case Events::DECOMPRESS:
		decompressFiles();
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

/*Compresses files.*/
void Simulation::compressFiles() {
	try {
		/*Gets files from GUI.*/
		const auto& files = gui->getFiles();
		int pos;

		/*Loops through every file and crompressed the ones
		marked with Events::COMPRESS.*/
		for (const auto& file : files) {
			if (file.second == Events::COMPRESS) {
				pos = file.first.find_last_of(".");
				qt->compressAndSave(file.first, file.first.substr(0, pos), gui->getThreshold());
			}
		}

		/*Tells GUI to update file list.*/
		gui->updateShowStatus();
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}
void Simulation::decompressFiles() {
	try {
		/*Gets files from GUI.*/
		const auto& files = gui->getFiles();
		int pos;

		/*Loops through every file and crompressed the ones
		marked with Events::DECOMPRESS.*/
		for (const auto& file : files) {
			if (file.second == Events::DECOMPRESS) {
				pos = file.first.find_last_of(".");
				qt->decompressAndSave(file.first, file.first.substr(0, pos));
			}
		}

		/*Tells GUI to update file list.*/
		gui->updateShowStatus();
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

/*Sets new QuadTree target format.*/
void Simulation::setFormat() {
	qt->setFormat(gui->getFormat());
	gui->updateShowStatus();
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