#ifndef GRAPHLITE_TEST_UTILS_H
#define GRAPHLITE_TEST_UTILS_H

#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "graph_lite.h"

using namespace graph_lite;

#define INTM(m) std::integral_constant<Map, m>
#define INTC(c) std::integral_constant<Container, c>

#define PAIR(m, c) std::pair<INTM(m), INTC(c)>

// 12 combinations in total
#define VERSATILE PAIR(Map::MAP, Container::VEC), PAIR(Map::MAP, Container::LIST), \
PAIR(Map::UNORDERED_MAP, Container::VEC), PAIR(Map::UNORDERED_MAP, Container::LIST)

#define NO_MULTI_EDGE PAIR(Map::MAP, Container::SET), PAIR(Map::MAP, Container::UNORDERED_SET), \
PAIR(Map::UNORDERED_MAP, Container::SET), PAIR(Map::UNORDERED_MAP, Container::UNORDERED_SET)

#define ONLY_MULTI_EDGE PAIR(Map::MAP, Container::MULTISET), PAIR(Map::MAP, Container::UNORDERED_MULTISET), \
PAIR(Map::UNORDERED_MAP, Container::MULTISET), PAIR(Map::UNORDERED_MAP, Container::UNORDERED_MULTISET)

#define EXTRACT_MC using MType = typename TypeParam::first_type;\
                   using CType = typename TypeParam::second_type;\
                   constexpr auto M = MType::value;\
                   constexpr auto C = CType::value;

using NoMultiEdge = testing::Types<VERSATILE, NO_MULTI_EDGE>;  // compile if disallowing multi-edges
using SupportMultiEdge = testing::Types<VERSATILE, ONLY_MULTI_EDGE>;  // compile if allowing multi-edges

// always gives a positive modulo
template<class IntType>
IntType mod(IntType a, IntType b) { return (a%b+b)%b; }

template<typename ItType, typename ValueType=std::remove_cv_t<typename std::iterator_traits<ItType>::value_type>>
bool neighbor_eq(ItType begin, ItType end, const std::multiset<ValueType>& ref) {
    std::multiset<ValueType> actual {begin, end};
    if (actual == ref) { return true; }
    else {
        std::cout << "Actual neighbors: ";
        for (auto & v: actual) { std::cout << v << " "; }
        std::cout << "\n";
        std::cout << "Provided neighbors: ";
        for (auto & v: ref) { std::cout << v << " "; }
        std::cout << "\n";
        return false;
    }
}

// test if node contains #count# val
template<typename ItType, typename ValueType=std::remove_cv_t<typename std::iterator_traits<ItType>::value_type>>
bool neighbor_contains(ItType begin, ItType end, const ValueType& val, size_t count=1) {
    std::multiset<ValueType> actual {begin, end};
    return actual.template count(val) == count;
}

// make an n-wheel graph, with n nodes
template<typename GraphType>
void make_wheel(GraphType& g_assign, int n) {
    assert(n >= 4);
    GraphType g;
    constexpr bool allow_multi_edge = GraphType::MULTI_EDGE == MultiEdge::ALLOWED;
    constexpr bool allow_self_loop = GraphType::SELF_LOOP == SelfLoop::ALLOWED;
    for (int i = 0; i < n; ++i) {
        ASSERT_EQ(g.add_nodes(i), 1);
        int num_self_loop_added = g.add_edge(i, i);
        if constexpr(allow_self_loop) {
            ASSERT_EQ(num_self_loop_added, 1);
            ASSERT_EQ(g.remove_edge(i, i), 1);
        } else {
            ASSERT_EQ(num_self_loop_added, 0);
            ASSERT_EQ(g.remove_edge(i, i), 0);
        }
    }
    ASSERT_EQ(g.add_nodes(0, 1, 2, 3), 0);
    for (int i = 0; i < n-1; ++i) {
        ASSERT_EQ(g.add_edge(i, mod(i+1, n-1)), 1);
    }
    for (int i = 0; i < n-1; ++i) {
        ASSERT_EQ(g.add_edge(i, n-1), 1);
        int num_multi_edge_added = g.add_edge(i, n-1);
        if constexpr(allow_multi_edge) {
            ASSERT_EQ(num_multi_edge_added, 1);
            ASSERT_EQ(g.remove_edge(i, n-1), 2);
            ASSERT_EQ(g.add_edge(i, n-1), 1);
        } else {
            ASSERT_EQ(num_multi_edge_added, 0);
        }
    }
    // unsuccessful edge additions
    ASSERT_EQ(g.add_edge(-1, 0), 0);
    ASSERT_EQ(g.add_edge(0, -1), 0);
    // unsuccessful edge removal
    ASSERT_EQ(g.remove_edge(0, 0), 0);
    ASSERT_EQ(g.remove_edge(0, 2), 0);
    // unsuccessful node removal
    ASSERT_EQ(g.remove_nodes(-1, -2, -3), 0);

    g_assign = std::move(g);
}

