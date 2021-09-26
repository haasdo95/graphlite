#include <utility>

#include "test_utils.h"
using namespace graph_lite;

template<typename T>
struct TD;

// CAVEAT: rehashing can invalidate graph iterators when unordered map is used!!!
// i.e. don't rely on old iter after add_nodes/remove_nodes
template<typename T>
class DemoMultiEdge: public testing::Test {};
TYPED_TEST_SUITE(DemoMultiEdge, SupportMultiEdge);

template<typename T>
class DemoNoMultiEdge: public testing::Test {};
TYPED_TEST_SUITE(DemoNoMultiEdge, NoMultiEdge);

template<EdgeDirection direction, MultiEdge me, SelfLoop sl, Map M, Container C>
void test_edge_count() {
    Graph<int, void, void, direction, me, sl, M, C> g;
    ASSERT_EQ(g.num_edges(), 0);
    g.template add_nodes(0, 1, 2);
    g.template add_edge(0, 1);
    g.template add_edge(1, 2);
    g.template add_edge(2, 0);
    ASSERT_EQ(g.num_edges(), 3);
    g.template add_edge(0, 1);
    g.template add_edge(0, 1);
    if constexpr(me==graph_lite::MultiEdge::ALLOWED) {
        ASSERT_EQ(g.num_edges(), 5);
    } else {
        ASSERT_EQ(g.num_edges(), 3);
    }
    g.template add_edge(1, 1);
    g.template add_edge(1, 1);
    if constexpr(sl==graph_lite::SelfLoop::ALLOWED) {
        if constexpr(me==graph_lite::MultiEdge::ALLOWED) {
            ASSERT_EQ(g.num_edges(), 7);  // (0, 1), (0, 1), (0, 1) (1, 2), (2, 0) (1, 1), (1, 1)
        } else {
            ASSERT_EQ(g.num_edges(), 4);  // (0, 1), (1, 2), (2, 0) (1, 1)
        }
    } else {
        if constexpr(me==graph_lite::MultiEdge::ALLOWED) {
            ASSERT_EQ(g.num_edges(), 5);  // (0, 1), (0, 1), (0, 1) (1, 2), (2, 0)
        } else {
            ASSERT_EQ(g.num_edges(), 3);  // (0, 1), (1, 2), (2, 0)
        }
    }
    g.template add_nodes(3);
    g.template add_edge(0, 3);
    g.template add_edge(1, 3);
    g.template add_edge(2, 3);
    g.template remove_nodes(1);  // purge all 1-related edges
    ASSERT_EQ(g.num_edges(), 3);  // only (2, 0), (0, 3), (2, 3) left
    g.template add_edge(0, 2);
    if constexpr(direction==graph_lite::EdgeDirection::UNDIRECTED and me==graph_lite::MultiEdge::DISALLOWED) {
        ASSERT_EQ(g.num_edges(), 3);
    } else {
        ASSERT_EQ(g.num_edges(), 4);
    }
    g.template remove_nodes(0);
    ASSERT_EQ(g.num_edges(), 1);  // only (2, 3) left
}

TYPED_TEST(DemoNoMultiEdge, edge_count) {
    EXTRACT_MC
    test_edge_count<EdgeDirection::UNDIRECTED, MultiEdge::DISALLOWED, SelfLoop::ALLOWED, M, C>();
    test_edge_count<EdgeDirection::UNDIRECTED, MultiEdge::DISALLOWED, SelfLoop::DISALLOWED, M, C>();
    test_edge_count<EdgeDirection::DIRECTED, MultiEdge::DISALLOWED, SelfLoop::ALLOWED, M, C>();
    test_edge_count<EdgeDirection::DIRECTED, MultiEdge::DISALLOWED, SelfLoop::DISALLOWED, M, C>();
}

