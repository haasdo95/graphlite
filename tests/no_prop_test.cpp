#include "test_utils.h"

using namespace graph_lite;

// basic functionality; no multi-edge/self-loop involved at all
#define WHEEL_BASIC(dir, me, sl) EXTRACT_MC\
                            constexpr auto direction = EdgeDirection::dir;\
                            constexpr auto multi_edge = MultiEdge::me;\
                            constexpr auto self_loop = SelfLoop::sl;\
                            Graph<int, void, void, direction, multi_edge, self_loop, M, C> g; \
                            make_wheel(g, 5);\
                            test_wheel_construct(g);\
                            flip(g);\
                            flip(g);\
                            test_wheel_construct(g);


// TODO: some more operations on the wheel after this that involves multi-edge & self-loop

// no multi edge tests
template <typename T>
class NoMultiEdgeTest: public testing::Test {};
TYPED_TEST_SUITE(NoMultiEdgeTest, NoMultiEdge);

TYPED_TEST(NoMultiEdgeTest, test_wheel_directed_no_sl) { WHEEL_BASIC(DIRECTED, DISALLOWED, DISALLOWED); }
TYPED_TEST(NoMultiEdgeTest, test_wheel_undirected_no_sl) { WHEEL_BASIC(UNDIRECTED, DISALLOWED, DISALLOWED); }
TYPED_TEST(NoMultiEdgeTest, test_wheel_directed_sl) { WHEEL_BASIC(DIRECTED, DISALLOWED, ALLOWED); }
TYPED_TEST(NoMultiEdgeTest, test_wheel_undirected_sl) { WHEEL_BASIC(UNDIRECTED, DISALLOWED, ALLOWED); }

// END OF no multi edge tests


// multi edge tests
template <typename T>
class MultiEdgeTest: public testing::Test {};
TYPED_TEST_SUITE(MultiEdgeTest, SupportMultiEdge);

TYPED_TEST(MultiEdgeTest, test_wheel_directed_no_sl) { WHEEL_BASIC(DIRECTED, ALLOWED, DISALLOWED); }
TYPED_TEST(MultiEdgeTest, test_wheel_undirected_no_sl) { WHEEL_BASIC(UNDIRECTED, ALLOWED, DISALLOWED); }
TYPED_TEST(MultiEdgeTest, test_wheel_directed_sl) { WHEEL_BASIC(DIRECTED, ALLOWED, ALLOWED); }
TYPED_TEST(MultiEdgeTest, test_wheel_undirected_sl) { WHEEL_BASIC(UNDIRECTED, ALLOWED, ALLOWED); }

// END OF multi edge tests
