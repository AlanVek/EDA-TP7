#include "imgui/imgui.h"
#include "imgui/imgui_impl_allegro5.h"
#include "imgui/imgui_stdlib.h"
#include "GUI.h"
#include <exception>
#include <allegro5/keyboard.h>
#include <allegro5/mouse.h>
#include <allegro5/allegro_primitives.h>
#include <boost/filesystem.hpp>

#include <iostream>

namespace GUI_data {
	const unsigned int width = 750;
	const unsigned int height = 400;

	const float minThreshold = 0.1;
	const float maxThreshold = 1;

	const enum {
		okFile,
		badFile
	};
}

/*GUI constructor. Clears 'format' string and sets Allegro resources.*/
GUI::GUI(void) : threshold(GUI_data::minThreshold) {
	format.clear();

	guiDisp = nullptr;
	guiQueue = nullptr;
	action = codes::NOTHING;
	setAllegro();

	force = false;
}

/*Initializes Allegro resources and throws different
messages in case of different errors.*/
void GUI::setAllegro() {
	/*Initializes Allegro resources.*/
	if (!al_init())
		throw std::exception("Failed to initialize Allegro.");

	else if (!al_init_primitives_addon())
		throw std::exception("Failed to initialize primitives addon.");

	else if (!(al_install_keyboard()))
		throw std::exception("Failed to initialize keyboard addon.");

	else if (!(al_install_mouse()))
		throw std::exception("Failed to initialize mouse addon.");

	else if (!(guiQueue = al_create_event_queue()))
		throw std::exception("Failed to create event queue.");

	else if (!(guiDisp = al_create_display(GUI_data::width, GUI_data::height)))
		throw std::exception("Failed to create display.");

	else {
		/*Attaches events to event queue.*/
		al_register_event_source(guiQueue, al_get_keyboard_event_source());
		al_register_event_source(guiQueue, al_get_mouse_event_source());
		al_register_event_source(guiQueue, al_get_display_event_source(guiDisp));

		initialImGuiSetup();
	}
}
//Set up for GUI with ImGUI.
void GUI::initialImGuiSetup(void) const {
	al_set_target_backbuffer(guiDisp);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	al_set_new_display_flags(ALLEGRO_RESIZABLE);

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplAllegro5_Init(guiDisp);

	//Sets screen to black.
	al_clear_to_color(al_map_rgb(0, 0, 0));
}

