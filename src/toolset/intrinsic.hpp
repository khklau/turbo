#ifndef TURBO_TOOLSET_INTRINSIC_HPP
#define TURBO_TOOLSET_INTRINSIC_HPP

#include <cstdint>

namespace turbo {
namespace toolset {

inline std::uint32_t count_leading_zero(std::uint32_t input)
{
#if defined( _WIN32) && defined(_MSC_VER)
    std::uint32_t result = 0U;
    return (_BitScanReverse(result&, input) == 0) ? 32U : 32U - result + 1U;
#elif defined(__GNUC__) || defined(__clang__)
    return (input == 0U) ? 32U : __builtin_clz(input);
#else
    std::uint32_t count = 0U;
    while (count < 32U && (input & 2147483648U) != 2147483648U)
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
    return (_BitScanReverse64(result&, input) == 0) ? 64U : 64U - result + 1U;
#else
    std::uint32_t high_result = count_leading_zero(static_cast<std::uint32_t>((input & 0xFFFFFFFF00000000) >> 32));
    return (high_result != 32U) ? high_result : count_leading_zero(static_cast<std::uint32_t>(input & 0x00000000FFFFFFFF)) + 32U;
#endif
}

} // namespace toolset
} // namespace turbo

#endif