TYPED_TEST(DemoMultiEdge, edge_count) {
    EXTRACT_MC
    test_edge_count<EdgeDirection::UNDIRECTED, MultiEdge::ALLOWED, SelfLoop::ALLOWED, M, C>();
    test_edge_count<EdgeDirection::UNDIRECTED, MultiEdge::ALLOWED, SelfLoop::DISALLOWED, M, C>();
    test_edge_count<EdgeDirection::DIRECTED, MultiEdge::ALLOWED, SelfLoop::ALLOWED, M, C>();
    test_edge_count<EdgeDirection::DIRECTED, MultiEdge::ALLOWED, SelfLoop::DISALLOWED, M, C>();
}

template<EdgeDirection direction, MultiEdge me, Map M, Container C>
void test_val_iter() {
    Graph<int, void, double, direction,
          me, SelfLoop::DISALLOWED, M, C> g;
    const auto& cg = g;
    g.add_nodes(0, 1);
    g.add_edge_with_prop(0, 1, 1.0);
    auto res_iv = g.edge_prop(g.find(0), 1);
    auto res_vi = g.edge_prop(0, g.find(1));
    auto res_ii = g.edge_prop(g.find(0), g.find(1));
    auto res_vv = g.edge_prop(0, 1);
    auto res = [&g](){
        if constexpr(direction==EdgeDirection::UNDIRECTED){
            return g.find_neighbor(0, 1).second->second.prop();
        } else {
            return g.find_out_neighbor(0, 1).second->second.prop();
        }
    }();
    auto res_c = [&cg](){
        if constexpr(direction==EdgeDirection::UNDIRECTED){
            return cg.find_neighbor(0, 1).second->second.prop();
        } else {
            return cg.find_out_neighbor(0, 1).second->second.prop();
        }
    }();
    ASSERT_EQ(res_c, res);
    ASSERT_EQ(res, res_iv);
    ASSERT_EQ(res_iv, res_vi);
    ASSERT_EQ(res_vi, res_ii);
    ASSERT_EQ(res_ii, res_vv);
    ASSERT_EQ(res_vv, 1.0);
    // mutability
    g.edge_prop(cg.find(0), 1) = -1.0;
    g.edge_prop(cg.find(0), 1)--;
    auto new_res = [&]{
        if constexpr(direction==graph_lite::EdgeDirection::UNDIRECTED) {
            return cg.find_neighbor(g.find(1), 0).second->second.prop();
        } else {
            return cg.find_in_neighbor(g.find(1), 0).second->second.prop();
        }
    }();
    ASSERT_EQ(new_res, -2.0);
    // unsuccessful queries of edge prop
    ASSERT_ANY_THROW(g.edge_prop(0, -1));
    ASSERT_ANY_THROW(g.edge_prop(g.find(0), -1));
    ASSERT_ANY_THROW(g.edge_prop(-1, 0));
    ASSERT_ANY_THROW(g.edge_prop(-1, g.find(0)));
}

TYPED_TEST(DemoMultiEdge, val_or_iter) {
    EXTRACT_MC
    test_val_iter<EdgeDirection::UNDIRECTED, MultiEdge::ALLOWED, M, C>();
    test_val_iter<EdgeDirection::DIRECTED, MultiEdge::ALLOWED, M, C>();
}

TYPED_TEST(DemoNoMultiEdge, val_or_iter) {
    EXTRACT_MC
    test_val_iter<EdgeDirection::UNDIRECTED, MultiEdge::DISALLOWED, M, C>();
    test_val_iter<EdgeDirection::DIRECTED, MultiEdge::DISALLOWED, M, C>();
}

struct Person {
    int id;
    std::string name;
    explicit Person(int id): id{id}, name{} {}
    Person(int id, std::string  name): id{id}, name{std::move(name)} {}
    bool operator==(const Person& other) const { return id==other.id; }
//    bool operator!=(const Person& other) const { return id!=other.id; }  // not needed
    bool operator<(const Person& other) const { return id<other.id; }
    friend std::ostream& operator<<(std::ostream& os, const Person& p);
};
namespace std {
    template<>
    struct hash<Person> {
        size_t operator()(const Person& p) const {
            return hash<int>()(p.id);
        }
    };
}