//First GUI run. Loops until a username has been given.
bool GUI::firstRun(void) {
	bool endOfSetUp = false;
	bool result = true;

	al_set_target_backbuffer(guiDisp);

	//Drawing loop.
	while (!endOfSetUp) {
		//Clears event queue and checks if user pressed ESC or closed display.
		if (checkGUIEvents()) {
			endOfSetUp = true;
			result = false;
		}
		else {
			//Starts the Dear ImGui frame.
			ImGui_ImplAllegro5_NewFrame();
			ImGui::NewFrame();

			//Sets window.
			ImGui::SetNextWindowSize(ImVec2(GUI_data::width, GUI_data::height / 2));
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::Begin("Initial Setup");

			/*Text input for compressed file format.*/
			ImGui::Text("Compressed files format: ");
			ImGui::SameLine();
			ImGui::InputText(" ~ ", &format);
			ImGui::NewLine();
			ImGui::NewLine();

			/*Exit button.*/
			if (ImGui::Button("Exit")) {
				endOfSetUp = true;
				result = false;
			}

			ImGui::SameLine();

			/*Button to continue.*/
			if (ImGui::Button("Continue") && format.length()) {
				endOfSetUp = true;
			}

			ImGui::End();

			//Rendering.
			ImGui::Render();
			al_clear_to_color(al_map_rgb(0, 0, 0));
			ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

			al_flip_display();
		}
	}
	return result;
}
/*Checks if user pressed ESC or closed display.
It also deals with display resizing.*/
bool GUI::checkGUIEvents(void) {
	bool result = false;

	//Gets events.
	while ((al_get_next_event(guiQueue, &guiEvent)))
	{
		ImGui_ImplAllegro5_ProcessEvent(&guiEvent);

		/*If the display has been closed or if the user has pressed ESC, return true. */
		if (guiEvent.type == ALLEGRO_EVENT_DISPLAY_CLOSE || (guiEvent.type == ALLEGRO_EVENT_KEY_DOWN &&
			guiEvent.keyboard.keycode == ALLEGRO_KEY_ESCAPE))
			result = true;

		//If the display has been resized, it tells ImGUI to take care of it.
		else if (guiEvent.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
		{
			ImGui_ImplAllegro5_InvalidateDeviceObjects();
			al_acknowledge_resize(guiDisp);
			ImGui_ImplAllegro5_CreateDeviceObjects();
		}
	}
	return result;
}

//Cycle that shows menu (called with every iteration).
const codes GUI::checkStatus(void) {
	codes result = codes::NOTHING;

	al_set_target_backbuffer(guiDisp);

	//If user pressed ESC or closed display, returns codes::END.
	if (checkGUIEvents()) {
		result = codes::END;
	}
	else {
		//Sets new ImGUI frame.
		ImGui_ImplAllegro5_NewFrame();
		ImGui::NewFrame();

		//Sets new window.
		ImGui::SetNextWindowSize(ImVec2(GUI_data::width, GUI_data::height));
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("EDA - TP6");

		/*Text input for file format.*/
		ImGui::Text("Compressed files format: ");
		ImGui::SameLine();
		ImGui::InputText(" ~ ", &format);

		ImGui::NewLine();
		ImGui::NewLine();

		/*Slider for threshold.*/
		ImGui::Text("Compression threshold:   ");
		ImGui::SameLine();
		ImGui::SliderFloat(" - ", &threshold, GUI_data::minThreshold, GUI_data::maxThreshold);

		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::Text("Actions ");

		if (ImGui::Button("Compress")) {
			action = codes::COMPRESS;
		}
		ImGui::SameLine();
		if (ImGui::Button("Decompress")) {
			action = codes::DECOMPRESS;
		}

		const auto show = [this](const char* path = nullptr) {
			if (force) { force = !force; return fs.pathContent(path, true); }
			return fs.pathContent(path, false);
		};

		ImGui::NewLine();
		ImGui::NewLine();

		ImGui::Text("Current path: ");
		ImGui::SameLine();

		std::string path = fs.getPath();
		ImGui::Text(path.c_str());
		for (const auto& file : show()) {
			if (Filesystem::isDir((path + '\\' + file).c_str())) {
				if (ImGui::Button(file.c_str())) {
					show(file.c_str());
					files.clear();
				}
			}
			else {
				if (ImGui::Checkbox(file.c_str(), (bool*)&files[path + '\\' + file])) {
					if ((bool)files[path + '\\' + file])
						files[path + '\\' + file] = action;
					else
						files[path + '\\' + file] = codes::NOTHING;
				}

				ImGui::NextColumn();
			}
		}

		/*Buttons for compression and decompression.*/
		ImGui::NewLine();
		ImGui::NewLine();

		/*Exit button.*/
		if (ImGui::Button("Exit"))
			result = codes::END;

		ImGui::SameLine();
		if (ImGui::Button("Back"))
			fs.back();

		/*Perform button.*/
		ImGui::SameLine();
		if (ImGui::Button("Perform")) {
			result = action;
		}

		ImGui::End();

		//Rendering.
		ImGui::Render();
		al_clear_to_color(al_map_rgb(0, 0, 0));
		ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

		al_flip_display();
	}
	return result;
}

/*Getters.*/
const std::string& GUI::getFormat(void) { return format; }
const float GUI::getThreshold(void) { return threshold; }
const std::map<std::string, codes>& GUI::getFiles(void) { return files; }

void GUI::updateShowStatus() { force = !force; }

//Cleanup. Frees resources.
GUI::~GUI() {
	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();
	if (guiQueue)
		al_destroy_event_queue(guiQueue);
	if (guiDisp)
		al_destroy_display(guiDisp);
}