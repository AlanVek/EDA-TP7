#include "imgui/imgui.h"
#include "imgui/imgui_impl_allegro5.h"
#include "imgui/imgui_stdlib.h"
#include "GUI.h"
#include <allegro5/keyboard.h>
#include <allegro5/mouse.h>
#include <allegro5/allegro_primitives.h>
#include <functional>
#include <ImgUtil.h>

namespace GUI_data {
	const unsigned int width = 920;
	const unsigned int height = 500;

	const float minThreshold = 0.1;
	const float maxThreshold = 1;

	const char* fixedFormat = ".png";
}

/*GUI constructor. Clears 'format' string and sets Allegro resources.*/
GUI::GUI(void) :
	threshold(GUI_data::minThreshold),
	guiDisp(nullptr),
	guiQueue(nullptr),
	action(Codes::NOTHING),
	force(true),
	deep(0),
	action_msg("none.")
{
	format.clear();

	setAllegro();

	path = fs.getPath();
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

/*Displays action buttons.*/
inline void GUI::displayActions() {
	ImGui::Text("Action to perform: ");

	/*Button callback for both buttons.*/
	const auto button_callback = [this](const Codes code, const char* msg) {
		action = code;
		action_msg = msg;
		files.clear();
		force = true;
	};

	/*Compress button.*/
	displayButton("Compress", std::bind(button_callback, Codes::COMPRESS, "compression."));
	ImGui::SameLine();

	/*Decompress button.*/
	displayButton("Decompress", std::bind(button_callback, Codes::DECOMPRESS, "decompression."));
	ImGui::SameLine();

	/*Message with selected option.*/
	ImGui::Text(("Selected: " + action_msg).c_str());
}

/*Displays threshold slider.*/
inline void GUI::displayThreshold() {
	ImGui::Text("Compression threshold: ");
	ImGui::SameLine();
	ImGui::SliderFloat(" - ", &threshold, GUI_data::minThreshold, GUI_data::maxThreshold);
}

/*Displays text input for file format.*/
inline Codes GUI::displayFormat() {
	ImGui::Text("Compression format:    ");
	ImGui::SameLine();
	int finder;
	if (ImGui::InputText(" ~ ", &format, ImGuiInputTextFlags_CharsHexadecimal) && format.length()) {
		finder = format.find_last_of('.');
		format = '.' + format.substr(finder + 1, format.length());
		force = true;
		return Codes::FORMAT;
	}

	return Codes::NOTHING;
}

/*Displays text input for path.*/
inline void GUI::displayPath() {
	ImGui::Text("New path:              ");
	ImGui::SameLine();
	if (ImGui::InputText("", &path, ImGuiInputTextFlags_CharsHexadecimal));

	ImGui::SameLine();

	displayButton("Go", std::bind(&Filesystem::newPath, &fs, path));
	ImGui::SameLine();

	displayButton("Reset path", std::bind(&Filesystem::newPath, &fs, Filesystem::currentPath()));
}

/*Displays path and files/folders in path.*/
void GUI::displayFiles() {
	path = fs.getPath();

	ImGui::Text("Current path: ");
	ImGui::SameLine();

	/*Shows path.*/
	ImGui::TextWrapped(path.c_str());

	ImGui::NewLine();

	/*'Select all' button.*/

	displayButton("Select All",
		[this]() {for (auto& file : files)file.second = action; });

	ImGui::SameLine();

	displayButton("Deselect all",
		[this]() {for (auto& file : files) file.second = action; });

	ImGui::NewLine();
	ImGui::Text("-----------------------------------");
	/*Loops through every file in files map.*/
	for (const auto& file : show()) {
		/*If it's a directory...*/
		if (Filesystem::isDir((path + '\\' + file).c_str())) {
			displayButton
			(
				file.c_str(),

				[this, &file]() {
					fs.newPath(path + '\\' + file);
					files.clear();
					deep++;
				}
			);
		}

		/*If it's a file...*/
		else if (Filesystem::isFile((path + '\\' + file).c_str())) {
			Codes& checker = files[path + '\\' + file];

			/*Sets a checkbox with its name. Updates file's value in map.*/
			if (ImGui::Checkbox(file.c_str(), (bool*)&checker)) {
				/*Replaces action in file's value in map.*/
				if ((bool*)checker)
					checker = action;
			}

			ImGui::NextColumn();
		}
	}
	ImGui::Text("-----------------------------------");
}

/*Displays button to go back.*/
inline void GUI::displayBackButton() {
	/*When pressed, it calls fs.back(), which updates the path.*/
	if (ImGui::ArrowButton("Back", ImGuiDir_Left) && deep) {
		fs.back();
		deep--;
	}
}

/*Sets a new ImGUI frame and window.*/
inline void GUI::newWindow() const {
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
inline void GUI::render() const {
	ImGui::Render();
	al_clear_to_color(al_map_rgb(0, 0, 0));
	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

	al_flip_display();
}

/*Getters.*/
const std::string& GUI::getFormat(void) const { return format; }
const float GUI::getThreshold(void) const { return threshold; }
const std::map<std::string, Codes>& GUI::getFiles(void) const { return files; }

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

//Cycle that shows menu (called with every iteration).
const Codes GUI::checkStatus(void) {
	Codes result;

	al_set_target_backbuffer(guiDisp);

	//If user pressed ESC or closed display, returns Codes::END.
	if (checkGUIEvents()) {
		result = Codes::END;
	}
	else {
		/*Sets new ImGui window.*/
		newWindow();

		/*Text input for new path.*/
		displayPath();

		ImGui::NewLine();
		ImGui::NewLine();

		/*Text input for file format.*/
		result = displayFormat();

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
		displayButton("Exit", [&result]() {result = Codes::END; }, [&result]() {result = Codes::NOTHING; });

		ImGui::SameLine();

		/*Perform button.*/
		displayButton("Perform", [this, &result]() {result = action; }, [&result]() {result = Codes::NOTHING; });

		ImGui::End();

		/*Rendering.*/
		render();
	}
	return result;
}

template <class F1, class F2>
inline auto GUI::displayButton(const char* text, const F1& f1, const F2& f2) -> decltype(f1())
{
	if (ImGui::Button(text))
		return f1();
	return f2();
}

/*Binding fs.pathContent with this->force and specified file format.
Helps to determine when to update file info.*/
const std::vector<std::string>& GUI::show(const char* path) {
	bool shouldForce = force;
	const char* showFormat;

	if (force) { force = !force; }
	if (action == Codes::COMPRESS || action == Codes::NOTHING)
		showFormat = GUI_data::fixedFormat;
	else
		showFormat = format.c_str();

	return fs.pathContent(path, shouldForce, 1, showFormat);
}