#include "imgui/imgui.h"
#include "imgui/imgui_impl_allegro5.h"
#include "imgui/imgui_stdlib.h"
#include "GUI.h"
#include <allegro5/keyboard.h>
#include <allegro5/mouse.h>
#include <allegro5/allegro_primitives.h>
#include <functional>

/*GUI data.*/
/***************************************/
namespace data {
	const unsigned int width = 920;
	const unsigned int height = 500;

	const float minThreshold = 0.1;
	const float maxThreshold = 1;

	const char* fixedFormat = ".png";
}
/***************************************/

/*GUI constructor. Clears 'format' string and sets Allegro resources.*/
GUI::GUI(void) :
	threshold(data::minThreshold),
	guiDisp(nullptr),
	guiQueue(nullptr),
	action(Events::COMPRESS),
	force(true),
	deep(0),
	action_msg("compression."),
	showingFormat(data::fixedFormat)
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

	else if (!(guiDisp = al_create_display(data::width, data::height)))
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
bool GUI::eventManager(void) {
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
const Events GUI::checkStatus(void) {
	Events result;

	al_set_target_backbuffer(guiDisp);

	//If user pressed ESC or closed display, returns Events::END.
	if (eventManager())
		result = Events::END;

	else {
		/*Sets new ImGui window.*/
		newWindow();

		/*Text input for new path.*/
		displayPath();

		ImGui::NewLine(); ImGui::NewLine();

		/*Text input for file format.*/
		result = displayFormat();

		ImGui::NewLine(); ImGui::NewLine();

		/*Actions for compression and decompression.*/
		displayActions();

		ImGui::NewLine(); ImGui::NewLine();

		/*Slider for threshold.*/
		ImGui::Text("Compression threshold: ");
		ImGui::SameLine();
		ImGui::SliderFloat("-", &threshold, data::minThreshold, data::maxThreshold);

		ImGui::NewLine(); ImGui::NewLine();

		/*Files from path.*/
		displayFiles();

		ImGui::NewLine();

		/*Back button.*/
		displayWidget("<-", [this]() {if (deep) { fs.back(); deep--; path = fs.getPath(); }});

		ImGui::SameLine();

		/*Exit button.*/
		displayWidget("Exit", [&result]() {result = Events::END; });

		ImGui::SameLine();

		/*Perform button.*/
		displayWidget("Perform", [this, &result]() {result = action; });

		ImGui::End();

		/*Rendering.*/
		render();
	}
	return result;
}

/*Displays action buttons.*/
inline void GUI::displayActions() {
	ImGui::Text("Action to perform: ");

	/*Button callback for both buttons.*/
	const auto button_callback = [this](const Events code, const char* msg, const std::string& newFormat) {
		action = code;
		action_msg = msg;
		files.clear();
		force = true;
		showingFormat = newFormat;
	};

	/*Compress button.*/
	displayWidget("Compress", std::bind(button_callback, Events::COMPRESS, "compression.", data::fixedFormat));
	ImGui::SameLine();

	/*Decompress button.*/
	displayWidget("Decompress", std::bind(button_callback, Events::DECOMPRESS, "decompression.", format));
	ImGui::SameLine();

	/*Message with selected option.*/
	ImGui::Text(("Selected: " + action_msg).c_str());
}

/*Displays text input for file format.*/
inline Events GUI::displayFormat() {
	ImGui::Text("Compression format:    ");
	ImGui::SameLine();
	if (ImGui::InputText(" ~ ", &format, ImGuiInputTextFlags_CharsNoBlank) && format.length()) {
		format = '.' + format.substr(format.find_last_of('.') + 1, format.length());
		force = true;
		if (action == Events::DECOMPRESS)
			showingFormat = format;
		return Events::FORMAT;
	}

	return Events::NOTHING;
}

/*Displays text input for path.*/
inline void GUI::displayPath() {
	ImGui::Text("New path:              ");
	ImGui::SameLine();
	ImGui::InputText(" ", &path);

	ImGui::SameLine();
	displayWidget("Go", [this]() {fs.newPath(path); deep = 0; });

	ImGui::SameLine();
	displayWidget("Reset path", [this]() {path = fs.originalPath();  fs.newPath(path); deep = 0; });
}

/*Displays path and files/folders in path.*/
void GUI::displayFiles() {
	std::string tempPath = fs.getPath();

	ImGui::Text("Current path: ");
	ImGui::SameLine();

	/*Shows path.*/
	ImGui::TextWrapped(tempPath.c_str());

	ImGui::NewLine();
	ImGui::Text(("Showing format: " + showingFormat).c_str());

	/*'Select all' button.*/
	displayWidget("Select All", [this]() {for (auto& file : files)file.second = action; });

	ImGui::SameLine();

	/*'Deselect all' button.*/
	displayWidget("Deselect all", [this]() {for (auto& file : files) file.second = Events::NOTHING; });

	//ImGui::NewLine();
	ImGui::Text("-----------------------------------");
	/*Loops through every file in files map.*/
	for (const auto& file : updateFiles()) {
		/*If it's a directory...*/
		if (Filesystem::isDir((tempPath + '\\' + file).c_str())) {
			/*Sets a button with its name. If pressed, it updates path,
			clears files map for new files and increases depth flag.*/
			displayWidget(file.c_str(),

				[this, &file, &tempPath]() {
					fs.newPath(tempPath + '\\' + file);
					files.clear();
					deep++;
					path = tempPath + '\\' + file;
				});
		}

		/*If it's a file...*/
		else if (Filesystem::isFile((tempPath + '\\' + file).c_str())) {
			Events& checker = files[tempPath + '\\' + file];

			/*Sets a checkbox with its name. Updates file's value in map.*/
			displayWidget(std::bind(ImGui::Checkbox, file.c_str(), (bool*)&checker),
				[&checker, this]() {
					if ((bool)checker && format.length()) checker = action;
					else checker = Events::NOTHING; });
		}
	}
	ImGui::Text("-----------------------------------");
}

/*Sets a new ImGUI frame and window.*/
inline void GUI::newWindow() const {
	//Sets new ImGUI frame.
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	//Sets new window positioned at (0,0).
	ImGui::SetNextWindowSize(ImVec2(data::width, data::height));
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
const std::map<std::string, Events>& GUI::getFiles(void) const { return files; }

/*Cleanup. Frees resources.*/
GUI::~GUI() {
	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();
	if (guiQueue)
		al_destroy_event_queue(guiQueue);
	if (guiDisp)
		al_destroy_display(guiDisp);
}

/*Displays given widget and applies callback according to widget state.*/
template <class Widget, class F1, class F2>
inline auto GUI::displayWidget(const Widget& widget, const F1& f1, const F2& f2) -> decltype(f1())
{
	if (widget())
		return f1();
	return f2();
}

/*Specialization of displayWidget template.
As ImGui::Button is the most used widget, when the given 'widget'
is actually a const char*, then the widget will be ImGui::Button.*/
template <class F1, class F2>
inline auto GUI::displayWidget(const char* txt, const F1& f1, const F2& f2)->decltype(f1()) {
	if (ImGui::Button(txt))
		return f1();
	return f2();
}

/*Binding fs.pathContent with this->force and specified file format.
Helps to determine when to update file info.*/
const std::vector<std::string>& GUI::updateFiles(const char* path) {
	bool shouldForce = force;

	if (force) { force = !force; }

	return fs.pathContent(path, shouldForce, { showingFormat.c_str() });
}