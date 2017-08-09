#include <turbo/container/heap.hpp>
#include <turbo/container/heap.hxx>
#include <gtest/gtest.h>
#include <cstdint>

namespace turbo {
namespace container {

template <class element_t, class compare_f, class allocator_t>
class heap_tester
{
public:
    typedef element_t element_type;
    typedef compare_f compare_func;
    typedef allocator_t allocator_type;
    typedef heap<element_t, compare_f, allocator_t> heap_type;
    heap_tester(heap_type& heap);
    bool has_heap_property() const
    {
	return heap_.has_heap_property();
    }
private:
    heap_type& heap_;
};

template <class e, class c, class a>
heap_tester<e, c, a>::heap_tester(heap_type& heap)
    :
	heap_(heap)
{ }

} // namespace container
} // namespace turbo

namespace tco = turbo::container;

TEST(heap_test, emplace_basic)
{
    tco::heap<std::uint8_t> heap1;
    tco::heap_tester<std::uint8_t> tester1(heap1);
    heap1.emplace_back(71);
    EXPECT_EQ(71, heap1.front()) << "The root of the heap is not the value just emplaced";
    EXPECT_EQ(71, heap1.back()) << "The right most leaf of the heap is not the value just emplaced";
    EXPECT_TRUE(tester1.has_heap_property()) << "The heap property is not preserved after emplace";
    heap1.emplace_back(10);
    EXPECT_EQ(71, heap1.front()) << "The root of the heap is not the largest value";
    EXPECT_EQ(10, heap1.back()) << "The right most leaf of the heap is not the just emplaced smallest value";
    EXPECT_TRUE(tester1.has_heap_property()) << "The heap property is not preserved after emplace";
    heap1.emplace_back(192);
    EXPECT_EQ(192, heap1.front()) << "The root of the heap is not the largest value";
    EXPECT_EQ(71, heap1.back()) << "The right most leaf of the heap is not the 2nd largest value";
    EXPECT_TRUE(tester1.has_heap_property()) << "The heap property is not preserved after emplace";
}
