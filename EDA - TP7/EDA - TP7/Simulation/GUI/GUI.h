#pragma once

#include <allegro5/allegro.h>
#include <map>
#include "Filesystem/Filesystem.h"

/*GUI event codes.*/
/********************************/
const enum class Events : int {
	NOTHING = 0,
	END,
	FORMAT,
	COMPRESS,
	DECOMPRESS
};
/********************************/

class GUI {
public:

	GUI();

	~GUI();

	const Events checkStatus(void);

	const std::string& getFormat() const;
	const float getThreshold() const;

	const std::map<std::string, Events>& getFiles(void) const;
	void updateShowStatus(void);
protected:

	/*Initial setup.*/
	/**********************************/
	void setAllegro();
	void initialImGuiSetup(void) const;
	/**********************************/

	/*Window displayers.*/
	/*************************************************************************************************/
	inline void newWindow() const;
	inline void displayPath();
	inline Events displayFormat();
	inline void displayActions();
	void displayFiles();

	template <class Widget, class F1, class F2 = void(*)(void)>
	inline auto displayWidget(const Widget&, const F1& f1, const F2 & = []() {}) -> decltype(f1());

	template <class F1, class F2 = void(*)(void)>
	inline auto displayWidget(const char*, const F1& f1, const F2 & = []() {})->decltype(f1());

	inline void render() const;
	/*************************************************************************************************/

	/*Exit and resize events.*/
	bool eventManager(void);

	/*Allegro data members.*/
	/******************************/
	ALLEGRO_DISPLAY* guiDisp;
	ALLEGRO_EVENT_QUEUE* guiQueue;
	ALLEGRO_EVENT guiEvent;
	/******************************/

	/*Flag data members.*/
	/******************************/
	bool force;
	int deep;
	std::string action_msg;
	Events action;
	/******************************/

	/*Data members modifiable by user.*/
	/**********************************/
	float threshold;
	std::string format, path;
	/**********************************/

	/*File handling.*/
	/*************************************************************/
	Filesystem fs;
	std::map <std::string, Events> files;
	const std::vector<std::string>& show(const char* = nullptr);
	/*************************************************************/
};
