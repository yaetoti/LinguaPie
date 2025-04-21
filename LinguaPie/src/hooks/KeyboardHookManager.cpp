#include "KeyboardHookManager.hpp"

#include <cassert>

#include "Application.hpp"

KeyboardHookManager::KeyboardHookManager()
: m_application(nullptr)
, m_hook(nullptr) {
}

KeyboardHookManager::~KeyboardHookManager() {
  UninstallHook();
}

void KeyboardHookManager::SetApplication(Application* application) {
  m_application = application;
}

bool KeyboardHookManager::InstallHook() {
  UninstallHook();
  m_hook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, nullptr, 0);
  assert(m_hook != nullptr);
  return m_hook != nullptr;
}

void KeyboardHookManager::UninstallHook() {
  if (m_hook) {
    UnhookWindowsHookEx(m_hook);
    m_hook = nullptr;
  }
}

KeyboardHookManager* KeyboardHookManager::Get() {
  if (!m_instance) {
    m_instance = new KeyboardHookManager();
  }

  return m_instance;
}

void KeyboardHookManager::Destroy() {
  if (m_instance) {
    delete m_instance;
    m_instance = nullptr;
  }
}

LRESULT KeyboardHookManager::HandleEvent(int nCode, WPARAM wParam, LPARAM lParam) {
  if (m_application) {
    return m_application->HandleKeyboardHook(nCode, wParam, lParam);
  }

  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT KeyboardHookManager::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    return Get()->HandleEvent(nCode, wParam, lParam);
  }

  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
