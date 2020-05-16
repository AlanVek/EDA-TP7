#pragma once

#include "GUI/GUI.h"
#include "QuadTree/QuadTree.h"

class Simulation {
public:
	Simulation(void);
	~Simulation(void);

	void dispatch(const codes&);

	const codes eventGenerator();

	bool isRunning(void);

	void getFirstData(void);

	void decompressFiles();
	void compressFiles();

private:
	/*Prevents from using copy constructor.*/
	Simulation(const Simulation&);

	GUI* gui;
	QuadTree* qt;

	bool running;
};
