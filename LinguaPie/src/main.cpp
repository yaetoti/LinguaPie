#include <Application.hpp>
#include <Windows.h>
#include <iostream>
#include <ostream>
#include <engine/Engine.hpp>

extern "C" {
  __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
  Engine::SetWorkingDirectory(L"./");
  Engine::Init();

  {
    std::unique_ptr<Application> application = std::make_unique<Application>();
    application->RunMainLoop();
  }

  Engine::Cleanup();
  return 0;
}