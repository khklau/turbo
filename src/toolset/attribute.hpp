#ifndef TURBO_TOOLSET_ATTRIBUTE_HPP
#define TURBO_TOOLSET_ATTRIBUTE_HPP

#if defined( _WIN32) && defined(_MSC_VER)
#define TURBO_DECL_EXPORT __declspec(dllexport)
#define TURBO_DECL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#define TURBO_DECL_EXPORT __attribute__((visibility("default")))
#define TURBO_DECL_IMPORT
#endif

#if !defined(TURBO_DECL_EXPORT)
#define TURBO_DECL_EXPORT
#endif
#if !defined(TURBO_DECL_IMPORT)
#define TURBO_DECL_IMPORT
#endif

#if defined(SHLIB_BUILD)
#define TURBO_SYMBOL_DECL TURBO_DECL_EXPORT
#else
#define TURBO_SYMBOL_DECL TURBO_DECL_IMPORT
#endif

#endif
