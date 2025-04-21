#include "LLMouseHookEvent.hpp"

LLMouseHookEvent::LLMouseHookEvent(int nCode, WPARAM wParam, LPARAM lParam)
  : nCode(nCode)
  , wParam(wParam)
  , lParam(lParam) {

}

void LLMouseHookEvent::SetReturnValue(LRESULT result) const {
  m_result.emplace(result);
}

bool LLMouseHookEvent::HasReturnValue() const {
  return m_result.has_value();
}

LRESULT LLMouseHookEvent::GetReturnValue() const {
  return m_result.value();
}
