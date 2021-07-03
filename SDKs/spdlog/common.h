// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/tweakme.h>
#include <spdlog/details/null_mutex.h>

#include <atomic>
#include <chrono>
#include <initializer_list>
#include <memory>
#include <exception>
#include <string>
#include <string_view>
#include <type_traits>
#include <functional>
#include <format>
#include <cassert>

#ifdef SPDLOG_COMPILED_LIB
#undef SPDLOG_HEADER_ONLY
#if defined(_WIN32) && defined(SPDLOG_SHARED_LIB)
#ifdef spdlog_EXPORTS
#define SPDLOG_API __declspec(dllexport)
#else
#define SPDLOG_API __declspec(dllimport)
#endif
#else // !defined(_WIN32) || !defined(SPDLOG_SHARED_LIB)
#define SPDLOG_API
#endif
#define SPDLOG_INLINE
#else // !defined(SPDLOG_COMPILED_LIB)
#define SPDLOG_API
#define SPDLOG_HEADER_ONLY
#define SPDLOG_INLINE inline
#endif // #ifdef SPDLOG_COMPILED_LIB

// visual studio upto 2013 does not support noexcept nor constexpr
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define SPDLOG_NOEXCEPT _NOEXCEPT
#define SPDLOG_CONSTEXPR
#else
#define SPDLOG_NOEXCEPT noexcept
#define SPDLOG_CONSTEXPR constexpr
#endif

#if defined(__GNUC__) || defined(__clang__)
#define SPDLOG_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define SPDLOG_DEPRECATED __declspec(deprecated)
#else
#define SPDLOG_DEPRECATED
#endif

// disable thread local on msvc 2013
#ifndef SPDLOG_NO_TLS
#if (defined(_MSC_VER) && (_MSC_VER < 1900)) || defined(__cplusplus_winrt)
#define SPDLOG_NO_TLS 1
#endif
#endif

#ifndef SPDLOG_FUNCTION
#define SPDLOG_FUNCTION static_cast<const char *>(__FUNCTION__)
#endif

#ifdef SPDLOG_NO_EXCEPTIONS
#define SPDLOG_TRY
#define SPDLOG_THROW(ex)                                                                                                                   \
    do                                                                                                                                     \
    {                                                                                                                                      \
        printf("spdlog fatal error: %s\n", ex.what());                                                                                     \
        std::abort();                                                                                                                      \
    } while (0)
#define SPDLOG_CATCH_ALL()
#else
#define SPDLOG_TRY try
#define SPDLOG_THROW(ex) throw(ex)
#define SPDLOG_CATCH_ALL() catch (...)
#endif

#ifdef _MSC_VER
#define SPDLOG_MSC_WARNING(...) __pragma(warning(__VA_ARGS__))
#else
#define SPDLOG_MSC_WARNING(...)
#endif

namespace spdlog {

class formatter;

namespace sinks {
class sink;
}

#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
using filename_t = std::wstring;
// allow macro expansion to occur in SPDLOG_FILENAME_T
#define SPDLOG_FILENAME_T_INNER(s) L##s
#define SPDLOG_FILENAME_T(s) SPDLOG_FILENAME_T_INNER(s)
#else
using filename_t = std::string;
#define SPDLOG_FILENAME_T(s) s
#endif

using log_clock = std::chrono::system_clock;
using sink_ptr = std::shared_ptr<sinks::sink>;
using sinks_init_list = std::initializer_list<sink_ptr>;
using err_handler = std::function<void(const std::string &err_msg)>;
using string_view_t = std::basic_string_view<char>;
using wstring_view_t = std::basic_string_view<wchar_t>;

namespace detail {
template<typename Int>
constexpr auto to_unsigned(Int value) -> typename std::make_unsigned<Int>::type
{
    return static_cast<typename std::make_unsigned<Int>::type>(value);
}

/**
  \rst
  A contiguous memory buffer with an optional growing ability. It is an internal
  class and shouldn't be used directly, only via `~fmt::basic_memory_buffer`.
  \endrst
 */
template<typename T>
class buffer
{
private:
    T *ptr_;
    size_t size_;
    size_t capacity_;

protected:
    // Don't initialize ptr_ since it is not accessed to save a few cycles.
    SPDLOG_MSC_WARNING(suppress : 26495)
    buffer(size_t sz) noexcept : size_(sz), capacity_(sz) {}

