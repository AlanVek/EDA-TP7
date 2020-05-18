#pragma once

#include "GUI/GUI.h"
#include "QuadTree/QuadTree.h"

class Simulation {
public:
	Simulation(void);
	~Simulation(void);

	void dispatch(const Events&);

	const Events eventGenerator();

	bool isRunning(void);

private:

	template <class T>
	void perform(const T&, const Events&);

	void setFormat();

	/*Prevents from using copy constructor.*/
	Simulation(const Simulation&);

	GUI* gui;
	QuadTree* qt;

	bool running;
};
