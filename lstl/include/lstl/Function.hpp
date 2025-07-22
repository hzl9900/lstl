#pragma once
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <typeinfo>
template <typename Fn> struct Function {
    static_assert(!std::is_same_v<Fn, Fn>, "function signature not valid");
};

template <typename ReturnType, class... Args>
struct Function<ReturnType(Args...)> {
  private:
    struct FnBase {
        virtual ReturnType m_call(Args... args) = 0;
        virtual std::unique_ptr<FnBase> m_clone() const = 0;
        virtual std::type_info const& m_type() const = 0;
        virtual ~FnBase() = default;
    };
    template <class Fn> struct FnImpl : FnBase {
        Fn m_f;
        template <class... CArgs>
        explicit FnImpl(std::in_place_t, CArgs&&... args)
            : m_f(std::forward<CArgs>(args)...) {}
        ReturnType m_call(Args... args) override {
            return std::invoke(m_f, std::forward<Args>(args)...);
        }
        std::type_info const& m_type() const override { return typeid(Fn); }
    };
    std::unique_ptr<FnBase> m_base;

  public:
    Function() = default;
    Function(std::nullptr_t) noexcept : Function() {}
    Function(Function&&) = default;
    Function& operator=(Function&&) = default;
    Function(Function const& that)
        : m_base(that.m_base ? that.m_base->clone() : nullptr) {}
    Function& operator=(Function const& that) {
        if (that.m_base)
            m_base = that.m_base->m_clone();
        else
            m_base = nullptr;
    }
    explicit operator bool() const noexcept { return m_base != nullptr; }
    bool operator==(std::nullptr_t) const noexcept { return m_base == nullptr; }
    bool operator!=(std::nullptr_t) const noexcept { return m_base != nullptr; }
    ReturnType operator()(Args... args) const {
        if (!m_base) [[unlikely]]
            throw std::bad_function_call();
        return m_base->m_call(std::forward<Args>(args)...);
    }
    std::type_info const& target_type() const noexcept {
        return m_base ? m_base->m_type() : typeid(void);
    }
    template <class Fn> Fn* target() const noexcept {
        return m_base && typeid(Fn) == m_base->m_type()
                   ? std::addressof(static_cast<FnImpl<Fn>*>(m_base.get())->m_f)
                   : nullptr;
    }
    void swap(Function& that) const noexcept { m_base.swap(that.m_base); }
};