    buffer(T *p = nullptr, size_t sz = 0, size_t cap = 0) noexcept : ptr_(p), size_(sz), capacity_(cap) {}

    ~buffer() = default;
    buffer(buffer &&) = default;

    /** Sets the buffer data and capacity. */
    void set(T *buf_data, size_t buf_capacity) noexcept
    {
        ptr_ = buf_data;
        capacity_ = buf_capacity;
    }

    /** Increases the buffer capacity to hold at least *capacity* elements. */
    virtual void grow(size_t capacity) = 0;

public:
    using value_type = T;
    using const_reference = const T &;

    buffer(const buffer &) = delete;
    void operator=(const buffer &) = delete;

    auto begin() noexcept -> T *
    {
        return ptr_;
    }
    auto end() noexcept -> T *
    {
        return ptr_ + size_;
    }

    auto begin() const noexcept -> const T *
    {
        return ptr_;
    }
    auto end() const noexcept -> const T *
    {
        return ptr_ + size_;
    }

    /** Returns the size of this buffer. */
    auto size() const noexcept -> size_t
    {
        return size_;
    }

    /** Returns the capacity of this buffer. */
    auto capacity() const noexcept -> size_t
    {
        return capacity_;
    }

    /** Returns a pointer to the buffer data. */
    auto data() noexcept -> T *
    {
        return ptr_;
    }

    /** Returns a pointer to the buffer data. */
    auto data() const noexcept -> const T *
    {
        return ptr_;
    }

    /** Clears this buffer. */
    void clear()
    {
        size_ = 0;
    }

    // Tries resizing the buffer to contain *count* elements. If T is a POD type
    // the new elements may not be initialized.
    void try_resize(size_t count)
    {
        try_reserve(count);
        size_ = count <= capacity_ ? count : capacity_;
    }

    // Tries increasing the buffer capacity to *new_capacity*. It can increase the
    // capacity by a smaller amount than requested but guarantees there is space
    // for at least one additional element either by increasing the capacity or by
    // flushing the buffer if it is full.
    void try_reserve(size_t new_capacity)
    {
        if (new_capacity > capacity_)
            grow(new_capacity);
    }

    void push_back(const T &value)
    {
        try_reserve(size_ + 1);
        ptr_[size_++] = value;
    }

    /** Appends data to the end of the buffer. */
    template<typename U>
    void append(const U *begin, const U *end);

