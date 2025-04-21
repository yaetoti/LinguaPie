#include <Application.hpp>
#include <Windows.h>
#include <iostream>
#include <ostream>
#include <ConsoleLib/ConsoleLib/Console.hpp>
#include <engine/Engine.hpp>

#include "hooks/KeyboardHookManager.hpp"
#include <hooks/LLMouseHookManager.hpp>

extern "C" {
  __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
  Engine::SetWorkingDirectory(L"D:/Projects/WinAPI/LinguaPie/");
  //Engine::SetWorkingDirectory(L"");
  //Console::Get()->RedirectStdHandles();
  Engine::Init();

  {
    std::unique_ptr<Application> application = std::make_unique<Application>();
    if (application->Initialize()) {
      // Keyboard hook
      //KeyboardHookManager::Get()->SetApplication(application.get());
      //KeyboardHookManager::Get()->InstallHook();
      // Mouse hook
      LLMouseHookManager::Get()->GetDispatcher()->AddListener(application.get());
      LLMouseHookManager::Get()->InstallHook();

      // Run
      application->RunMainLoop();

      // Shutdown
      KeyboardHookManager::Get()->UninstallHook();
      LLMouseHookManager::Shutdown();
    }
  }

  Engine::Cleanup();
  //Console::Get()->Pause();
  return 0;
}