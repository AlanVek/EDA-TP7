#include "imgui/imgui.h"
#include "imgui/imgui_impl_allegro5.h"
#include "imgui/imgui_stdlib.h"
#include "GUI.h"
#include <allegro5/keyboard.h>
#include <allegro5/mouse.h>
#include <allegro5/allegro_primitives.h>

namespace GUI_data {
	const unsigned int width = 750;
	const unsigned int height = 400;

	const float minThreshold = 0.1;
	const float maxThreshold = 1;
}

/*GUI constructor. Clears 'format' string and sets Allegro resources.*/
GUI::GUI(void) : threshold(GUI_data::minThreshold) {
	format.clear();

	guiDisp = nullptr;
	guiQueue = nullptr;
	action = Codes::NOTHING;
	setAllegro();

	force = false;
	deep = 0;
	action_msg = "none.";
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
const Codes GUI::checkStatus(void) {
	Codes result = Codes::NOTHING;
	Codes temp;

	al_set_target_backbuffer(guiDisp);

	//If user pressed ESC or closed display, returns Codes::END.
	if (checkGUIEvents()) {
		result = Codes::END;
	}
	else {
		/*Sets new ImGui window.*/
		newWindow();

		/*Text input for file format.*/
		temp = displayFormat();
		if ((bool)temp) result = temp;

		ImGui::NewLine();
		ImGui::NewLine();

		/*Actions for compression and decompression.*/
		displayActions();

		ImGui::NewLine();
		ImGui::NewLine();

		/*Slider for threshold.*/
		displayThreshold();

		ImGui::NewLine();
		ImGui::NewLine();

		/*Files from path.*/
		displayFiles();

		ImGui::NewLine();
		ImGui::NewLine();

		/*Back button.*/
		displayBackButton();

		ImGui::SameLine();

		/*Exit button.*/
		temp = displayExitButton();
		if ((bool)temp) result = temp;

		ImGui::SameLine();

		/*Perform button.*/
		temp = displayPerformButton();
		if ((bool)temp) result = temp;

		ImGui::End();

		/*Rendering.*/
		render();
	}
	return result;
}

/*Displays action buttons.*/
inline void GUI::displayActions() {
	ImGui::Text("Actions: ");

	/*Compress button.*/
	if (ImGui::Button("Compress") && format.length()) {
		action = Codes::COMPRESS;
		action_msg = "compression.";
	}

	/*Decompress button.*/
	ImGui::SameLine();
	if (ImGui::Button("Decompress") && format.length()) {
		action = Codes::DECOMPRESS;
		action_msg = "decompression.";
	}

	/*Message with selected option.*/
	ImGui::Text(("Selected: " + action_msg).c_str());
}

/*Displays threshold slider.*/
inline void GUI::displayThreshold() {
	ImGui::Text("Compression threshold:   ");
	ImGui::SameLine();
	ImGui::SliderFloat(" - ", &threshold, GUI_data::minThreshold, GUI_data::maxThreshold);
}

/*Displays text input for file format.*/
inline Codes GUI::displayFormat() {
	Codes result = Codes::NOTHING;
	ImGui::Text("Compressed files format: ");
	ImGui::SameLine();
	if (ImGui::InputText(" ~ ", &format) && format.length())
		result = Codes::FORMAT;

	return result;
}

/*Displays path and files/folders in path.*/
void GUI::displayFiles() {
	std::string path = fs.getPath();

	/*Binding fs.pathContent with this->force.
	Helps to determine when to update file info.*/
	const auto show = [this](const char* path = nullptr) {
		if (force) { force = !force; return fs.pathContent(path, true); }
		return fs.pathContent(path, false);
	};

	ImGui::Text("Current path: ");
	ImGui::SameLine();

	/*Shows path.*/
	ImGui::TextWrapped(path.c_str());

	/*Loops through every file in files map.*/
	for (const auto& file : show()) {
		/*If it's a directory...*/
		if (Filesystem::isDir((path + '\\' + file).c_str())) {
			/*Sets a button with its name.*/
			if (ImGui::Button(file.c_str())) {
				/*If the button is pressed, the path changes and
				goes inside the folder.*/
				show(file.c_str());
				files.clear();
				deep++;
			}
		}

		/*If it's a file...*/
		else if (Filesystem::isFile((path + '\\' + file).c_str())) {
			/*Sets a checkbox with its name. Updates file's value in map.*/
			if (ImGui::Checkbox(file.c_str(), (bool*)&files[path + '\\' + file])) {
				/*Replaces action in file's value in map.*/
				if ((bool)files[path + '\\' + file])
					files[path + '\\' + file] = action;
			}

			ImGui::NextColumn();
		}
	}
}

/*Displays button to go back.*/
inline void GUI::displayBackButton() {
	/*When pressed, it calls fs.back(), which updates the path.*/
	if (ImGui::ArrowButton("Back", ImGuiDir_Left) && deep) {
		fs.back();
		deep--;
	}
}

/*Displays exit button.*/
inline Codes GUI::displayExitButton() {
	Codes result = Codes::NOTHING;

	/*If pressed, it returns Codes::END.*/
	if (ImGui::Button("Exit"))
		result = Codes::END;

	return result;
}

/*Displays perform button.*/
inline Codes GUI::displayPerformButton() {
	Codes result = Codes::NOTHING;

	/*If pressed, it returns the corresponding action.*/
	if (ImGui::Button("Perform")) {
		result = action;
	}
	return result;
}

/*Sets a new ImGUI frame and window.*/
inline void GUI::newWindow() {
	//Sets new ImGUI frame.
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	//Sets new window positioned at (0,0).
	ImGui::SetNextWindowSize(ImVec2(GUI_data::width, GUI_data::height));
	ImGui::SetNextWindowPos(ImVec2(0, 0));

	/*Begins window and sets title.*/
	ImGui::Begin("EDA - TP7");
}

/*Rendering.*/
inline void GUI::render() {
	ImGui::Render();
	al_clear_to_color(al_map_rgb(0, 0, 0));
	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

	al_flip_display();
}

/*Getters.*/
const std::string& GUI::getFormat(void) { return format; }
const float GUI::getThreshold(void) { return threshold; }
const std::map<std::string, Codes>& GUI::getFiles(void) { return files; }

/*Toggles 'force' variable.*/
void GUI::updateShowStatus() { force = !force; }

/*Cleanup. Frees resources.*/
GUI::~GUI() {
	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();
	if (guiQueue)
		al_destroy_event_queue(guiQueue);
	if (guiDisp)
		al_destroy_display(guiDisp);
}