#ifndef GRAPHLITE_DESERIALIZE_H
#define GRAPHLITE_DESERIALIZE_H

#include <parser.hpp>
#include <resolver.hpp>
#include <graph_lite.h>

#include <cassert>
#include <iostream>
#include <variant>

namespace graph_lite {
    namespace detail {
        template<typename PT>
        struct converter {
            std::optional<std::function<PT(const std::map<std::string, std::string>&)>> conv;
        };
    }

    template<typename NodeType=int, typename NodePropType=void, typename EdgePropType=void,
            EdgeDirection direction=EdgeDirection::UNDIRECTED,
            MultiEdge multi_edge=MultiEdge::DISALLOWED,
            SelfLoop self_loop=SelfLoop::DISALLOWED,
            Map adj_list_spec=Map::UNORDERED_MAP,
            Container neighbors_container_spec=Container::UNORDERED_SET>
    class Deserializer {
    public:
        using GType = Graph<NodeType, NodePropType, EdgePropType, direction, multi_edge, self_loop, adj_list_spec, neighbors_container_spec>;
    private:
        using NS = dot_parser::detail::node_stmt_v;
        using ES = dot_parser::detail::edge_stmt_v;

        using M = std::map<std::string, std::string>;

        void check_validity(const dot_parser::dot_graph_flat& g) const {
            if ((multi_edge==MultiEdge::ALLOWED) != g.is_strict) {
                throw std::runtime_error("inconsistent graph strictness; "
                                         "make sure to disallow multi-edges for strict graphs/allow multi-edges for non-strict graphs");
            }
            if ((direction==EdgeDirection::UNDIRECTED) != (g.graph_type=="graph")) {
                throw std::runtime_error("inconsistent edge direction; "
                                         "make sure to set UNDIRECTED for graphs/DIRECTED for digraphs");
            }
        }

        template<typename PT>
        static constexpr bool is_convertible_from_map = std::is_constructible_v<PT, typename M::const_iterator, typename M::const_iterator>;

        enum class prop_converter_resolv {
            VOID, USER_DEF, MAP_CONV
        };

        template<bool is_node>
        prop_converter_resolv resolve_prop_converter() const {
            using PT = std::conditional_t<is_node, NodePropType, EdgePropType>;
            std::string node_or_edge = is_node ? "node" : "edge";
            const detail::converter<PT>& conv = [this]() -> auto& {
                if constexpr(is_node) {
                    return node_conv;
                } else {
                    return edge_conv;
                }
            }();
            // case by case
            if constexpr(std::is_void_v<PT>) {
                std::cerr << node_or_edge << " property is void; ignoring ALL parsed attrs\n";
                return prop_converter_resolv::VOID;
            } else if (conv.conv.has_value()) {
                std::cerr << "using " << node_or_edge << " converter provided\n";
                return prop_converter_resolv::USER_DEF;
            } else if constexpr(is_convertible_from_map<PT>) {
                std::cerr << node_or_edge << " prop can be directly converted from parsed string pairs; converting\n";
                return prop_converter_resolv::MAP_CONV;
            } else {
                throw std::runtime_error("failed to resolve " + node_or_edge + " property converter");
            }
        }

        std::function<NodeType(const std::string&)> resolve_node_name_converter() const {
            if (node_name_conv.has_value()) {
                std::cerr << "using node name converter provided\n";
                return [this](const std::string& node_name) -> NodeType {
                    return node_name_conv.value()(node_name);
                };
            } else if constexpr(std::is_constructible_v<NodeType, std::string>) {
                std::cerr << "NodeType can be implicitly/explicitly converted from the parsed string\n";
                return [](const std::string& node_name) -> NodeType {
                    return static_cast<NodeType>(node_name);
                };
            } else if constexpr(std::is_integral_v<NodeType>) {
                std::cerr << "NodeType is an integral type; using std::stoi for conversion\n";
                return [](const std::string& node_name) -> NodeType {
                    return std::stoi(node_name);
                };
            } else {
                throw std::runtime_error("failed to resolve node name converter");
            }
        }

