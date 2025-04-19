#pragma once

#include <string>

struct Engine final {
  static void Init();
  static void Cleanup();

  static void SetWorkingDirectory(std::wstring workingDirectory);
  static const std::wstring& GetWorkingDirectory();
  static std::wstring GetDirectory(const std::wstring& directory);

private:
  inline static std::wstring m_workingDirectory;
};
