#pragma once

#include <allegro5/allegro.h>
#include <map>
#include "Filesystem/Filesystem.h"

const enum class Codes : int {
	NOTHING = 0,
	END,
	FORMAT,
	COMPRESS,
	DECOMPRESS
};

class GUI {
public:

	GUI();

	~GUI();

	const Codes checkStatus(void);

	const std::string& getFormat();
	const float getThreshold();

	const std::map<std::string, Codes>& getFiles(void);

	void updateShowStatus();

protected:

	/*Initial setup.*/
	void setAllegro();
	void initialImGuiSetup(void) const;

	/*Window displayers.*/
	inline void newWindow();
	inline Codes displayFormat();
	inline void displayActions();
	inline void displayThreshold();
	void displayFiles();
	inline void displayBackButton();
	inline Codes displayExitButton();
	inline Codes displayPerformButton();
	inline void render();

	/*Exit and resize events.*/
	bool checkGUIEvents(void);

	/*Allegro data members.*/
	ALLEGRO_DISPLAY* guiDisp;
	ALLEGRO_EVENT_QUEUE* guiQueue;
	ALLEGRO_EVENT guiEvent;

	/*Flag data members.*/
	bool force;
	int deep;
	std::string action_msg;
	Codes action;

	/*Data members modifiable by user.*/
	float threshold;
	std::string format;

	/*File handling.*/
	Filesystem fs;
	std::map <std::string, Codes> files;
};
