#pragma once
#include <cstddef>
#include <utility>
namespace lstl {
template <typename T> struct DefaultDeleter {
    constexpr DefaultDeleter() noexcept = default;
    template <typename U> DefaultDeleter(const DefaultDeleter<U>&) noexcept {}
    void operator()(T* p) { delete p; }
};
template <typename T> struct DefaultDeleter<T[]> {
    constexpr DefaultDeleter() noexcept = default;
    template <typename U> DefaultDeleter(const DefaultDeleter<U>&) noexcept {}
    void operator()(T* p) { delete[] p; }
};
template <typename T, typename Deleter = DefaultDeleter<T>>
class LUniquePtrData {
    T* p;
    Deleter d;
};

template <typename T, typename Deleter = DefaultDeleter<T>> class UniquePtr {
  public:
    // using declrations
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;

    // ctors
    UniquePtr() noexcept : m_p(nullptr) {}
    UniquePtr(std::nullptr_t) noexcept : m_p(nullptr) {}

    explicit UniquePtr(pointer p) noexcept : m_p(p) {}

    UniquePtr(UniquePtr&& that) noexcept {
        m_p = std::exchange(that.m_p, nullptr);
    }

    template <typename U>
    UniquePtr(UniquePtr<U> that) noexcept : m_p(that.m_p) {}

    UniquePtr(const UniquePtr&) = delete;

    pointer* release() { return std::exchange(m_p, nullptr); }

    void reset(pointer p) {
        if (m_p)
            Deleter{}(m_p);
        m_p = p;
    }

    pointer get() const { return m_p; }

    T& operator*() const { return *m_p; }

    pointer operator->() const { return m_p; }

  private:
    pointer m_p;

    template <typename U, class UDeleter> friend class UniquePtr;
};

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> : public UniquePtr<T, Deleter> {};

template <typename T> UniquePtr<T> makeUnique() {
    return UniquePtr<T>(new T());
}
template <typename T, typename... Args>
UniquePtr<T> makeUnique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}
template <typename T> UniquePtr<T> makeUniqueForOverwrite() {
    return UniquePtr<T>(new T);
}
} // namespace lstl
