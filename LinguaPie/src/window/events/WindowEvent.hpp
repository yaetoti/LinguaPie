#pragma once

enum class WindowEventType {
  RESIZE,
  KEY,
  MOUSE_BUTTON,
  MOUSE_MOVE,
  MOUSE_SCROLL,
  COUNT
};

struct WindowEvent;

struct WindowEvent {
  virtual ~WindowEvent() = default;

  WindowEventType type;

  template <typename T>
  T* As() requires(std::derived_from<T, WindowEvent>) {
    return reinterpret_cast<T*>(this);
  }

  template <typename T>
  const T* As() const requires(std::derived_from<T, WindowEvent>) {
    return reinterpret_cast<const T*>(this);
  }

protected:
  explicit WindowEvent(WindowEventType type)
  : type(type) {
  }
};
