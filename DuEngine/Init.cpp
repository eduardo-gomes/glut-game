#include "Init.hpp"

#include "audio/audio.hpp"
#include "manager/AssetsManager.hpp"
#include "scenes.hpp"
//void OnWindowResize(double fovy, double aspect){} //constant fov
Renderer *renderer;
void (*Setup)() = nullptr;
DUENGEXT void SetSetup(void (*SetupFunc)()) {
	Setup = SetupFunc;
}

//called when window is created
void window::Inicializa() {
	scene::StartImGui();
	renderer = new Renderer;
}
//update with delta in nanoseconds
void window::update(int64_t delta) {
	scene::BaseScene::GetInstance()->Update(delta);
}
void window::render() {	 //called by MainLoop
	scene::BaseScene::GetInstance()->Render();

	//New frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window::window);
	ImGui::NewFrame();
	//Call ImGui scene
	scene::BaseScene::GetInstance()->RenderGUI();
	//Render ImGui Frame
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
DUENGEXT bool Start(const std::string &WindowName, int AUDIO) {
	if (!Setup) throw std::runtime_error("Setup not defined. Define using SetSetup(void (*)())");
	Man::Manager::Instance = std::make_unique<Man::Manager>();
	window::OpenglDebugOutput = false;
	LOGDEBUG("DuEngine Version: " DUENG_VERSION);
	LOGDEBUG("Dear ImGui Version: " IMGUI_VERSION);
	LOGPTDEBUG();
	if (window::init_window(WindowName.c_str())) {
		if (AUDIO) audio::audioOut = std::make_unique<audio::audio>();
		LOGDEBUG("CALL Setup");
		Setup();
		LOGDEBUG("ENTERING window::MainLoop");
		window::MainLoop();
	} else
		logger::erro("Falied to init window");
	Man::log::close();
	return 1;
}
void CleanUp() {  //MainLoop CallBack
	delete scene::BaseScene::GetInstance();
	scene::StopImGui();
	delete renderer;
	window::close_window();
	audio::audioOut.reset();
	Man::Manager::Instance.reset();
}
void Stop() {
	window::quit = true;
}
