#include "Engine.hpp"

#include "DxContext.hpp"
#include <ConsoleLib/ConsoleLib.hpp>

void Engine::Init() {
  //Console::Get()->RedirectStdHandles();
  DxContext::Get()->Initialize();
}

void Engine::Cleanup() {
  DxContext::Destroy();
}

void Engine::SetWorkingDirectory(std::wstring workingDirectory) {
  m_workingDirectory = std::move(workingDirectory);
}

const std::wstring& Engine::GetWorkingDirectory() {
  return m_workingDirectory;
}

std::wstring Engine::GetDirectory(const std::wstring& directory) {
  return m_workingDirectory + directory;
}