    template<typename I>
    auto operator[](I index) -> T &
    {
        return ptr_[index];
    }
    template<typename I>
    auto operator[](I index) const -> const T &
    {
        return ptr_[index];
    }
};

#if defined(_SECURE_SCL) && _SECURE_SCL
// Make a checked iterator to avoid MSVC warnings.
template<typename T>
using checked_ptr = stdext::checked_array_iterator<T *>;
template<typename T>
auto make_checked(T *p, size_t size) -> checked_ptr<T>
{
    return {p, size};
}
#else
template<typename T>
using checked_ptr = T *;
template<typename T>
inline auto make_checked(T *p, size_t) -> T *
{
    return p;
}
#endif

template<typename T>
template<typename U>
void buffer<T>::append(const U *begin, const U *end)
{
    while (begin != end)
    {
        auto count = to_unsigned(end - begin);
        try_reserve(size_ + count);
        auto free_cap = capacity_ - size_;
        if (free_cap < count)
            count = free_cap;
        std::uninitialized_copy_n(begin, count, make_checked(ptr_ + size_, count));
        size_ += count;
        begin += count;
    }
}


template<typename T>
constexpr auto count_digits_fallback(T n) -> int
{
    int count = 1;
    for (;;)
    {
        // Integer division is slow so do it for a group of four digits instead
        // of for every digit. The idea comes from the talk by Alexandrescu
        // "Three Optimization Tips for C++". See speed-test for a comparison.
        if (n < 10)
            return count;
        if (n < 100)
            return count + 1;
        if (n < 1000)
            return count + 2;
        if (n < 10000)
            return count + 3;
        n /= 10000u;
        count += 4;
    }
}

// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
constexpr inline auto count_digits(uint64_t n) -> int
{
#ifdef FMT_BUILTIN_CLZLL
    if (!is_constant_evaluated())
    {
        // https://github.com/fmtlib/format-benchmark/blob/master/digits10
        // Maps bsr(n) to ceil(log10(pow(2, bsr(n) + 1) - 1)).
        constexpr uint16_t bsr2log10[] = {1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10,
            10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 19, 20};
        auto t = bsr2log10[FMT_BUILTIN_CLZLL(n | 1) ^ 63];
        constexpr const uint64_t zero_or_powers_of_10[] = {
            0, 0, FMT_POWERS_OF_10(1U), FMT_POWERS_OF_10(1000000000ULL), 10000000000000000000ULL};
        return t - (n < zero_or_powers_of_10[t]);
    }
#endif
    return count_digits_fallback(n);
}

// Counts the number of digits in n. BITS = log2(radix).
template<int BITS, typename UInt>
constexpr auto count_digits(UInt n) -> int
{
#ifdef FMT_BUILTIN_CLZ
    if (num_bits<UInt>() == 32)
        return (FMT_BUILTIN_CLZ(static_cast<uint32_t>(n) | 1) ^ 31) / BITS + 1;
#endif
    int num_digits = 0;
    do
    {
        ++num_digits;
    } while ((n >>= BITS) != 0);
    return num_digits;
}

// It is a separate function rather than a part of count_digits to workaround
// the lack of static constexpr in constexpr functions.
inline uint64_t count_digits_inc(int n)
{
    // An optimization by Kendall Willets from https://bit.ly/3uOIQrB.
    // This increments the upper 32 bits (log10(T) - 1) when >= T is added.
#define FMT_INC(T) (((sizeof(#T) - 1ull) << 32) - T)
    static constexpr uint64_t table[] = {
        FMT_INC(0), FMT_INC(0), FMT_INC(0),                            // 8
        FMT_INC(10), FMT_INC(10), FMT_INC(10),                         // 64
        FMT_INC(100), FMT_INC(100), FMT_INC(100),                      // 512
        FMT_INC(1000), FMT_INC(1000), FMT_INC(1000),                   // 4096
        FMT_INC(10000), FMT_INC(10000), FMT_INC(10000),                // 32k
        FMT_INC(100000), FMT_INC(100000), FMT_INC(100000),             // 256k
        FMT_INC(1000000), FMT_INC(1000000), FMT_INC(1000000),          // 2048k
        FMT_INC(10000000), FMT_INC(10000000), FMT_INC(10000000),       // 16M
        FMT_INC(100000000), FMT_INC(100000000), FMT_INC(100000000),    // 128M
        FMT_INC(1000000000), FMT_INC(1000000000), FMT_INC(1000000000), // 1024M
        FMT_INC(1000000000), FMT_INC(1000000000)                       // 4B
    };
    return table[n];
}

// Optional version of count_digits for better performance on 32-bit platforms.
constexpr inline auto count_digits(uint32_t n) -> int
{
#ifdef FMT_BUILTIN_CLZ
    if (!is_constant_evaluated())
    {
        auto inc = count_digits_inc(FMT_BUILTIN_CLZ(n | 1) ^ 31);
        return static_cast<int>((n + inc) >> 32);
    }
#endif
    return count_digits_fallback(n);
}
}

template<typename T, size_t SIZE = 500, typename Allocator = std::allocator<T>>
class basic_memory_buffer final : public detail::buffer<T>
{
private:
    T store_[SIZE];

    // Don't inherit from Allocator avoid generating type_info for it.
    Allocator alloc_;

