#include <Application.hpp>
#include <Windows.h>
#include <iostream>
#include <ostream>
#include <ConsoleLib/ConsoleLib/Console.hpp>
#include <engine/Engine.hpp>

#include "hooks/KeyboardHookManager.hpp"

extern "C" {
  __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

std::wstring HKLToText(HKL hkl) {
  // Extract the low-order word to get the locale identifier
  LANGID langId = LOWORD(hkl);
  wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

  // Get the locale name (human-readable text) from the locale identifier
  if (LCIDToLocaleName(MAKELCID(langId, SORT_DEFAULT), localeName, LOCALE_NAME_MAX_LENGTH, 0)) {
    return std::wstring(localeName);
  }

  // Fallback: return the numeric language ID if conversion fails
  wchar_t buffer[16];
  swprintf(buffer, 16, L"0x%04X", langId);
  return std::wstring(buffer);
}


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
  Engine::SetWorkingDirectory(L"D:/Projects/WinAPI/LinguaPie/");
  Engine::Init();

  HKL hkl = GetKeyboardLayout(0); // Get the current keyboard layout
  std::wcout << L"Language: " << HKLToText(hkl) << std::endl;
  //return 0;


  {
    std::unique_ptr<Application> application = std::make_unique<Application>();
    if (application->Initialize()) {
      KeyboardHookManager::Get()->SetApplication(application.get());
      KeyboardHookManager::Get()->InstallHook();

      application->RunMainLoop();

      KeyboardHookManager::Get()->UninstallHook();
    }
  }

  Engine::Cleanup();
  Console::Get()->Pause();
  return 0;
}