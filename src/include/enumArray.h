#pragma once

#include <array>
#include <cstddef>
#include <utility>

template <typename T, typename Enum, std::size_t N>
class EnumArray {
    public:
    template <typename... Args>
        requires(sizeof...(Args) == N)

    constexpr EnumArray(Args&&... args) : data_{T{std::forward<Args>(args)}...} {}

    constexpr T& operator[](const std::size_t& i) noexcept { return data_[i]; }
    const constexpr T& operator[](const std::size_t& i) const noexcept { return data_[i]; }

    constexpr T& operator[](const Enum e) noexcept { return data_[std::to_underlying(e)]; }
    const constexpr T& operator[](const Enum e) const noexcept { return data_[std::to_underlying(e)]; }

    private:
    std::array<T, N> data_;
};
