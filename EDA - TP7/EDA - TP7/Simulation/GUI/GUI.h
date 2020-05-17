#pragma once

#include <allegro5/allegro.h>
#include <map>
#include "Filesystem/Filesystem.h"

const enum class codes : int {
	NOTHING = 0,
	END,
	FORMAT,
	COMPRESS,
	DECOMPRESS,
};

class GUI {
public:

	GUI();

	~GUI();

	const codes checkStatus(void);

	bool firstRun(void);

	const std::string& getFormat();
	const float getThreshold();

	void setAllegro();

	const std::map<std::string, codes>& getFiles(void);

protected:
	void initialImGuiSetup(void) const;

	ALLEGRO_DISPLAY* guiDisp;
	ALLEGRO_EVENT_QUEUE* guiQueue;
	ALLEGRO_EVENT guiEvent;
	bool checkGUIEvents(void);

	std::string format;
	float threshold;

	bool force;

	std::map <std::string, codes> files;
	codes action;
	Filesystem fs;
};
