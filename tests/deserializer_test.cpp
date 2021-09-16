#include <gtest/gtest.h>
#include <graph_lite.h>
#include <deserialize.h>
#include <serialize.h>

const std::string test_path = "../test_files/";

TEST(test_deserializer, simple) {
    dot_parser::parse("digraph {\n{A B}->C\n}");
    std::stringstream ss;
    // minimal
    ss << "graph {1; 2; 3}";
    graph_lite::Deserializer ds_default;
    ASSERT_ANY_THROW(ds_default.deserialize_from_dot(ss));  // input is not strict
    ss << "strict graph{1; 2; 3}";
    auto g_0 = ds_default.deserialize_from_dot(ss);
    ASSERT_EQ(g_0.size(), 3);
    // param validity
    std::string s = "strict digraph{1}";
    ss << s;
    ASSERT_ANY_THROW(ds_default.deserialize_from_dot(ss));  // input is directed
    graph_lite::Deserializer<int, void, void, graph_lite::EdgeDirection::DIRECTED> ds_directed;
    ss << s;
    auto g_1 = ds_directed.deserialize_from_dot(ss);
    ASSERT_EQ(g_1.size(), 1);
}

TEST(test_serializer, test_0) {
    std::string test_file = test_path + "test_0.dot";
    // when prop type is already map-like
    using UM = std::unordered_map<std::string, std::string>;
    using M = std::map<std::string, std::string>;
    graph_lite::Deserializer<std::string, UM, UM, graph_lite::EdgeDirection::DIRECTED,
                             graph_lite::MultiEdge::ALLOWED, graph_lite::SelfLoop::ALLOWED,
                             graph_lite::Map::MAP, graph_lite::Container::VEC> ds_dg_map;
    auto g_0_map = ds_dg_map.deserialize_from_dot_file(test_file);
    ASSERT_EQ(g_0_map.num_edges(), 17);
    ASSERT_EQ(g_0_map.node_prop("G").at("color"), "rouge");
    ASSERT_EQ(g_0_map.count_in_neighbors("G"), 5);  // EFGGG
    ASSERT_EQ(g_0_map.count_out_neighbors("G"), 3);  // GGG
    // custom converter
    graph_lite::Deserializer<std::string, std::string, std::string, graph_lite::EdgeDirection::DIRECTED,
                             graph_lite::MultiEdge::ALLOWED, graph_lite::SelfLoop::ALLOWED,
                             graph_lite::Map::MAP, graph_lite::Container::VEC> ds_dg_conv;
    auto converter = [](const M& parsed_pairs) {
        const auto& parsed = parsed_pairs.at("color");
        if (parsed=="rouge") { return "red"; }
        else if (parsed=="blanc") { return "white"; }
        else { return "black"; }
    };
    ds_dg_conv.register_node_prop_converter(converter);
    ds_dg_conv.register_edge_prop_converter(converter);
    ds_dg_conv.register_node_name_converter([](const std::string& name){ return "\"" + name + "\""; });
    static_assert(std::is_constructible_v<std::string, typename M::const_iterator, typename M::const_iterator>);
    auto g_0_conv = ds_dg_conv.deserialize_from_dot_file(test_file);
    graph_lite::Serializer s_0_conv{g_0_conv};
    auto fmt = [](const std::string& prop) { return "color="+prop; };
    s_0_conv.register_node_formatter(fmt);
    s_0_conv.register_edge_formatter(fmt);
    std::stringstream ss;
    s_0_conv.serialize_to_dot(ss);
    ASSERT_EQ(ss.str(), "digraph {\n"
              "\t\"A\"[color=white]; \"B\"[color=black]; \"C\"[color=black]; \"D\"[color=black]; \"E\"[color=black]; \"F\"[color=black]; \"G\"[color=red]; \"X\"[color=black]; \n"
              "\t\"A\"->\"C\"[color=black]; \"A\"->\"D\"[color=black]; \"A\"->\"X\"[color=black]; \"A\"->\"X\"[color=black]; \"C\"->\"E\"[color=black]; \"C\"->\"F\"[color=black]; \"D\"->\"E\"[color=black]; \"D\"->\"F\"[color=black]; \"X\"->\"A\"[color=black]; \"X\"->\"A\"[color=black]; \"E\"->\"G\"[color=black]; \"F\"->\"G\"[color=black]; \"G\"->\"G\"[color=black]; \"G\"->\"G\"[color=black]; \"G\"->\"G\"[color=black]; \"B\"->\"C\"[color=black]; \"B\"->\"D\"[color=black]; \n"
              "}\n");
    // the absence of converter results in a runtime failure
    ds_dg_conv.delete_node_name_converter();
    ds_dg_conv.delete_edge_prop_converter();
    ASSERT_ANY_THROW(ds_dg_conv.deserialize_from_dot_file(test_file));  // failed to convert edge prop
    ds_dg_conv.register_edge_prop_converter(converter);
    ds_dg_conv.delete_node_prop_converter();
    ASSERT_ANY_THROW(ds_dg_conv.deserialize_from_dot_file(test_file));  // failed to convert node prop
    // can also discard properties by setting void
    graph_lite::Deserializer<std::string, void, void, graph_lite::EdgeDirection::DIRECTED,
                             graph_lite::MultiEdge::ALLOWED, graph_lite::SelfLoop::ALLOWED,
                             graph_lite::Map::MAP, graph_lite::Container::VEC> ds_0;
    auto g_0 = ds_0.deserialize_from_dot_file(test_file);
    ASSERT_EQ(g_0.num_edges(), 17);  // basically the same graph, minus the properties
}

