#pragma once

#include <list>

struct CallbackInfo final {
  explicit CallbackInfo(bool isCancellable)
  : m_isCancellable(isCancellable)
  , m_isCancelled(false) {
  }

  [[nodiscard]] bool IsCancellable() const {
    return m_isCancellable;
  }

  [[nodiscard]] bool IsCancelled() const {
    return m_isCancelled;
  }

  void Cancel() {
    if (m_isCancellable) {
      m_isCancelled = true;
    }
  }

private:
  bool m_isCancellable;
  bool m_isCancelled;
};

template <typename E>
struct EventListener {
  virtual ~EventListener() = default;

  virtual void HandleEvent(const E& e, CallbackInfo& info) = 0;
};

template <typename E>
struct EventDispatcher final {
  using CbType = EventListener<E>*;
  using CbId = typename std::list<CbType>::iterator;

  CbId AddListener(CbType callback) {
    return m_listeners.emplace(m_listeners.end(), callback);
  }

  void RemoveListener(CbId id) {
    m_listeners.erase(id);
  }

  void RemoveAllListeners() {
    m_listeners.clear();
  }

  void Dispatch(const E& e, bool isCancellable = false) {
    CallbackInfo info(isCancellable);
    for (const auto& listener : m_listeners) {
      listener->HandleEvent(e, info);
      if (info.IsCancelled()) {
        break;
      }
    }
  }

private:
  std::list<CbType> m_listeners;
};