#include <gtest/gtest.h>
#include <graph_lite.h>
#include <serialize.h>

using namespace graph_lite;

#include <utility>

template<typename S>
void compare(const S& s, const std::string& sol) {
    std::stringstream ss;
    s.serialize_to_dot(ss);
    ASSERT_EQ(ss.str(), sol);
}



TEST(serializer, simple) {
    Graph<int, void, void, EdgeDirection::UNDIRECTED, MultiEdge::DISALLOWED, SelfLoop::DISALLOWED, Map::MAP, Container::VEC> g;
    Serializer empty_s{g};
    compare(empty_s, "strict graph {\n"
                     "}\n");

    g.add_nodes(0, 1, 2, 3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    Serializer s{g};
    std::string simple_sol = "strict graph {\n"
                         "\t0; 1; 2; 3; \n"
                         "\t0--1; 1--2; 2--3; \n"
                         "}\n";
    compare(s, simple_sol);

    s.set_max_num_nodes_per_line(2);
    s.set_max_num_edges_per_line(2);
    std::string simple_fmt_sol = "strict graph {\n"
                                 "\t0; 1; \n"
                                 "\t2; 3; \n"
                                 "\t0--1; 1--2; \n"
                                 "\t2--3; \n"
                                 "}\n";
    compare(s, simple_fmt_sol);

    s.unset_max_num_nodes_per_line();
    s.unset_max_num_edges_per_line();
    compare(s, simple_sol);

    // test multi-edge/self-loop
    auto make_graph = [](auto& gg) {
        gg.add_nodes(0, 1, 2, 3, 4);
        gg.add_edge(0, 0); gg.add_edge(4, 4); gg.add_edge(4, 4);
        gg.add_edge(0, 1); gg.add_edge(0, 2); gg.add_edge(0, 3);
        gg.add_edge(1, 4); gg.add_edge(2, 4); gg.add_edge(3, 4);
        gg.add_edge(1, 2); gg.add_edge(1, 2);
        gg.add_edge(2, 3); gg.add_edge(2, 3);
    };

    Graph<int, void, void, EdgeDirection::UNDIRECTED,
        MultiEdge::ALLOWED, SelfLoop::ALLOWED, Map::MAP, graph_lite::Container::MULTISET> udg;
    make_graph(udg);
    Serializer udgs{udg};
    compare(udgs, "graph {\n"
                "\t0; 1; 2; 3; 4; \n"
                "\t0--0; 0--1; 0--2; 0--3; 1--2; 1--2; 1--4; 2--3; 2--3; 2--4; 3--4; 4--4; 4--4; \n"
                "}\n");

    Graph<int, void, void, EdgeDirection::DIRECTED,
        MultiEdge::ALLOWED, SelfLoop::ALLOWED, Map::MAP, graph_lite::Container::MULTISET> dg;
    make_graph(dg);
    Serializer dgs{dg};
    compare(dgs, "digraph {\n"
                 "\t0; 1; 2; 3; 4; \n"
                 "\t0->0; 0->1; 0->2; 0->3; 1->2; 1->2; 1->4; 2->3; 2->3; 2->4; 3->4; 4->4; 4->4; \n"
                 "}\n");
}

template<typename NPT, typename EPT>
using graph_with_prop = Graph<int, NPT, EPT, EdgeDirection::DIRECTED, MultiEdge::ALLOWED, SelfLoop::DISALLOWED, Map::MAP, Container::VEC>;
struct Noble {
    std::string name;
    std::string address;
    Noble(std::string name, std::string address): name(std::move(name)), address(std::move(address)) {}
    friend std::ostream& operator<<(std::ostream& os, const Noble& p) {
        os << p.name << " de " << p.address;
        return os;
    }
    std::map<std::string, std::string> to_map() {
        return {{"name", name}, {"address", address}};
    };
};

struct Empty{};

TEST(serializer, with_prop) {
    // PropType is already serializable
    graph_with_prop<Noble, Noble> g;
    g.add_node_with_prop(0, "Duke", "York");
    g.add_node_with_prop(1, "Lorenzo", "Medici");
    g.add_edge_with_prop(0, 1, "Otto", "Bismarck");
    Serializer s{g};
    compare(s, "digraph {\n"
               "\t0[label=\"Duke de York\"]; 1[label=\"Lorenzo de Medici\"]; \n"
               "\t0->1[label=\"Otto de Bismarck\"]; \n"
               "}\n");
    // override with custom formatter
    auto fmt = [](const Noble& n) { return "person=" + n.name + "@" + n.address; };
    s.register_node_formatter(fmt);
    compare(s, "digraph {\n"
               "\t0[person=Duke@York]; 1[person=Lorenzo@Medici]; \n"
               "\t0->1[label=\"Otto de Bismarck\"]; \n"
               "}\n");
    s.delete_node_formatter();
    s.register_edge_formatter(fmt);
    compare(s, "digraph {\n"
               "\t0[label=\"Duke de York\"]; 1[label=\"Lorenzo de Medici\"]; \n"
               "\t0->1[person=Otto@Bismarck]; \n"
               "}\n");
    s.delete_edge_formatter();
    // prop is already a map
    graph_with_prop<std::map<std::string, std::string>, std::map<std::string, std::string>> mg;
    mg.add_node_with_prop(0, std::make_pair("name", "Duke"), std::make_pair("address", "York"));
    mg.add_node_with_prop(1, Noble("Lorenzo", "Medici").to_map());
    mg.add_edge_with_prop(0, 1, Noble("Otto", "Bismarck").to_map());
    Serializer ms{mg};
    compare(ms, "digraph {\n"
               "\t0[address=\"York\", name=\"Duke\"]; 1[address=\"Medici\", name=\"Lorenzo\"]; \n"
               "\t0->1[address=\"Bismarck\", name=\"Otto\"]; \n"
               "}\n");
    // user provided formatter has the highest priority
    auto otto = [](const std::map<std::string, std::string>& p) {
        return "person=\"" + p.at("name") + " von " + p.at("address") + "\"";
    };
    ms.register_edge_formatter(otto);
    compare(ms, "digraph {\n"
               "\t0[address=\"York\", name=\"Duke\"]; 1[address=\"Medici\", name=\"Lorenzo\"]; \n"
               "\t0->1[person=\"Otto von Bismarck\"]; \n"
               "}\n");
    graph_with_prop<Empty, void> empty_node;
    Serializer empty_node_s{empty_node};
    ASSERT_ANY_THROW(empty_node_s.serialize_to_dot(std::cout));
    graph_with_prop<void, Empty> empty_edge;
    Serializer empty_edge_s{empty_edge};
    ASSERT_ANY_THROW(empty_edge_s.serialize_to_dot(std::cout));
}