std::ostream &operator<<(std::ostream &os, const Person &p) {
    os << "id=" << p.id;
    return os;
}

template<MultiEdge me, Map M, Container C>
void test_imp_conv() {
    Graph<int, std::string, Person, EdgeDirection::UNDIRECTED,
          me, SelfLoop::DISALLOWED, M, C> g;
    const auto& cg = g;
    g.add_node_with_prop(0, "0");
    g.add_node_with_prop(1, "1");
    g.add_edge_with_prop(0, 1, 123);
    auto pos_nc_1 = g.find_neighbor(0, 1).second;  // PairIterator
    auto pos_c_1 = cg.find_neighbor(0, 1).second;
    auto pos_nc_2 = g.neighbors(0).first;
    auto pos_c_2 = cg.neighbors(0).first;
    // comparison between non const and const always succeeds
    ASSERT_EQ(pos_nc_1, pos_c_1);
    ASSERT_EQ(pos_c_1, pos_nc_1);
    ASSERT_EQ(pos_nc_2, pos_c_2);
    ASSERT_EQ(pos_c_2, pos_nc_2);

    ASSERT_EQ(pos_nc_1, pos_nc_2);
    ASSERT_EQ(pos_c_1, pos_c_2);
    //node iter imp conv
    ASSERT_EQ(g.end(), cg.end());
    ASSERT_EQ(cg.begin(), g.begin());

    // two entries refer to the same entity
    ASSERT_EQ(&g.edge_prop(0, 1), &g.edge_prop(1, 0));

    // piecewise-construction
    Graph<int, std::map<std::string, int>, std::map<std::string, int>,
            EdgeDirection::DIRECTED, me, SelfLoop::DISALLOWED, M, C> mg;
    mg.template add_node_with_prop(0, std::make_pair("age", 21), std::make_pair("salary", 21000));
    auto pos_0 = mg.template find(0);
    ASSERT_EQ(mg.template node_prop(pos_0).at("age"), 21);
    ASSERT_EQ(mg.template node_prop(pos_0).at("salary"), 21000);
    mg.template add_node_with_prop(1);  // empty node attr
    ASSERT_EQ(mg.template node_prop(1).size(), 0);
    mg.template add_edge_with_prop(0, 1);  // empty edge attr
    ASSERT_EQ(mg.template edge_prop(0, 1).size(), 0);
    mg.template add_edge_with_prop(1, 0, std::map<std::string, int>{{"common-friends", 3}});  // can't use pair if only one
    ASSERT_EQ(mg.template edge_prop(1, 0).at("common-friends"), 3);
}

TYPED_TEST(DemoMultiEdge, test_imp_conv) {
    EXTRACT_MC
    test_imp_conv<MultiEdge::ALLOWED, M, C>();
}
TYPED_TEST(DemoNoMultiEdge, test_imp_conv) {
    EXTRACT_MC
    test_imp_conv<MultiEdge::DISALLOWED, M, C>();
}

