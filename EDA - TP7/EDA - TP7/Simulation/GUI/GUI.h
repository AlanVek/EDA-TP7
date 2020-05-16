#pragma once

#include <allegro5/allegro.h>
#include <string>

const enum class codes {
	NOTHING,
	END,
	FORMAT,
	COMPRESS,
	DECOMPRESS
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

protected:
	void initialImGuiSetup(void) const;

	ALLEGRO_DISPLAY* guiDisp;
	ALLEGRO_EVENT_QUEUE* guiQueue;
	ALLEGRO_EVENT guiEvent;
	bool checkGUIEvents(void);

	std::string format;
	float threshold;
};
