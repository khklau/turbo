#ifndef TURBO_TOOLSET_INTRINSIC_HPP
#define TURBO_TOOLSET_INTRINSIC_HPP

#include <cstdint>
#include <limits>

namespace turbo {
namespace toolset {

constexpr int uint32_digits()
{
    return std::numeric_limits<std::uint32_t>::digits;
}

constexpr int uint64_digits()
{
    return std::numeric_limits<std::uint64_t>::digits;
}

constexpr std::uint32_t pow_2_to_31()
{
    return 1U << 31;
}

inline std::uint32_t count_leading_zero(std::uint32_t input)
{
#if defined( _WIN32) && defined(_MSC_VER)
    std::uint32_t result = 0U;
    return (_BitScanReverse(result&, input) == 0) ? uint32_digits() : uint32_digits() - result + 1U;
#elif defined(__GNUC__) || defined(__clang__)
    return (input == 0U) ? uint32_digits() : __builtin_clz(input);
#else
    std::uint32_t count = 0U;
    while (count < uint32_digits() && (input & pow_2_to_31()) != pow_2_to_31())
    {
	input = input << 1;
	++count;
    }
    return count;
#endif
}

inline std::uint64_t count_leading_zero(std::uint64_t input)
{
#if defined( _WIN32) && defined(_MSC_VER)
    std::uint64_t result = 0U;
    return (_BitScanReverse64(result&, input) == 0) ? uint64_digits() : uint64_digits() - result + 1U;
#else
    std::uint32_t high_result = count_leading_zero(static_cast<std::uint32_t>((input & 0xFFFFFFFF00000000) >> uint32_digits()));
    return (high_result != uint32_digits()) ? high_result : count_leading_zero(static_cast<std::uint32_t>(input & 0x00000000FFFFFFFF)) + uint32_digits();
#endif
}

} // namespace toolset
} // namespace turbo

#endif

