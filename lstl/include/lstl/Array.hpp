#pragma once
#include <cstddef>
#include <iterator>
#include <limits>
#include <stdexcept>
namespace lstl {
template <typename T, std::size_t N> class Array {
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  private:
    T m_data[N];

  public:
    reference at(size_t pos) {
        if (pos < 0 || pos >= N) [[unlikely]]
            throw std::out_of_range("out of range");
        return m_data[pos];
    }
    const_reference at(size_t pos) const {
        if (pos < 0 || pos >= N) [[unlikely]]
            throw std::out_of_range("out of range");
        return m_data[pos];
    }

    reference operator[](size_type i) { return m_data[i]; }
    const_reference operator[](size_type i) const { return m_data[i]; }

    reference front() { return m_data[0]; }

    const_reference front() const { return m_data[0]; }

    reference back() { return m_data[N - 1]; }

    const_reference back() const { return m_data[N - 1]; }

    // 注意当*this为空时, data()不一定返回空指针
    T* data() noexcept { return m_data; }

    const T* data() const noexcept { return m_data; }

    iterator begin() noexcept { return m_data; }

    const_iterator begin() const noexcept { return m_data; }

    const_iterator cbegin() const noexcept { return m_data; }

    iterator end() noexcept { return m_data + N; }

    const_iterator end() const noexcept { return m_data + N; }

    const_iterator cend() const noexcept { return m_data + N; }

    reverse_iterator rbegin() noexcept {
        return std::make_reverse_iterator(m_data);
    }

    const_reverse_iterator rbegin() const noexcept {
        return std::make_reverse_iterator(m_data);
    }

    const_reverse_iterator crbegin() const noexcept {
        return std::make_reverse_iterator(m_data);
    }

    reverse_iterator rend() noexcept {
        return std::make_reverse_iterator(m_data + N);
    }

    const_reverse_iterator rend() const noexcept {
        return std::make_reverse_iterator(m_data + N);
    }

    const_reverse_iterator crend() const noexcept {
        return std::make_reverse_iterator(m_data + N);
    }

    constexpr bool empty() const noexcept { return N == 0; }

    constexpr size_t size() const noexcept { return N; }

    static constexpr std::size_t max_size() noexcept {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    void fill(const T& value) {
        for (size_t i = 0; i < N; i++) {
            this->m_data[i] = value;
        }
    }

    void swap(Array& that) { std::swap(m_data, that.m_data); }
};

template <typename T, std::size_t N>
constexpr lstl::Array<std::remove_cv_t<T>, N> toArray(T (&)[N]) {}

template <typename T, std::size_t N>
constexpr lstl::Array<std::remove_cv_t<T>, N> toArray(T (&&)[N]) {}
} // namespace lstl