template<typename GraphType>
void test_wheel_construct(const GraphType& g) {
    int n = static_cast<int>(g.size());
    assert(n>=4);
    using NeighborIt = typename GraphType::NeighborsConstIterator;
    constexpr auto direction = GraphType::DIRECTION;
    NeighborIt begin;
    NeighborIt end;
    // center has n-1 in-neighbors
    if constexpr(direction==EdgeDirection::DIRECTED) {
        std::tie(begin, end) = g.in_neighbors(n-1);
    } else {
        std::tie(begin, end) = g.neighbors(n-1);
    }
    ASSERT_EQ(std::distance(begin, end), n-1);
    if constexpr(GraphType::DIRECTION==EdgeDirection::DIRECTED) {
        std::tie(begin, end) = g.out_neighbors(n-1);
        ASSERT_EQ(std::distance(begin, end), 0);
    } else {  // all are pointing to center
        std::tie(begin, end) = g.neighbors(n-1);
        ASSERT_EQ(std::distance(begin, end), n-1);
    }
    for (int i = 0; i < n - 1; ++i) {
        if constexpr(GraphType::DIRECTION==EdgeDirection::UNDIRECTED) {
            std::tie(begin, end) = g.neighbors(i);
            ASSERT_TRUE(neighbor_eq(begin, end, {n-1, mod(i+1, n-1), mod(i-1, n-1)}));
        } else {
            std::tie(begin, end) = g.out_neighbors(i);
            ASSERT_TRUE(neighbor_eq(begin, end, {n-1, mod(i+1, n-1)}));
            std::tie(begin, end) = g.in_neighbors(i);
            ASSERT_TRUE(neighbor_eq(begin, end, {mod(i-1, n-1)}));
        }
    }
}

template<typename GraphType>
void flip(GraphType& g) {
    int n = static_cast<int>(g.size());
    std::vector<std::pair<int, int>> v;
    for (auto&& [node, nbrs]: g) {
        typename GraphType::NeighborsIterator n_begin;
        typename GraphType::NeighborsIterator n_end;
        if constexpr(GraphType::DIRECTION==EdgeDirection::DIRECTED) {
            std::tie(n_begin, n_end) = nbrs.out;
        } else {
            std::tie(n_begin, n_end) = nbrs;
        }
        for (auto it=n_begin; it!=n_end; ++it) {
            const int& out_neighbor = [&it]{
                if constexpr(std::is_void_v<typename GraphType::edge_prop_type>) {
                    return *it;
                } else {
                    return it->first;
                }
            }();
            v.emplace_back(node, out_neighbor);
        }
    }
    for (const auto& [src, tgt]: v) {
        ASSERT_EQ(g.remove_edge(src, tgt), 1);
        ASSERT_EQ(g.add_edge(tgt, src), 1);
    }
}

#endif //GRAPHLITE_TEST_UTILS_H
