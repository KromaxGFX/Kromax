#ifndef KROMAX_SRC_CORE_EXPECTED_H
#define KROMAX_SRC_CORE_EXPECTED_H

/*
 * Expected.h
 *
 * C++20 std::expected backport for the internal implementation.
 * No exceptions, no RTTI. Errors are KxResult values.
 */

#include <utility>
#include <type_traits>

#include "kx_core.h"

namespace kx {

template <typename T>
class Expected {
public:
    using value_type = T;

    constexpr Expected() noexcept
        : has_value_(true), value_() {}

    constexpr Expected(const T& v) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : has_value_(true), value_(v) {}

    constexpr Expected(T&& v) noexcept(std::is_nothrow_move_constructible_v<T>)
        : has_value_(true), value_(std::move(v)) {}

    constexpr Expected(KxResult err) noexcept
        : has_value_(false), error_(err) {}

    Expected(const Expected& other) noexcept(
        std::is_nothrow_copy_constructible_v<T>)
        : has_value_(other.has_value_) {
        if (has_value_) {
            new (&value_) T(other.value_);
        } else {
            error_ = other.error_;
        }
    }

    Expected(Expected&& other) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : has_value_(other.has_value_) {
        if (has_value_) {
            new (&value_) T(std::move(other.value_));
        } else {
            error_ = other.error_;
        }
    }

    ~Expected() noexcept {
        reset();
    }

    Expected& operator=(const Expected& other) noexcept(
        std::is_nothrow_copy_constructible_v<T>) {
        if (this != &other) {
            reset();
            has_value_ = other.has_value_;
            if (has_value_) {
                new (&value_) T(other.value_);
            } else {
                error_ = other.error_;
            }
        }
        return *this;
    }

    Expected& operator=(Expected&& other) noexcept(
        std::is_nothrow_move_constructible_v<T>) {
        if (this != &other) {
            reset();
            has_value_ = other.has_value_;
            if (has_value_) {
                new (&value_) T(std::move(other.value_));
            } else {
                error_ = other.error_;
            }
        }
        return *this;
    }

    Expected& operator=(const T& v) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        reset();
        has_value_ = true;
        new (&value_) T(v);
        return *this;
    }

    Expected& operator=(T&& v) noexcept(std::is_nothrow_move_assignable_v<T>) {
        reset();
        has_value_ = true;
        new (&value_) T(std::move(v));
        return *this;
    }

    Expected& operator=(KxResult err) noexcept {
        reset();
        has_value_ = false;
        error_ = err;
        return *this;
    }

    constexpr bool has_value() const noexcept { return has_value_; }
    constexpr explicit operator bool() const noexcept { return has_value_; }

    constexpr T& value() & noexcept { return value_; }
    constexpr const T& value() const& noexcept { return value_; }
    constexpr T&& value() && noexcept { return std::move(value_); }
    constexpr const T&& value() const&& noexcept { return std::move(value_); }

    constexpr KxResult error() const noexcept { return error_; }

    template <typename U>
    constexpr T value_or(U&& default_value) const& noexcept(
        std::is_nothrow_copy_constructible_v<T> &&
        std::is_nothrow_constructible_v<U, T>) {
        return has_value_ ? value_ : T(std::forward<U>(default_value));
    }

    template <typename U>
    T value_or(U&& default_value) && noexcept(
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_constructible_v<U, T>) {
        return has_value_ ? std::move(value_) : T(std::forward<U>(default_value));
    }

private:
    void reset() noexcept {
        if (has_value_) {
            value_.~T();
            has_value_ = false;
        }
    }

    union {
        T value_;
    };
    bool has_value_;
    KxResult error_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_EXPECTED_H */