    // Deallocate memory allocated by the buffer.
    void deallocate()
    {
        T *data = this->data();
        if (data != store_)
            alloc_.deallocate(data, this->capacity());
    }

protected:
    void grow(size_t size) final override;

public:
    using value_type = T;
    using const_reference = const T &;

    explicit basic_memory_buffer(const Allocator &alloc = Allocator())
        : alloc_(alloc)
    {
        this->set(store_, SIZE);
    }
    ~basic_memory_buffer()
    {
        deallocate();
    }

private:
    // Move data from other to this buffer.
    void move(basic_memory_buffer &other)
    {
        alloc_ = std::move(other.alloc_);
        T *data = other.data();
        size_t size = other.size(), capacity = other.capacity();
        if (data == other.store_)
        {
            this->set(store_, capacity);
            std::uninitialized_copy(other.store_, other.store_ + size, detail::make_checked(store_, capacity));
        }
        else
        {
            this->set(data, capacity);
            // Set pointer to the inline array so that delete is not called
            // when deallocating.
            other.set(other.store_, 0);
        }
        this->resize(size);
    }

public:
    /**
      \rst
      Constructs a :class:`fmt::basic_memory_buffer` object moving the content
      of the other object to it.
      \endrst
     */
    basic_memory_buffer(basic_memory_buffer &&other) noexcept
    {
        move(other);
    }

    /**
      \rst
      Moves the content of the other ``basic_memory_buffer`` object to this one.
      \endrst
     */
    auto operator=(basic_memory_buffer &&other) noexcept -> basic_memory_buffer &
    {
        assert(this != &other);
        deallocate();
        move(other);
        return *this;
    }

    // Returns a copy of the allocator associated with this buffer.
    auto get_allocator() const -> Allocator
    {
        return alloc_;
    }

    /**
      Resizes the buffer to contain *count* elements. If T is a POD type new
      elements may not be initialized.
     */
    void resize(size_t count)
    {
        this->try_resize(count);
    }

    /** Increases the buffer capacity to *new_capacity*. */
    void reserve(size_t new_capacity)
    {
        this->try_reserve(new_capacity);
    }

