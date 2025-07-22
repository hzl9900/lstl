#pragma once
#include <atomic>
#include <utility>
namespace lstl {
template <typename T> struct SpControlBlock {
    T* m_data;
    std::atomic<int> m_refcnt;

    explicit SpControlBlock(T* ptr) : m_data(ptr), m_refcnt(1) {}
    SpControlBlock(SpControlBlock&&) = delete;
    void incref() { m_refcnt.fetch_add(1); }
    void decref() {
        if (m_refcnt.fetch_sub(1) == 1)
            delete this;
    }
};
template <typename T> struct SharedPtr {
    SpControlBlock<T>* m_pcb;
    explicit SharedPtr(T* ptr) : m_pcb(ptr) {}
    SharedPtr(SharedPtr const& that) : m_pcb(that.m_pcb) {
        m_pcb->m_refcnt++;
    }

    ~SharedPtr() {
        // TODO
        if (m_pcb->m_refcnt.load() == 0) {
            delete m_pcb;
        }
    }

    T* get() const noexcept { return m_pcb->m_data; }
    T& operator*() const noexcept { return *m_pcb->m_data; }
    T* operator->() const noexcept { return m_pcb->m_data; }
};

template <typename T, class... Args> auto makeShared(Args... args) {
    return SharedPtr<T>(new T(std::forward<T>(args)...));
}
} // namespace lstl
