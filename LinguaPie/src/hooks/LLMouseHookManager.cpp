#include <cassert>
#include "LLMouseHookManager.hpp"

static LLMouseHookManager* s_instance = nullptr;

LLMouseHookManager::LLMouseHookManager()
: m_hook(nullptr) {

}

LLMouseHookManager::~LLMouseHookManager() {
  Cleanup();
}

bool LLMouseHookManager::Initialize() {
  return true;
}

void LLMouseHookManager::Cleanup() {
  UninstallHook();
}

bool LLMouseHookManager::InstallHook() {
  UninstallHook();
  m_hook = SetWindowsHookExW(WH_MOUSE_LL, MouseProc, nullptr, 0);
  return m_hook != nullptr;
}

void LLMouseHookManager::UninstallHook() {
  if (m_hook) {
    UnhookWindowsHookEx(m_hook);
    m_hook = nullptr;
  }
}

LRESULT LLMouseHookManager::HandleEvent(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode != HC_ACTION) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }

  LLMouseHookEvent event(nCode, wParam, lParam);
  m_dispatcher.Dispatch(event, true);

  if (event.HasReturnValue()) {
    return event.GetReturnValue();
  }

  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT LLMouseHookManager::MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
  return LLMouseHookManager::Get()->HandleEvent(nCode, wParam, lParam);
}

LLMouseHookManager* LLMouseHookManager::Get() {
  if (!s_instance) {
    s_instance = new LLMouseHookManager();
  }

  return s_instance;
}

void LLMouseHookManager::Shutdown() {
  assert(s_instance);
  delete s_instance;
  s_instance = nullptr;
}

EventDispatcher<LLMouseHookEvent>* LLMouseHookManager::GetDispatcher() {
  return &m_dispatcher;
}