TYPED_TEST(DemoMultiEdge, multi_edge_self_loop_undirected) {
    EXTRACT_MC
    using GType = Graph<Person, void, int, EdgeDirection::UNDIRECTED,
                        MultiEdge::ALLOWED, SelfLoop::ALLOWED,
                        M, C>;
    GType g;
    g.add_nodes(Person{0, "zero"}, Person{1, "one"}, Person{2, "two"}, Person{3, "three"});
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < i; ++j) {  // add self edges
            ASSERT_EQ(g.add_edge_with_prop(i, i, j), 1);
        }
        for (int j = i + 1; j < 4; ++j) {
            for (int k = 1; k <= j - i; ++k) {
                ASSERT_EQ(g.add_edge_with_prop(i, j, k), 1);
            }
        }
    }
    // check neighbor correctness
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i==j) {
                ASSERT_EQ(g.count_edges(i, j), i);
            } else if (i < j) {
                ASSERT_EQ(g.count_edges(i, j), j - i);
            } else {
                assert(i > j);
                ASSERT_EQ(g.count_edges(i, j), i - j);
            }
        }
    }
    ASSERT_EQ(g.count_edges(-1, 0), 0);
    ASSERT_EQ(g.count_edges(0, -1), 0);  // correct behavior when node is invalid
    // example of using the neighbor view
    ASSERT_ANY_THROW(g.neighbors(-1));
    ASSERT_ANY_THROW(g.find_neighbor(-1, 0)); // throw when node is invalid
    auto pos_2 = g.find(2);
    auto [out_begin, out_end] = g.neighbors(pos_2);
    auto same_node = [](int n){
        return [n](const auto & p) {
            return p.first==Person{n};
        };
    };
    ASSERT_EQ(std::count_if(out_begin, out_end, same_node(2)), 2);
    ASSERT_EQ(std::count_if(out_begin, out_end, same_node(3)), 1);
    ASSERT_EQ(std::count_if(out_begin, out_end, same_node(0)), 2);
    ASSERT_EQ(std::count_if(out_begin, out_end, same_node(1)), 1);

    auto [found_2_to_3, edge_2_to_3] = g.find_neighbor(pos_2, 3);
    ASSERT_TRUE(found_2_to_3);
    ASSERT_EQ(edge_2_to_3->first, Person{ 3 });
    ASSERT_EQ(edge_2_to_3->second.prop(), 1);
    // remove edge by iterator
    auto [found_2_to_2, edge_2_to_2] = g.find_neighbor(2, 2);
    ASSERT_TRUE(found_2_to_2);
    int removed = edge_2_to_2->second.prop();
    ASSERT_TRUE(removed < 2);
    ASSERT_EQ(g.remove_edge(pos_2, edge_2_to_2), 1);  // remove one of the two self edges
    auto [found_2_to_2_other, edge_2_to_2_other] = g.find_neighbor(pos_2, 2);
    ASSERT_TRUE(found_2_to_2_other);
    int other_prop = 1 - removed;
    ASSERT_EQ(edge_2_to_2_other->second.prop(), other_prop);  // should find the other self edge
    ASSERT_EQ(g.add_edge_with_prop(pos_2, pos_2, removed), 1); // revert the change
    // remove edge by value; this always removes all edges in between
    ASSERT_EQ(g.remove_edge(3, 3), 3);
    ASSERT_EQ(g.count_edges(3, 3), 0);
    for (int i = 0; i < 3; ++i) {
        ASSERT_EQ(g.add_edge_with_prop(3, 3, i), 1);  // revert the change
    }
    // remove node 2 and check neighbors
    ASSERT_EQ(g.count_neighbors(0), 6);
    ASSERT_EQ(g.count_neighbors(3), 9);
    ASSERT_EQ(g.remove_nodes(pos_2), 1);
    ASSERT_EQ(g.count_neighbors(0), 4);
    ASSERT_EQ(g.count_neighbors(3), 8);
    ASSERT_EQ(g.remove_nodes(0, 3, -1), 2);
    ASSERT_EQ(g.size(), 1);
    auto [begin_1, end_1] = g.neighbors(1);
    ASSERT_EQ(std::distance(begin_1, end_1), 1);
    ASSERT_EQ(begin_1->first, Person{1});
    ASSERT_EQ(begin_1->second.prop(), 0);
}