        GType deserialize_impl(const dot_parser::dot_graph_raw& raw_graph) const {
            auto flat_graph = dot_parser::flatten(dot_parser::resolve(raw_graph));
            auto convert_to_node_type = resolve_node_name_converter();
            prop_converter_resolv node_resolv = resolve_prop_converter<true>();
            prop_converter_resolv edge_resolv = resolve_prop_converter<false>();
            check_validity(flat_graph);
            // start constructing graph
            GType g;
            for (const auto& statement: flat_graph.statements) {
                if (std::holds_alternative<NS>(statement)) {
                    const NS& ns = std::get<NS>(statement);
                    NodeType node = convert_to_node_type(ns.node_name);
                    if constexpr(std::is_void_v<NodePropType>) {
                        g.add_nodes(std::move(node));
                    } else {
                        static_assert(node_resolv!=prop_converter_resolv::VOID);
                        M attrs {ns.attrs.begin(), ns.attrs.end()};
                        if (node_resolv==prop_converter_resolv::MAP_CONV) {
                            // constructed from parsed map
                            g.add_node_with_prop(std::move(node),
                                                 attrs.begin(), attrs.end());
                        } else {
                            // user-defined converter
                            assert(node_resolv==prop_converter_resolv::USER_DEF);
                            g.add_node_with_prop(std::move(node),
                                                 node_conv.conv.value()(attrs));
                        }

                    }
                } else {
                    assert(std::holds_alternative<ES>(statement));
                    const ES& es = std::get<ES>(statement);
                    M attrs {es.attrs.begin(), es.attrs.end()};  // the same attrs for all edges
                    for (const dot_parser::edge& e: es.edges) {
                        NodeType src = convert_to_node_type(e.src);
                        NodeType tgt = convert_to_node_type(e.tgt);
                        if constexpr(std::is_void_v<EdgePropType>) {
                            g.add_edge(std::move(src), std::move(tgt));
                        } else {
                            static_assert(edge_resolv!=prop_converter_resolv::VOID);
                            if (edge_resolv==prop_converter_resolv::MAP_CONV) {
                                // constructed from parsed map
                                g.add_edge_with_prop(std::move(src), std::move(tgt),
                                                     attrs.begin(), attrs.end());
                            } else {
                                // user-defined converter
                                assert(node_resolv==prop_converter_resolv::USER_DEF);
                                g.add_edge_with_prop(std::move(src), std::move(tgt),
                                                     edge_conv.conv.value()(attrs));
                            }
                        }
                    }
                }
            }
            return g;
        }

    private:
        std::optional<std::function<NodeType(std::string)>> node_name_conv;
        detail::converter<NodePropType> node_conv;
        detail::converter<EdgePropType> edge_conv;
    public:
        template<typename F>
        void register_node_name_converter(F f) {
            static_assert(std::is_assignable_v<decltype(node_name_conv), F>);
            node_name_conv = f;
        }
        void delete_node_name_converter() {
            node_name_conv = std::nullopt;
        }

        template<typename F>
        void register_node_prop_converter(F f) {
            static_assert(not std::is_void_v<NodePropType>, "no node prop needed at all");
            static_assert(std::is_assignable_v<decltype(node_conv.conv), F>);
            node_conv.conv = f;
        }
        void delete_node_prop_converter() {
            static_assert(not std::is_void_v<NodePropType>, "no node prop needed at all");
            edge_conv.conv = std::nullopt;
        }

        template<typename F>
        void register_edge_prop_converter(F f) {
            static_assert(not std::is_void_v<EdgePropType>, "no edge prop needed at all");
            static_assert(std::is_assignable_v<decltype(edge_conv.conv), F>);
            edge_conv.conv = f;
        }
        void delete_edge_prop_formatter() {
            static_assert(not std::is_void_v<EdgePropType>, "no edge prop needed at all");
            edge_conv.conv = std::nullopt;
        }

        GType deserialize_from_dot_file(const std::string& path) const {
            auto raw_graph = dot_parser::parse_file(path);
            return deserialize_impl(raw_graph);
        }
        GType deserialize_from_dot(std::istream& is) const {
            std::string inp { std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>() };
            auto raw_graph = dot_parser::parse(inp);
            return deserialize_impl(raw_graph);
        }
    };

}


#endif //GRAPHLITE_DESERIALIZE_H
