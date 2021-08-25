#include <gtest/gtest.h>
#include <random>
#include "graph_lite.h"

using namespace graph_lite;
using namespace graph_lite::detail;

#define INT(c) std::integral_constant<Container, c>

typedef testing::Types<INT(Container::VEC), INT(Container::LIST), INT(Container::MULTISET), INT(Container::UNORDERED_MULTISET)> ContainersWithDup;
typedef testing::Types<INT(Container::SET), INT(Container::UNORDERED_SET)> ContainersWithoutDup;
typedef testing::Types<INT(Container::VEC), INT(Container::LIST), INT(Container::MULTISET),
                       INT(Container::UNORDERED_MULTISET), INT(Container::SET), INT(Container::UNORDERED_SET)> Containers;

// helpers
template<typename ContainerType>
void make_rand_container(ContainerType& c, int max, size_t size) {
    std::mt19937 rng(666);
    std::uniform_int_distribution<> distrib(1, max);
    for (int i = 0; i < size; ++i) {
        container::insert(c, distrib(rng));
    }
}
// all containers should pass these
template <typename T>
class ContainerAllTest : public testing::Test {};
TYPED_TEST_SUITE(ContainerAllTest, Containers);

TYPED_TEST(ContainerAllTest, insert) {
    constexpr auto v = TypeParam::value;
    constexpr auto m = MultiEdgeTraits<v>::value;
    typename Graph<int, void, void, EdgeDirection::UNDIRECTED, m,
        SelfLoop::DISALLOWED, Map::UNORDERED_MAP, v>::NeighborsContainerType c;
    container::insert(c, 1);
    container::insert(c, 2);
    container::insert(c, 3);
    ASSERT_EQ(c.size(), 3);
}

TYPED_TEST(ContainerAllTest, find_and_remove) {
    constexpr auto v = TypeParam::value;
    constexpr auto m = MultiEdgeTraits<v>::value;
    typename Graph<int, void, void, EdgeDirection::UNDIRECTED, m,
    SelfLoop::DISALLOWED, Map::UNORDERED_MAP, v>::NeighborsContainerType c;    int max = 100;
    make_rand_container(c, max, 1000);
    for (int x = 0; x < max; ++x) {
        ASSERT_EQ(std::find(c.cbegin(), c.cend(), x), container::find(c, x));
        container::erase_all(c, x);
        ASSERT_EQ(container::find(c, x), c.end()) << "not all removed";
    }
}
// END OF all containers should pass these

// only containers that support dup should pass these
template <typename T>
class ContainerDupTest : public testing::Test {};
TYPED_TEST_SUITE(ContainerDupTest, ContainersWithDup);

TYPED_TEST(ContainerDupTest, insert) {
    constexpr auto v = TypeParam::value;
    constexpr auto m = MultiEdgeTraits<v>::value;
    typename Graph<int, void, void, EdgeDirection::UNDIRECTED, m,
        SelfLoop::DISALLOWED, Map::UNORDERED_MAP, v>::NeighborsContainerType c;    container::insert(c, 1);
    container::insert(c, 1);
    container::insert(c, 2);
    container::insert(c, 2);
    container::insert(c, 3);
    ASSERT_EQ(c.size(), 5);
    ASSERT_EQ(container::erase_all(c, 1), 2);
    ASSERT_EQ(container::erase_one(c, 1), 0);
    ASSERT_EQ(container::erase_one(c, 2), 1);
    auto remaining_two_pos = container::find(c, 2);
    ASSERT_NE(remaining_two_pos, c.cend()) << "should only remove one";
    container::erase_one(c, remaining_two_pos);
    ASSERT_EQ(container::find(c, 2), c.end()) << "removing by pos fails";
}

TYPED_TEST(ContainerDupTest, find_and_remove) {
    constexpr auto v = TypeParam::value;
    constexpr auto m = MultiEdgeTraits<v>::value;
    typename Graph<int, void, void, EdgeDirection::UNDIRECTED, m,
        SelfLoop::DISALLOWED, Map::UNORDERED_MAP, v>::NeighborsContainerType c;    int max = 100;
    make_rand_container(c, max, 1000);
    for (int x = 0; x < max; ++x) {
        size_t count = container::erase_all(c, x);
        for(int i=0; i<count; ++i) { container::insert(c, x); }
        size_t one_by_one_count = 0;
        while(container::find(c, x)!=c.end()) {
            ++one_by_one_count;
            ASSERT_EQ(container::erase_one(c, x), 1);
        }
        ASSERT_EQ(count, one_by_one_count);
    }
}
// only containers that support dup should pass these

// only containers that does NOT support dup should pass these
template <typename T>
class ContainerNoDupTest : public testing::Test {};
TYPED_TEST_SUITE(ContainerNoDupTest, ContainersWithoutDup);

TYPED_TEST(ContainerNoDupTest, insert) {
    constexpr auto v = TypeParam::value;
    constexpr auto m = MultiEdgeTraits<v>::value;
    typename Graph<int, void, void, EdgeDirection::UNDIRECTED, m,
        SelfLoop::DISALLOWED, Map::UNORDERED_MAP, v>::NeighborsContainerType c;    container::insert(c, 1);
    container::insert(c, 1);
    container::insert(c, 2);
    container::insert(c, 2);
    container::insert(c, 3);
    ASSERT_EQ(c.size(), 3);
    ASSERT_EQ(container::erase_one(c, 666), 0);
    ASSERT_EQ(container::erase_all(c, 1), 1);
    ASSERT_EQ(container::erase_one(c, 2), 1);
    ASSERT_EQ(container::find(c, 2), c.end());
    ASSERT_NE(container::find(c, 3), c.cend());
    container::insert(c, 3);
    ASSERT_EQ(container::erase_all(c, 3), 1);
}

TYPED_TEST(ContainerNoDupTest, find_and_remove) {
    constexpr auto v = TypeParam::value;
    constexpr auto m = MultiEdgeTraits<v>::value;
    typename Graph<int, void, void, EdgeDirection::UNDIRECTED, m,
        SelfLoop::DISALLOWED, Map::UNORDERED_MAP, v>::NeighborsContainerType c;    int max = 100;
    make_rand_container(c, max, 1000);
    for (int x = 0; x < max; ++x) {
        ASSERT_LE(container::erase_one(c, x), 1);
        ASSERT_EQ(container::find(c, x), c.end());
    }
}
// END OF only containers that does NOT support dup should pass these