TYPED_TEST(DemoMultiEdge, self_loop_multi_edge_directed) {
    EXTRACT_MC
    using GType = Graph<int, std::string, double, EdgeDirection::DIRECTED,
                        MultiEdge::ALLOWED, SelfLoop::ALLOWED,
                        M, C>;
    GType g;
    ASSERT_EQ(g.add_node_with_prop(0, "0"), 1);
    ASSERT_EQ(g.add_node_with_prop(1, "1"), 1);
    ASSERT_EQ(g.add_node_with_prop(2, "2"), 1);
    ASSERT_EQ(g.add_node_with_prop(3, "3"), 1);
    // adding self edge
    ASSERT_EQ(g.add_edge_with_prop(0, 0, 0.0), 1);
    // add duplicate edges
    ASSERT_EQ(g.add_edge_with_prop(0, 1, 1.0), 1);
    ASSERT_EQ(g.add_edge_with_prop(0, 1, 1.0), 1);
    ASSERT_EQ(g.add_edge_with_prop(0, 3, 3.0), 1);
    ASSERT_EQ(g.add_edge_with_prop(0, 3, 3.0), 1);
    // add non-dup edges
    ASSERT_EQ(g.add_edge_with_prop(1, 2, 1.0), 1);
    ASSERT_EQ(g.add_edge_with_prop(2, 1, -1.0), 1);
    ASSERT_EQ(g.add_edge_with_prop(2, 3, 1.0), 1);
    ASSERT_EQ(g.add_edge_with_prop(3, 2, -1.0), 1);
    // unsuccessful remove edges
    ASSERT_EQ(g.remove_edge(-1, 0), 0);
    ASSERT_EQ(g.remove_edge(0, -1), 0);
    ASSERT_EQ(g.remove_edge(0, 2), 0);

    const GType& cg = g;
    // unsuccessful find neighbors
    ASSERT_ANY_THROW(cg.find_in_neighbor(-1, 0));
    ASSERT_ANY_THROW(g.find_out_neighbor(g.find(-1), 0));
    // unsuccessful in-out neighbors access
    ASSERT_ANY_THROW(g.out_neighbors(-1));
    ASSERT_ANY_THROW(cg.out_neighbors(-1));
    ASSERT_ANY_THROW(cg.in_neighbors(g.find(-1)));
    ASSERT_ANY_THROW(g.count_in_neighbors(cg.find(-1)));
    ASSERT_ANY_THROW(cg.count_out_neighbors(g.find(-1)));
    // check edges
    auto one_pos = cg.find(1);
    auto [nb, ne] = cg.out_neighbors(one_pos);
    ASSERT_EQ(nb->second.prop(), 1.0);
    ASSERT_EQ(std::distance(nb, ne), 1);  // only one out-neighbor
    auto [in_nb, in_ne] = cg.in_neighbors(one_pos);
    ASSERT_EQ(std::distance(in_nb, in_ne), 3);  // 3 in-neighbors
    ASSERT_EQ(cg.count_in_neighbors(0), 1);
    ASSERT_EQ(cg.count_out_neighbors(0), 5);
    auto [found_zero_in, zero_in] = cg.find_in_neighbor(0, 0);
    auto [found_zero_out, zero_out] = cg.find_out_neighbor(g.find(0), 0);
    auto [found_zero_non_c, zero_out_non_c] = g.find_out_neighbor(0, 0);
    ASSERT_TRUE(zero_out_non_c==zero_out);
    ASSERT_TRUE(zero_out==zero_out_non_c);
    ASSERT_TRUE(found_zero_in && found_zero_out && found_zero_non_c);
    ASSERT_EQ(zero_in->first, 0);
    ASSERT_EQ(zero_out->first, 0);
    ASSERT_EQ(zero_in->second.prop(), 0.0);
    ASSERT_EQ(zero_out->second.prop(), 0.0);
    ASSERT_EQ(cg.count_edges(0, 3), 2);
    ASSERT_EQ(cg.count_edges(0, 0), 1);
    // remove nodes and test correctness
    ASSERT_EQ(g.remove_nodes(2, 3), 2);
    ASSERT_EQ(cg.find(2), g.end());
    ASSERT_EQ(cg.size(), 2);
    ASSERT_EQ(cg.count_out_neighbors(g.find(1)), 0);
    ASSERT_EQ(cg.count_in_neighbors(g.find(1)), 2);
}

