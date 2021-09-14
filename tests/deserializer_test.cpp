#include <gtest/gtest.h>
#include <graph_lite.h>
#include <deserialize.h>


TEST(test_deserializer, simple) {
    std::stringstream ss;
    ss << "graph {1; 2; 3}";
    graph_lite::Deserializer ds;
    auto g = ds.deserialize_from_dot(ss);
    for (const auto& [node, nbrs]: g) {
        std::cout << node << ' ';
    }
}