    // Directly append data into the buffer
    using detail::buffer<T>::append;
    template<typename ContiguousRange>
    void append(const ContiguousRange &range)
    {
        append(range.data(), range.data() + range.size());
    }
};

template<typename T, size_t SIZE, typename Allocator>
void basic_memory_buffer<T, SIZE, Allocator>::grow(size_t size)
{
    const size_t max_size = std::allocator_traits<Allocator>::max_size(alloc_);
    size_t old_capacity = this->capacity();
    size_t new_capacity = old_capacity + old_capacity / 2;
    if (size > new_capacity)
        new_capacity = size;
    else if (new_capacity > max_size)
        new_capacity = size > max_size ? size : max_size;
    T *old_data = this->data();
    T *new_data = std::allocator_traits<Allocator>::allocate(alloc_, new_capacity);
    // The following code doesn't throw, so the raw pointer above doesn't leak.
    std::uninitialized_copy(old_data, old_data + this->size(), detail::make_checked(new_data, new_capacity));
    this->set(new_data, new_capacity);
    // deallocate must not throw according to the standard, but even if it does,
    // the buffer already uses the new storage and will deallocate it in
    // destructor.
    if (old_data != store_)
        alloc_.deallocate(old_data, old_capacity);
}

using memory_buf_t = basic_memory_buffer<char, 250>;
using wmemory_buf_t = basic_memory_buffer<wchar_t, 250>;

template <typename Char, size_t SIZE>
auto to_string(const basic_memory_buffer<Char, SIZE>& buf)
-> std::basic_string<Char> {
    auto size = buf.size();
    return std::basic_string<Char>(buf.data(), size);
}

#ifdef SPDLOG_WCHAR_TO_UTF8_SUPPORT
#ifndef _WIN32
#error SPDLOG_WCHAR_TO_UTF8_SUPPORT only supported on windows
#else
template<typename T>
struct is_convertible_to_wstring_view : std::is_convertible<T, wstring_view_t>
{};
#endif // _WIN32
#else
template<typename>
struct is_convertible_to_wstring_view : std::false_type
{};
#endif // SPDLOG_WCHAR_TO_UTF8_SUPPORT

#if defined(SPDLOG_NO_ATOMIC_LEVELS)
using level_t = details::null_atomic_int;
#else
using level_t = std::atomic<int>;
#endif

#define SPDLOG_LEVEL_TRACE 0
#define SPDLOG_LEVEL_DEBUG 1
#define SPDLOG_LEVEL_INFO 2
#define SPDLOG_LEVEL_WARN 3
#define SPDLOG_LEVEL_ERROR 4
#define SPDLOG_LEVEL_CRITICAL 5
#define SPDLOG_LEVEL_OFF 6

#if !defined(SPDLOG_ACTIVE_LEVEL)
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif

// Log level enum
namespace level {
enum level_enum
{
    trace = SPDLOG_LEVEL_TRACE,
    debug = SPDLOG_LEVEL_DEBUG,
    info = SPDLOG_LEVEL_INFO,
    warn = SPDLOG_LEVEL_WARN,
    err = SPDLOG_LEVEL_ERROR,
    critical = SPDLOG_LEVEL_CRITICAL,
    off = SPDLOG_LEVEL_OFF,
    n_levels
};

#if !defined(SPDLOG_LEVEL_NAMES)
#define SPDLOG_LEVEL_NAMES                                                                                                                 \
    {                                                                                                                                      \
        "trace", "debug", "info", "warning", "error", "critical", "off"                                                                    \
    }
#endif

#if !defined(SPDLOG_SHORT_LEVEL_NAMES)

#define SPDLOG_SHORT_LEVEL_NAMES                                                                                                           \
    {                                                                                                                                      \
        "T", "D", "I", "W", "E", "C", "O"                                                                                                  \
    }
#endif

SPDLOG_API const string_view_t &to_string_view(spdlog::level::level_enum l) SPDLOG_NOEXCEPT;
SPDLOG_API void set_string_view(spdlog::level::level_enum l, const string_view_t &s) SPDLOG_NOEXCEPT;
SPDLOG_API const char *to_short_c_str(spdlog::level::level_enum l) SPDLOG_NOEXCEPT;
SPDLOG_API spdlog::level::level_enum from_str(const std::string &name) SPDLOG_NOEXCEPT;

} // namespace level

//
// Color mode used by sinks with color support.
//
enum class color_mode
{
    always,
    automatic,
    never
};

//
// Pattern time - specific time getting to use for pattern_formatter.
// local time by default
//
enum class pattern_time_type
{
    local, // log localtime
    utc    // log utc
};

//
// Log exception
//
class SPDLOG_API spdlog_ex : public std::exception
{
public:
    explicit spdlog_ex(std::string msg);
    spdlog_ex(const std::string &msg, int last_errno);
    const char *what() const SPDLOG_NOEXCEPT override;

private:
    std::string msg_;
};

[[noreturn]] SPDLOG_API void throw_spdlog_ex(const std::string &msg, int last_errno);
[[noreturn]] SPDLOG_API void throw_spdlog_ex(std::string msg);

struct source_loc
{
    SPDLOG_CONSTEXPR source_loc() = default;
    SPDLOG_CONSTEXPR source_loc(const char *filename_in, int line_in, const char *funcname_in)
        : filename{filename_in}
        , line{line_in}
        , funcname{funcname_in}
    {}

    SPDLOG_CONSTEXPR bool empty() const SPDLOG_NOEXCEPT
    {
        return line == 0;
    }
    const char *filename{nullptr};
    int line{0};
    const char *funcname{nullptr};
};

namespace details {
// make_unique support for pre c++14

#if __cplusplus >= 201402L // C++14 and beyond
using std::make_unique;
#else
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&...args)
{
    static_assert(!std::is_array<T>::value, "arrays not supported");
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif
} // namespace details
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "common-inl.h"
#endif