TYPED_TEST(DemoNoMultiEdge, undirected) {
    EXTRACT_MC
    using GType = Graph<std::string, int, double, EdgeDirection::UNDIRECTED,
                        MultiEdge::DISALLOWED, SelfLoop::DISALLOWED,
                        M, C>;
    GType g;
    ASSERT_EQ(g.add_node_with_prop(std::string{"Alice"}, 19), 1);
    ASSERT_EQ(g.add_node_with_prop(std::string{"Bob"}, 20), 1);
    ASSERT_EQ(g.add_node_with_prop(std::string{"Cyrus"}, 21), 1);
    ASSERT_EQ(g.add_node_with_prop(std::string{"Alice"}, 100), 0);
    // check node existence
    ASSERT_TRUE(g.has_node("Bob"));
    ASSERT_FALSE(g.has_node("bob"));
    // add edge between existing nodes
    ASSERT_EQ(g.find("alice"), g.end());
    ASSERT_EQ(g.add_edge_with_prop("Alice", "Bob", 0.1), 1);
    ASSERT_EQ(g.add_edge_with_prop("Bob", "Cyrus", 0.2), 1);
    ASSERT_EQ(g.add_edge_with_prop("Cyrus", "Alice", 0.3), 1);
    ASSERT_EQ(g.add_edge_with_prop("Alice", "Bob", 123), 0);  // existing edge
    ASSERT_EQ(g.add_edge_with_prop("Bob", "Alice", 123), 0);  // existing edge
    ASSERT_EQ(g.add_edge_with_prop("Alice", "Alice", 123), 0);  // no self loop
    ASSERT_EQ(g.add_edge_with_prop("alice", "Bob", 0.1), 0); // non existent src
    ASSERT_EQ(g.add_edge_with_prop("Alice", "bob", 0.1), 0); // non existent tgt
    // simple neighbor queries
    auto alice_pos = g.find("Alice");
    auto out_nbrs_by_it = g.neighbors(alice_pos);
    auto out_nbrs_by_value = g.neighbors("Alice");
    ASSERT_EQ(out_nbrs_by_it, out_nbrs_by_value);
    ASSERT_ANY_THROW(g.neighbors("alice"));
    ASSERT_EQ(g.count_edges("Alice", "Bob"), 1);
    ASSERT_EQ(g.count_edges("alice", "bob"), 0);
    ASSERT_EQ(g.count_edges(alice_pos, "bob"), 0);
    // unsuccessful edge removal
    ASSERT_EQ(g.remove_edge(alice_pos, alice_pos), 0);
    // add and remove an isolated node
    ASSERT_EQ(g.add_node_with_prop(std::string{"Derek"}, 22), 1);
    ASSERT_EQ(g.count_edges(alice_pos, "Derek"), 0);
    ASSERT_EQ(g.remove_nodes("Derek"), 1);
    ASSERT_EQ(g.remove_nodes("Derek"), 0);
    ASSERT_FALSE(g.has_node("Derek"));
    // search for a node in the (out)neighbors of another node
    alice_pos = g.find("Alice"); // iter can be invalidated after node changes
    auto [found_a, it_a] = g.find_neighbor(alice_pos, "Cyrus");
    ASSERT_TRUE(found_a && it_a->second.prop()==0.3);
    auto [found_b, it_b] = g.find_neighbor("Bob", "Cyrus");
    ASSERT_TRUE(found_b && it_b->second.prop()==0.2);
    // remove a connected node
    ASSERT_EQ(g.remove_nodes("Cyrus"), 1);
    alice_pos = g.find("Alice"); // iter can be invalidated after node changes
    ASSERT_FALSE(g.find_neighbor(alice_pos, "Cyrus").first);
    ASSERT_FALSE(g.find_neighbor("Bob", "Cyrus").first);
    ASSERT_EQ(g.find("Cyrus"), g.end());
    // get him back
    g.add_node_with_prop(std::string{"Cyrus"}, 21);
    g.add_edge_with_prop("Cyrus", "Alice", 0);
    g.add_edge_with_prop("Cyrus", "Bob", 0);
    // isolate him by removing edges
    ASSERT_EQ(g.remove_edge("Cyrus", "Bob"), 1);
    auto [found_a_again, it_a_again] = g.find_neighbor(alice_pos, "Cyrus");
    ASSERT_TRUE(found_a_again);
    ASSERT_EQ(g.remove_edge(alice_pos, it_a_again), 1);
    auto c_nbrs = g.neighbors(g.find("Cyrus"));
    ASSERT_EQ(std::distance(c_nbrs.first, c_nbrs.second), 0); // should be no neighbor left
    // access helper for node prop
    int& c_prop = g.node_prop("Cyrus");
    ASSERT_EQ(c_prop, 21);
    c_prop = 12;
    const GType& cg = g;
    ASSERT_EQ(cg.node_prop("Cyrus"), 12);
    g.node_prop(alice_pos) = 0;
    ASSERT_EQ(g.node_prop("Alice"), 0);
}

