#pragma once

#include <Windows.h>
#include <optional>

struct LLMouseHookEvent final {
  LLMouseHookEvent(int nCode, WPARAM wParam, LPARAM lParam);

  void SetReturnValue(LRESULT result) const;
  [[nodiscard]] bool HasReturnValue() const;
  [[nodiscard]] LRESULT GetReturnValue() const;

public:
  int nCode;
  WPARAM wParam;
  LPARAM lParam;

private:
  mutable std::optional<LRESULT> m_result;
};
