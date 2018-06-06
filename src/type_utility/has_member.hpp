#ifndef TURBO_TYPE_UTILITY_HAS_MEMBER_HPP
#define TURBO_TYPE_UTILITY_HAS_MEMBER_HPP

namespace turbo {
namespace type_utility {

template <typename arg_t>
class has_mem_type
{
private:
    using yes_t = char[1];
    using no_t = char[2];
    struct fallback_t
    {
	int some_member;
    };
    struct derived_t : arg_t, fallback_t
    { };
    template <typename some_t>
    static no_t& test(decltype(some_t::some_member)*);
    template <typename some_t>
    static yes_t& test(some_t*);
public:
    enum
    {
	result = (sizeof(test<derived_t>(nullptr)) == sizeof(yes_t))
    };
};

} // namespace type_utility
} // namespace turbo

#endif
