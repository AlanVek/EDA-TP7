#include "Simulation.h"
#include <iostream>

//Simulation constructor.
Simulation::Simulation(void) : running(true)
{
	gui = new GUI;
	qt = new QuadTree;
}

//Polls GUI and dispatches according to button code.
void Simulation::dispatch(const Codes& code) {
	switch (code) {
		/*User asked to exit.*/
	case Codes::END:
		running = false;
		break;

		/*User asked to compress.*/
	case Codes::COMPRESS:
		compressFiles();
		break;

		/*User asked to decompress.*/
	case Codes::DECOMPRESS:
		decompressFiles();
		break;

		/*User changed target file format.*/
	case Codes::FORMAT:
		setFormat();
		break;
	default:
		break;
	}
}

/*Generates event from GUI.*/
const Codes Simulation::eventGenerator() { return gui->checkStatus(); }

/*Compresses files.*/
void Simulation::compressFiles() {
	try {
		/*Gets files from GUI.*/
		const auto& files = gui->getFiles();
		int pos;

		/*Loops through every file and crompressed the ones
		marked with Codes::COMPRESS.*/
		for (const auto& file : files) {
			if (file.second == Codes::COMPRESS) {
				pos = file.first.find_last_of(".");
				qt->compressAndSave(file.first, file.first.substr(0, pos) + "_comp_", gui->getThreshold());
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
		marked with Codes::DECOMPRESS.*/
		for (const auto& file : files) {
			if (file.second == Codes::DECOMPRESS) {
				pos = file.first.find_last_of(".");
				qt->decompressAndSave(file.first, file.first.substr(0, pos) + "_decomp_");
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