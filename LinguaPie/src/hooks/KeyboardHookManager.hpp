#pragma once
#include <Windows.h>

struct Application;

struct KeyboardHookManager final {
  KeyboardHookManager();
  ~KeyboardHookManager();

  void SetApplication(Application* application);
  bool InstallHook();
  void UninstallHook();

  static KeyboardHookManager* Get();
  static void Destroy();

private:
  KeyboardHookManager(const KeyboardHookManager&) = delete;
  KeyboardHookManager& operator=(const KeyboardHookManager&) = delete;
  KeyboardHookManager(KeyboardHookManager&&) = delete;
  KeyboardHookManager& operator=(KeyboardHookManager&&) = delete;

  LRESULT HandleEvent(int nCode, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
  Application* m_application;
  HHOOK m_hook;

  inline static KeyboardHookManager* m_instance = nullptr;
};
