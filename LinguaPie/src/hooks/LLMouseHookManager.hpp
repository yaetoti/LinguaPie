#pragma once

#include <Windows.h>
#include <utils/EventDispatcher.hpp>
#include <hooks/events/LLMouseHookEvent.hpp>

struct LLMouseHookManager final {
  LLMouseHookManager();
  ~LLMouseHookManager();

  bool Initialize();
  void Cleanup();

  bool InstallHook();
  void UninstallHook();

  EventDispatcher<LLMouseHookEvent>* GetDispatcher();

  static LLMouseHookManager* Get();
  static void Shutdown();

private:
  LRESULT HandleEvent(int nCode, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
  HHOOK m_hook;
  EventDispatcher<LLMouseHookEvent> m_dispatcher;
};
