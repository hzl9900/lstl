#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace lstl {
template <typename T, typename Alloc = std::allocator<T>> class Vector {
  public:
    // 成员类型
    // 抄 cppreference
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    // https://www.zhihu.com/question/643546853
    // pointer别名应该没用
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T const&;
    using iterator = T*;
    using const_iterator = T const*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  private:
    T* m_data;
    std::size_t m_size;
    std::size_t m_capacity;
    [[no_unique_address]] Alloc m_alloc;

  public:
    // begin ctor
    // (1)
    Vector() noexcept(noexcept(Alloc())) : Vector(Alloc()) {}

    // (2)
    explicit Vector(const Alloc& alloc = Alloc())
        : m_data(nullptr), m_size(0), m_capacity(0), m_alloc(alloc) {}

    // (3)
    explicit Vector(std::size_t n, Alloc const& alloc = Alloc())
        : m_alloc(alloc) {
        m_data = m_alloc.allocate(n);
        m_capacity = n;
        m_size = n;
        for (std::size_t i = 0; i <= n; i++) {
            std::construct_at(&m_data[i]);
            // 对placement new 的包装
            // 调用::new 并转发Args
            // 特殊处理T[]
        }
    }

    // (4)
    Vector(std::size_t n, T const& value, Alloc const& alloc = Alloc())
        : m_alloc(alloc) {
        m_data = m_alloc.allocate(n);
        m_capacity = n;
        m_size = n;
#ifndef NDEBUG
        std::cout << m_data << "\n";
#endif
        for (std::size_t i = 0; i != n; i++) {
            std::construct_at(&m_data[i], value);
        }
    }

    // (5)
    template <
        typename InputIterator
        // ,
        //           std::enable_if_t<std::is_convertible_v<
        //               typename
        //               std::iterator_traits<InputIterator>::iterator_category,
        //               std::random_access_iterator_tag>>
        >
    Vector(InputIterator first, InputIterator last,
            Alloc const& alloc = Alloc())
        : m_alloc(alloc) {
        std::size_t n = last - first;
        m_data = m_alloc.allocate(n);
        m_capacity = m_size = n;
        for (std::size_t i = 0; i < n; i++) {
            std::construct_at(&m_data[i], *first);
            ++first;
        }
    }

    // (6)
    // TODO: range

    // (7)
    Vector(Vector const& that) : m_alloc(that.m_alloc) {
        m_capacity = that.m_size;
        m_size = that.m_size;
        if (m_size) {
            m_data = m_alloc.allocate(m_size);
            for (std::size_t i = 0; i != m_size; i++) {
                std::construct_at(&m_data[i],
                                  std::move_if_noexcept(that.m_data[i]));
            }
        } else {
            m_data = nullptr;
        }
    }

    // (8)
    Vector(Vector&& that) noexcept : m_alloc(std::move(that.m_alloc)) {
        m_data = std::exchange(that.m_data, nullptr);
        m_size = std::exchange(that.m_size, 0);
        m_capacity = std::exchange(that.m_capacity, 0);
    }

    //(9)
    Vector(Vector const& that, Alloc const& alloc) : m_alloc(alloc) {
        m_capacity = that.m_size;
        m_size = that.m_size;
        if (m_size) {
            m_data = m_alloc.allocate(m_size);
            for (std::size_t i = 0; i != m_size; i++) {
                std::construct_at(&m_data[i],
                                  std::move_if_noexcept(that.m_data[i]));
            }
        } else {
            m_data = nullptr;
        }
    }

    //(10)
    Vector(Vector&& that, Alloc const& alloc) noexcept : m_alloc(alloc) {
        m_data = std::exchange(that.m_data, nullptr);
        m_size = std::exchange(that.m_size, 0);
        m_capacity = std::exchange(that.m_capacity, 0);
    }

    // (11)
    Vector(std::initializer_list<T> lst, Alloc const& alloc = Alloc())
        : Vector(lst.begin(), lst.end(), alloc) {}

    // dtor
    // TODO
    // 要求线性复杂度
    ~Vector() noexcept {
        for (std::size_t i = 0; i != m_size; i++) {
            std::destroy_at(&m_data[i]);
        }
        if (m_capacity != 0) {
            m_alloc.deallocate(m_data, m_capacity);
        }
    }

    // operator=

    Vector& operator=(Vector const& that) {
        if (&that == this) [[unlikely]]
            return *this;
        reserve(that.m_size);
        m_size = that.m_size;
        for (std::size_t i = 0; i != m_size; i++) {
            std::construct_at(&m_data[i], std::as_const(that.m_data[i]));
        }
        return *this;
    }

    Vector& operator=(Vector&& that) noexcept(
        std::allocator_traits<
            Alloc>::propagate_on_container_move_assignment::value ||
        std::allocator_traits<Alloc>::is_always_equal::value) {
        if (&that == this) [[unlikely]]
            return *this;
        for (std::size_t i = 0; i != m_size; i++) {
            std::destroy_at(&m_data[i]);
        }
        if (m_capacity) {
            m_alloc.deallocate(m_data, m_capacity);
        }
        m_data = std::exchange(that.m_data, nullptr);
        m_size = std::exchange(that.m_size, 0);
        m_capacity = std::exchange(that.m_capacity, 0);
        return *this;
    }

    Vector& operator=(std::initializer_list<value_type> lst) {
        assign(lst.begin(), lst.end());
        return *this;
    }

    // assign

    // (1)
    void assign(size_type n, T const& value) {
        clear();
        reserve(n);
        m_size = n;
        for (std::size_t i = 0; i != m_size; i++) {
            std::construct_at(&m_data[i], value);
        }
    }

    // (2)
    template <
        typename InputIterator,
        std::enable_if_t<std::is_convertible_v<
            typename std::iterator_traits<InputIterator>::iterator_category,
            std::random_access_iterator_tag>>>
    void assign(InputIterator first, InputIterator last) {
        clear();
        std::size_t n = last - first;
        reserve(n);
        m_size = n;
        for (std::size_t i = 0; i != m_size; i++) {
            std::construct_at(&m_data[i], *first);
            first++;
        }
    }

    // (3)
    void assign(std::initializer_list<T> lst) {
        assign(lst.begin(), lst.end());
    }

    // end assign
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // begin assign_range
    // TODO: 未实现
    // end assign_range
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // begin get_allocator

    allocator_type get_Allocator() const noexcept { return m_alloc; }

    // end get_allocator

    ///////////////////////////////////////////////////////////////////
    // begin at
    // 越界抛出 std::out_of_range

    reference at(size_type i) {
        if (i >= m_size) [[unlikely]]
            throw std::out_of_range("vector::at out_of_range");
    }

    const_reference& at(size_type i) const {
        if (i >= m_size) [[unlikely]]
            throw std::out_of_range("vector::at out_of_range");
    }

    // end at
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // begin operator[]
    // C++26前, 越界是UB
    // C++26后, 加固后的标准库默认自带边界检查
    const_reference operator[](std::size_t i) const noexcept {
        return m_data[i];
    }

    reference operator[](std::size_t i) noexcept { return m_data[i]; }

    // end operator[]
    ///////////////////////////////////////////////////////////////////

    // front & back

    const_reference front() const noexcept { return *m_data; }
    reference front() noexcept { return *m_data; }
    const_reference back() const noexcept { return m_data[m_size - 1]; }
    reference back() noexcept { return m_data[m_size - 1]; }

    // data()
    // 注意*this为空时, data()不一定返回空指针
    T* data() noexcept { return m_data; }
    T const* data() const noexcept { return m_data; }
    T* cdata() const noexcept { return m_data; }

    // begin & end
    //*this 为空时, begin() == end()
    iterator begin() noexcept { return m_data; }
    const_iterator begin() const noexcept { return m_data; }
    const_iterator cbegin() const noexcept { return m_data; }

    iterator end() noexcept { return m_data + m_size; }
    const_iterator end() const noexcept { return m_data + m_size; }
    const_iterator cend() const noexcept { return m_data + m_size; }

    // rbegin & rend

    reverse_iterator rbegin() noexcept {
        return std::make_reverse_iterator(m_data + m_size);
    }
    const_reverse_iterator rbegin() const noexcept {
        return std::make_reverse_iterator(m_data + m_size);
    }
    const_reverse_iterator crbegin() noexcept {
        return std::make_reverse_iterator(m_data + m_size);
    }

    reverse_iterator rend() noexcept {
        return std::make_reverse_iterator(m_data);
    }
    const_reverse_iterator rend() const noexcept {
        return std::make_reverse_iterator(m_data);
    }
    const_reverse_iterator crend() noexcept {
        return std::make_reverse_iterator(m_data);
    }

    // about size

    bool empty() const noexcept { return m_size == 0; }

    size_type size() const noexcept { return m_size; }

    // TODO: 更细的判断
    static constexpr std::size_t max_size() noexcept {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    void reserve(size_type n) {
        if (n < m_capacity)
            return;
        n = std::max(n, m_capacity * 2);
        auto old_data = m_data;
        auto old_capacity = m_capacity;
        if (n == 0) {
            m_data = nullptr;
            m_capacity = 0;
        } else {
            m_data = m_alloc.allocate(n);
            m_capacity = n;
        }
        if (old_capacity) {
            for (std::size_t i = 0; i != m_size; i++) {
                std::construct_at(&m_data[i],
                                  std::move_if_noexcept(old_data[i]));
            }
            for (std::size_t i = 0; i != m_size; i++) {
                std::destroy_at(&old_data[i]);
            }
            m_alloc.deallocate(old_data, old_capacity);
        }
    }

    size_type capacity() const noexcept { return m_capacity; }

    void shrink_to_fit() noexcept {
        auto old_data = m_data;
        auto old_capacity = m_capacity;
        m_capacity = m_size;
        if (m_size == 0) {
            m_data = nullptr;
        } else {
            m_data = m_alloc.allocate(m_size);
        }
        if (old_capacity != 0) [[likely]] {
            // ?
            for (std::size_t i = 0; i != old_capacity; i++) {
                std::construct_at(&m_data[i],
                                  std::move_if_noexcept(old_data[i]));
                // 强异常安全
                // 只有T 移动构造noexcept 和禁止复制构造时, 返回T&&
                // 否则返回const T&
                std::destroy_at(&old_data[i]);
            }
            m_alloc.deallocate(old_data, old_capacity);
        }
    }

    // modifier

    void clear() noexcept {
        for (std::size_t i = 0; i != m_size; i++) {
            std::destroy_at(&m_data[i]);
            // 调用p的析构函数: p->~T()
        }
        m_size = 0;
    }

    iterator insert(const_iterator it, T const& value) {
        std::size_t j = it - m_data;
        reserve(m_size + 1);
        for (std::size_t i = m_size; i != j; i--) {
            std::construct_at(&m_data[i], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size++;
        std::construct_at(m_data[j], value);
        return m_data + j;
    }
    iterator insert(const_iterator it, T&& value) {
        std::size_t j = it - m_data;
        reserve(m_size + 1);
        for (std::size_t i = m_size; i != j; i--) {
            std::construct_at(&m_data[i], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size++;
        std::construct_at(m_data[j], std::move(value));
        return m_data + j;
    }
    iterator insert(const_iterator it, std::size_t n, T const& value) {
        std::size_t j = it - m_data;
        if (n == 0) [[unlikely]]
            return const_cast<T*>(it);
        reserve(m_size + n);
        for (std::size_t i = m_size; i != j; i--) {
            std::construct_at(&m_data[i], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size += n;
        for (std::size_t i = j; i != j + n; i++) {
            std::construct_at(m_data[j], value);
        }
        return m_data + j;
    }
    template <
        typename InputIterator,
        std::enable_if_t<std::is_convertible_v<
            typename std::iterator_traits<InputIterator>::iterator_category,
            std::random_access_iterator_tag>>>
    iterator insert(const_iterator it, InputIterator first,
                    InputIterator last) {
        std::size_t j = it - m_data;
        std::size_t n = last - first;
        if (n == 0) [[unlikely]]
            return const_cast<T*>(it);
        reserve(m_size + n);
        for (std::size_t i = m_size; i != j; i--) {
            std::construct_at(&m_data[i + n - 1], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size += n;
        for (std::size_t i = j; i != j + n; ++i) {
            std::construct_at(m_data[j], *first);
            ++first;
        }
        return m_data + j;
    }
    iterator insert(const_iterator it, std::initializer_list<T> lst) {
        return insert(it, lst.begin(), lst.end());
    }

    template <typename... Args>
    iterator emplace(const_iterator it, Args&&... args) {
        std::size_t j = it - m_data;
        reserve(m_size + 1);
        for (std::size_t i = m_size; i != j; i--) {
            std::construct_at(&m_data[i], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size++;
        std::construct_at(m_data[j], std::forward<Args>(args)...);
        return m_data + j;
    }

    T* erase(const_iterator it) noexcept(std::is_nothrow_move_assignable_v<T>) {
        std::size_t i = it - m_data;
        for (std::size_t j = i + 1; j != m_size; j++) {
            m_data[j - 1] = std::move(m_data[j]);
        }
        m_size--;
        std::destroy_at(&m_data[m_size]);
        return const_cast<T*>(it);
    }

    T*
    erase(const_iterator first,
          const_iterator last) noexcept(std::is_nothrow_move_assignable_v<T>) {
        std::size_t diff = last - first;
        for (std::size_t j = last - m_data; j != m_size; j++) {
            m_data[j - diff] = std::move(m_data[j]);
        }
        m_size -= diff;
        for (std::size_t j = m_size; j != m_size + diff; j++) {
            std::destroy_at(&m_data[m_size]);
        }
        return const_cast<T*>(first);
    }

    void push_back(T const& value) {
        if (m_size + 1 >= m_capacity) [[unlikely]]
            reserve(m_size + 1);
        std::construct_at(&m_data[m_size], value);
        m_size++;
    }

    void push_back(T&& value) {
        if (m_size + 1 >= m_capacity) [[unlikely]]
            reserve(m_size + 1);
        std::construct_at(&m_data[m_size], std::move(value));
        m_size++;
    }

    template <class... Args> reference emplace_back(Args&&... args) {
        if (m_size + 1 >= m_capacity) [[unlikely]]
            reserve(m_size + 1);
        T* p = &m_data[m_size];
        std::construct_at(p, std::forward<Args>(args)...);
        m_size++;
        return *p;
    }

    void pop_back() noexcept {
        m_size--;
        std::destroy_at(&m_data[m_size]);
    }

    void resize(size_type n) {
        if (n > max_size())
            throw std::length_error("too long");
        if (n < m_size) {
            for (std::size_t i = n; i != m_size; i++) {
                std::destroy_at(&m_data[i]);
            }
            m_size = n;
        } else if (n > m_size) {
            reserve(n);
            for (std::size_t i = m_size; i != n; i++) {
                std::construct_at(&m_data[i]);
            }
        }
        m_size = n;
    }

    void resize(size_type n, T const& value) {
        if (n > max_size())
            throw std::length_error("too long");
        if (n < m_size) {
            for (std::size_t i = n; i != m_size; i++) {
                std::destroy_at(&m_data[i]);
            }
            m_size = n;
        } else if (n > m_size) {
            reserve(n);
            for (std::size_t i = m_size; i != n; i++) {
                std::construct_at(&m_data[i], value);
            }
        }
        m_size = n;
    }

    void swap(Vector& that) noexcept(
        std::allocator_traits<
            allocator_type>::propagate_on_container_swap::value ||
        std::allocator_traits<allocator_type>::is_always_equal::value) {
        std::swap(m_data, that.m_data);
        std::swap(m_size, that.m_size);
        std::swap(m_capacity, that.m_capacity);
        std::swap(m_alloc, that.m_alloc);
    }

    // comparison

    bool operator==(T const& that) const noexcept {
        return std::equal(this->begin(), this->end(), that.begin(), that.end());
    }
    bool operator!=(T const& that) const noexcept { return !(*this == that); }
    bool operator<(T const& that) const noexcept {
        return std::lexicographical_compare(this->begin(), this->end(),
                                            that.begin(), that.end());
    }
    bool operator>(T const& that) const noexcept { return that < *this; }
    bool operator<=(T const& that) const noexcept { return !(that < *this); }
    bool operator>=(T const& that) const noexcept { return !(that > *this); }
};
} // namespace lstl
