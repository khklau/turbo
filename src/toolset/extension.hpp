#ifndef TURBO_TOOLSET_EXTENSION_HPP
#define TURBO_TOOLSET_EXTENSION_HPP

#if defined(__GNUC__) || defined(__clang__)
#if defined(__has_builtin)
#if __has_builtin(__builtin_expect)
#define TURBO_LIKELY(x) __builtin_expect(x, 1)
#define TURBO_UNLIKELY(x) __builtin_expect(x, 0)
#endif
#endif
#endif

#if !defined(TURBO_LIKELY)
#define TURBO_LIKELY(x) x
#endif
#if !defined(TURBO_UNLIKELY)
#define TURBO_UNLIKELY(x) x
#endif

#endif