TYPED_TEST(DemoNoMultiEdge, directed) {
    EXTRACT_MC
    using GType = Graph<int, std::string, double,
                        EdgeDirection::DIRECTED, MultiEdge::DISALLOWED, SelfLoop::DISALLOWED,
                        M, C>;
    GType g;
    g.add_node_with_prop(1, "A");
    g.add_node_with_prop(2, "B");
    g.add_node_with_prop(3, std::string{"C"});
    ASSERT_EQ(g.size(), 3);
    for (const auto& node: g) {  // range loop
        ASSERT_TRUE(node <= 3);
    }
    for (auto it=g.begin(); it!=g.end(); ++it) {  // better use the helper method node_prop
        g.node_prop(it) += "123";
    }
    const auto & cg = g;
    auto cbegin = cg.begin();
    cbegin = g.begin(); // implicit from iter to const_iter should work
    //    auto begin = g.begin();
    //    begin = cg.begin();  // the other way around should not work; losing const-ness

    for (auto it=cg.begin(); it!=cg.end(); ++it) {  // the string should be const now
        std::cout << *it << ": " << cg.node_prop(it) << "\n";
        //        cg.node_prop(it) = "should not work, either";  // for the same reason as above
    }

    ASSERT_EQ(g.remove_edge(1, 1), 0);  // no-op to remove a non-existing edge
    ASSERT_EQ(g.remove_edge(1, 2), 0);  // no-op to remove a non-existing edge

    for (auto it_i=g.begin(); it_i!=g.end(); ++it_i) {
        for (auto it_j=g.begin(); it_j!=g.end(); ++it_j) {
            std::cout << "adding edge between " << *it_i << " and " << *it_j << std::endl;
            int num_added = g.add_edge_with_prop(it_i, it_j, static_cast<double>(*it_i - *it_j));
            ASSERT_TRUE(*it_i==*it_i or num_added==1);
        }
    }

    for (auto it=cg.begin(); it!=cg.end(); ++it) {
        auto [out_begin, out_end] = cg.out_neighbors(it);
        for (auto n_it=out_begin; n_it!=out_end; ++n_it) {
            //            n_it->second.prop() = 777;  // should NOT be assignable
            ASSERT_EQ(n_it->second.prop(), *it-n_it->first);
            std::cout << "Edge Prop of Edge " << *it << "->" << n_it->first << ": " << n_it->second.prop() << "\n";
        }
    }

    for (auto it=g.begin(); it!=g.end(); ++it) {  // flip all edge props
        auto [out_begin, out_end] = g.template out_neighbors(it);
        for (auto n_it=out_begin; n_it!=out_end; ++n_it) {
            n_it->second.prop() = -(n_it->second.prop());
        }
    }
    // check edge prop flipping
    for (auto it=cg.begin(); it!=cg.end(); ++it) {
        auto [out_begin, out_end] = g.template out_neighbors(it);
        for (auto n_it=out_begin; n_it!=out_end; ++n_it) {
            ASSERT_EQ(n_it->second.prop(), n_it->first-*it);
        }
    }
}
