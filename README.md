# Introduction
Graphlite is a lightweight generic graph library that supports 
- node properties
- edge properties
- directed/undirected graphs
- multi-edges & self-loops
- user-specified data structures for the adjacency list and neighbor containers

There are a number of C++ graph libraries in the marketplace, among which the Boost Graph Library(BGL) is the best-known. The downside of the BGL is that even though it's header-only, it still has Boost as its dependency. This library, being a generic header-only graph library, is very much inspired by the BGL, except that it is completely self-contained and all the functionality is in one singular header file.

Note that unlike the BGL, this library offers the graph data structure but not the various graph algorithms. The data structure alone can already prove useful for simple tasks that only require graph traversal. Even for tasks that require graph algorithms, the usage tends to be limited to one or two graph algorithms, which can be readily implemented on top of the data structure, and, even better, shared with others. 

This library requires C++ 17(or higher).

# Documentation
1. [Graph and its Iterators](#1-graph-and-its-iterators)

   - [Construction](#construction-of-a-graph)
   - [Node Iterators](#node-iterators)
   - [Neighbor Iterators](#neighbor-iterators)

2. [Queries](#2-queries)

    - [Find a node by value](#find-a-node-by-value)
    - [Test whether a node exists in the graph](#test-whether-a-node-exists-in-the-graph)
    - [Return size](#return-size)
    - [Count the number of edges between two nodes](#count-the-number-of-edges-between-two-nodes)
    - [Get the neighbors of a node](#get-the-neighbors-of-a-node)
    - [Count the neighbors of a node](#count-the-neighbors-of-a-node)
    - [Find a neighbor of a node by value](#find-a-neighbor-of-a-node-by-value)
    - [Get node property](#get-node-property)
    - [Get edge property](#get-edge-property)

3. [Graph Editing](#3-graph-editing)

    - [Add nodes to the graph](#add-nodes-to-the-graph)
    - [Remove nodes from the graph](#remove-nodes-from-the-graph)
    - [Add edges to the graph](#add-edges-to-the-graph)
    - [Remove edges from the graph](#remove-edges-from-the-graph)
        - [Remove all](#1-remove-all-edges-between-an-ordered-pair-of-nodes)
        - [Remove one](#2-remove-one-edge-from-the-graph-by-position)

## 1. Graph and its Iterators
### Construction of a `Graph`
We start with the template parameters of the `Graph` class.
```c++
template<typename NodeType=int, typename NodePropType=void, typename EdgePropType=void,
        EdgeDirection direction=EdgeDirection::UNDIRECTED,
        MultiEdge multi_edge=MultiEdge::DISALLOWED,
        SelfLoop self_loop=SelfLoop::DISALLOWED,
        Map adj_list_spec=Map::UNORDERED_MAP,
        Container neighbors_container_spec=Container::UNORDERED_SET>
class Graph;
```
- `NodeType`
Type of nodes in the graph. Must not be cv-qualified or a reference. Must support `operator<<` and `operator==`. Must support `operator<` if adjacency list is a `std::map`. Must support hashing if adjacency list is a `std::unorder_map`
- `NodePropType`
Type of node properties. `void` if not needed. Must not be cv-qualified or a reference
- `EdgePropType`
Type of edge properties. `void` if not needed. Must not be cv-qualified or a reference
- `EdgeDirection direction`
Non-type template parameter that takes value from the enum below. Determines if the graph should be directed or undirected
```c++
enum class EdgeDirection { DIRECTED, UNDIRECTED };
```
- `MultiEdge multi_edge`
Non-type template parameter that takes value from the enum below. Determines if multi-edges are allowed. If `ALLOWED`, no runtime check for multi-edges will happen during edge addition. Otherwise, runtime check will occur and edge addition will fail if the edge is already present
```c++
enum class MultiEdge { ALLOWED, DISALLOWED };
```
- `SelfLoop self_loop`
Non-type template parameter that takes value from the enum below. Determines if self-loops are allowed. If `ALLOWED`, no runtime check for self-loops will happen during edge addition. Otherwise, runtime check will occur and edge addition will fail if the edge being added is a self-loop
```c++
enum class SelfLoop { ALLOWED, DISALLOWED };
```
- `Map adj_list_spec` 
Non-type template parameter that takes value from the enum below. Determines the data structure of the adjacency list
```c++
enum class Map { MAP, UNORDERED_MAP };
```
- `Container neighbors_container_spec`
Non-type template parameter that takes value from the enum below. Determines the data structure in which the neighbors of each node should be stored
    - if `multi_edge` is `DISALLOWED`, `MULTISET` and `UNORDERED_MULTISET` are not supported
    - if `multi_edge` is `ALLOWED`, `SET` and `UNORDERED_SET` are not supported
```c++
enum class Container { VEC, LIST, SET, UNORDERED_SET, MULTISET, UNORDERED_MULTISET };
```

*Examples*
```c++
//(default)undirected simple graph using hashtable for adj list and hashset for neighbors
Graph g_0;
// directed graph with edge weights, using rb tree for adj list and vector for neighbors
Graph<std::string, void, double, 
      EdgeDirection::DIRECTED, MultiEdge::DISALLOWED, SelfLoop::DISALLOWED, 
      Map::MAP, Container::VEC> g_1;
```

For the rest of the documentation we refer to the full type(with template params) of `Graph` as `GType`.

### Node Iterators
Much like the STL containers, `Graph` also has custom iterators. The iterator types can be accessed as 
```c++
typename GType::Iterator non_const_it;
typename GType::ConstIterator const_it;
```
Obviously, default-constructed iterators are rarely useful. The begin and end iterators can be accessed using the following
```c++
Iterator begin() noexcept;
Iterator end() noexcept;
ConstIterator begin() const noexcept;
ConstIterator end() const noexcept;
ConstIterator cbegin() noexcept;
ConstIterator cend() noexcept;
```
which enables the following loops for graph traversal
```c++
for (auto it=g.begin(); it!=g.end(); ++it) {
    // classic for loop
}
for (auto&& v: g) {
    // range-based for loop
}
```
Neither of the above will be very useful without knowing how the iterators are dereferenced through `operator*`. The return value of `operator*` depends on (1) constness of the iterator (2) whether node prop is needed (3) whether the graph is directed or undirected. `*it` gives a 3-tuple `(node, node_prop, neighbors_view)` if node prop is needed, or a 2-tuple `(node, neighbors_view)` if node prop is not needed. 

*Examples*
```c++
// NodePropType is void
Graph<int> g; 
for (auto&& [node, neighbors_view]: g) {
    // node has type const int& for the same reason why map keys are const references
}

// NodePropType is double
Graph<int, double> g_non_const;  
for (auto&& [node, node_prop, neighbors_view]: g_non_const) {
    // node has type const int&
    // node_prop has type double&
    node_prop = 0.0; // zero out all node prop
}

// g_const is const
const Graph<int, double>& g_const = g_non_const;
for (auto&& [node, node_prop, neighbors_view]: g_const) {
    // node has type const int&
    // node_prop has type const double& 
    // node_prop = 0.0; WON'T COMPILE! node_prop is const ref
}
```
Of course, we don't have to use structured binding. After all, it's just a syntax sugar for 
```c++
for (auto it=g_non_const.begin(); it!=g_non_const.end(); ++it) {
    std::get<1>(*it) = 0.0; // zero out all node prop
}
for (auto it=g_const.begin(); it!=g_const.end(); ++it) {
    // std::get<1>(*it) = 0.0; WON'T COMPILE! assigning to const ref
}
```

### Neighbor Iterators
You might have noticed that we haven't talked about the type of `neighbors_view`, which is what we're about to do now. 
Just as `Iterator/ConstIterator` iterates through the adjacency list, 
```c++
typename GType::NeighborsConstIterator const_nbr_it;
typename GType::NeighborsIterator nbr_it;
```
iterates through the neighbors of each node. 

- If `EdgePropType` is `void`, both `*const_nbr_it` and `*nbr_it` have type `const NodeType&`
- If `EdgePropType` is not `void`, both `*const_nbr_it` and `*nbr_it` have type `std::pair`. The first fields are both `const NodeType`. The second fields exposes a public method
```c++
const EdgePropType& prop() const  // called by const_nbr_it->second.prop()
EdgePropType& prop()  // called by nbr_it->second.prop()
```

*Examples*

the following code snippet zeros out the edge property if the end node has value 42
```c++
if (nbr_it->first == 42) {
    nbr_it->second.prop() = 0.0;
}
```

Instead of returning a reference to the actual neighbor container, this library always returns a pair of neighbor iterators, also known as a `NeighborsView`, defined below
```c++
using NeighborsView = std::pair<NeighborsIterator, NeighborsIterator>;
using NeighborsConstView = std::pair<NeighborsConstIterator,
                                     NeighborsConstIterator>;
```

For `UNDIRECTED` graphs, `NeighborsView` and `NeighborsConstView` are exactly the types of `neighbors_view` from the previous section. 

However, for `DIRECTED` graphs, a node can have both in-neighbors and out-neighbors. If we have two nodes, u, v, and a directed edge (u -> v), then u is an in-neighbor of v and v is an out-neighbor of u. To describe the two types of neighbors, we need two neighbor views instead of one, using the following template
```c++
template<typename T>
struct OutIn {
    T out;
    T in;
}
```
`OutIn<NeighborsView>` and `OutIn<NeighborsConstView>` are the type of `neighbors_view` for directed graphs. Indeed, using another `std::pair` could work, but memorizing which is first and which is second is a non-trivial task. 

*Examples*
```c++
// graphs with (1) no node property (2) edge property with type double
Graph<int, void, double, EdgeDirection::UNDIRECTED> udg;
for (auto&& [node, neighbors_view]: udg) {
    // neighbors_view has type NeighborsView
    auto [nbr_begin, nbr_end] = neighbors_view;
    for (auto nbr_it=nbr_begin; nbr_it!=nbr_end; ++nbr_it) {
        // zero out edge property for edge (node, 42)
        if (nbr_it->first==42) {
            nbr_it->second.prop() = 0.0;
        }
    }
}

// edge property is immutable for a const graph
const auto& const_udg = udg;
for (auto&& [node, neighbors_view]: udg) {
    // neighbors_view has type NeighborsConstView
    // nbr_it->second.prop() has type const double& 
    auto [nbr_begin, nbr_end] = neighbors_view;
    for (auto nbr_it=nbr_begin; nbr_it!=nbr_end; ++nbr_it) {
        if (nbr_it->first==42) {
            // nbr_it->second.prop() = 0.0; WON'T COMPILE!
        }
    }
}

// directed graphs requires more neighbor information
Graph<int, void, double, EdgeDirection::DIRECTED> dg;
for (auto&& [node, neighbors_view]: udg) {
    // neighbors_view has type OutIn<NeighborsView>
    auto [out_nbr_begin, out_nbr_end] = neighbors_view.out;
    auto [in_nbr_begin, in_nbr_end] = neighbors_view.in;
    for (auto out_nbr_it=out_nbr_begin; 
              out_nbr_it!=out_nbr_end; ++out_nbr_it) {
        // zero out edge property for edge (node -> 42)
        if (out_nbr_it->first==42) {
            out_nbr_it->second.prop() = 0.0;
        }
    }
    for (auto in_nbr_it=in_nbr_begin; 
              in_nbr_it!=in_nbr_end; ++in_nbr_it) {
        // zero out edge property for edge (node <- 24)
        if (in_nbr_it->first==24) {
            in_nbr_it->second.prop() = 0.0;
        }
    }
}
```

### Invalidation Rules for Iterators
Both node and neighbor iterators follow the iterator invalidation rules of their underlying STL containers, as summarized by [this SO answer](https://stackoverflow.com/a/6442829).

For example
- If `Map::UNORDERED_MAP` is chosen, existing node iterators may be invalidated after adding or removing nodes due to rehashing
- If `Container::SET` is chosen, existing neighbor iterators will not be invalidated by the addition/removal of (other) edges
- Modifying node/edge properties never invalidates existing iterators


## 2. Queries
### Find a node by value
```c++
ConstIterator find(const T& node_identifier) const noexcept;
Iterator find(const T& node_identifier) noexcept;
```

Searches the graph for a node with value equivalent to `node_identifier` and returns an iterator to it if found; otherwise it returns Graph::end().

`node_identifier` must be implicitly or explicitly convertible to `NodeType`.

*Examples*
```c++
Graph g;
typename Graph::Iterator pos = g.find(123);
const Graph& cg = g;
typename Graph::ConstIterator cpos = cg.find(123);
```

### Test whether a node exists in the graph
```c++
bool has_node(const T& node_identifier) const noexcept;
```

Returns true if the node is in the graph. `node_identifier` must be implicitly or explicitly convertible to `NodeType`.

### Return size
```c++
size_t size() const noexcept;
```

Returns the number of nodes in the graph.

### Count the number of edges between two nodes
```c++
int count_edges(const U& source_iv, const V& target_iv) const noexcept;
```
Returns the number of edges going out of `source_iv` and into `target_iv`. 

Returns 0 if either `source_iv` or `target_iv` doesn't represent an existing node.

Each of `source_iv` and `target_iv` can be either an iterator or a value. 

*Examples*
```c++
Graph g;
auto pos_0 = g.find(0);
auto pos_1 = g.find(1);
// these four do the same thing
g.count_edges(0, 1);
g.count_edges(pos_0, pos_1);
g.count_edges(0, pos_1);
g.count_edges(pos_0, 1);
```

### Get the neighbors of a node
#### *(1) Exclusive to **undirected** graphs*
```c++
NeighborsConstView neighbors(const T& node_iv) const;
NeighborsView neighbors(const T& node_iv);
```
Returns a pair of iterators marking the beginning and the end of the neighbors of `node_iv`. 

`node_iv` can be either an iterator or a value. 

An exception will be thrown if `node_iv` does not represent an existing node. 

#### *(2) Exclusive to **directed** graphs*
```c++
NeighborsConstView out_neighbors(const T& node_iv) const;
NeighborsView out_neighbors(const T& node_iv);
NeighborsConstView in_neighbors(const T& node_iv) const;
NeighborsView in_neighbors(const T& node_iv);
```
Returns a pair of iterators marking the beginning and the end of out/in-neighbors of `node_iv`.

`node_iv` can be either an iterator or a value. 

An exception will be thrown if `node_iv` does not represent an existing node. 

*Examples*
```c++
Graph<int, void, void, EdgeDirection::UNDIRECTED> udg;
auto [nbr_begin, nbr_end] = udg.neighbors(123);

Graph<int, void, void, EdgeDirection::DIRECTED> dg;
auto pos_123 = dg.find(123);
auto [out_nbr_begin, out_nbr_end] = dg.out_neighbors(pos_123);
auto [in_nbr_begin, in_nbr_end] = dg.in_neighbors(pos_123);
```

### Count the neighbors of a node
#### *(1) Exclusive to **undirected** graphs*
```c++
int count_neighbors(const T& node_iv) const;
```
Returns the number of neighbors of `node_iv`. 

An exception will be thrown if `node_iv` does not represent an existing node. 

#### *(2) Exclusive to **directed** graphs*
```c++
int count_out_neighbors(const T& node_iv) const;
int count_in_neighbors(const T& node_iv) const;
```
Returns the number of out/in-neighbors of `node_iv`. 

An exception will be thrown if `node_iv` does not represent an existing node. 

> Almost always more efficient than calling std::distance on the iterator pair.

### Find a neighbor of a node by value
#### *(1) Exclusive to **undirected** graphs*
```c++
std::pair<bool, NeighborsConstIterator> 
find_neighbor(const U& src_iv, const V& tgt_identifier) const;

std::pair<bool, NeighborsIterator> 
find_neighbor(const U& src_iv, const V& tgt_identifier);
```
Returns a pair (is_found, neighbor_pos). The first field is true if a node with value equivalent to `tgt_identifier` is found within the neighborhood of `src_iv`. The second field is an iterator pointing to the neighbor if found at all. 

`src_iv` can be either an iterator or a value. `tgt_identifier` must be implicitly or explicitly convertible to `NodeType`. 

An exception will be thrown if `src_iv` does not represent an existing node. 

#### *(2) Exclusive to **directed** graphs*
```c++
std::pair<bool, NeighborsConstIterator> 
find_out_neighbor(const U& src_iv, const V& tgt_identifier) const;

std::pair<bool, NeighborsIterator> 
find_out_neighbor(const U& src_iv, const V& tgt_identifier);

std::pair<bool, NeighborsConstIterator> 
find_in_neighbor(const U& src_iv, const V& tgt_identifier) const;

std::pair<bool, NeighborsIterator> 
find_in_neighbor(const U& src_iv, const V& tgt_identifier);
```

Similar to its undirected counterpart, except that the search for `tgt_identifier` will be carried out in the out/in-neighborhood of `src_iv`. 

An exception will be thrown if `src_iv` does not represent an existing node. 

*Examples*
```c++
Graph<int, void, void, EdgeDirection::UNDIRECTED> udg;
auto [found_0_1, pos_0_1] = udg.find_neighbor(0, 1);

Graph<int, void, void, EdgeDirection::DIRECTED> dg;
auto pos_1 = dg.find(1);
auto [found_1_0, pos_1_0] = dg.find_out_neighbor(pos_1, 0);
```

### Get node property
*Exclusive to graphs **with node properties***
```c++
const NodePropType& node_prop(const T& node_iv) const;
NodePropType& node_prop(const T& node_iv);
```

`node_iv` can be either an iterator or a value.

An exception will be thrown if `node_iv` does not represent an existing node. 

*Examples*
```c++
Graph<std::string, int> g;
std::cout << "Alice's age: " << g.node_prop("Alice") << "\n";

auto bob_pos = g.find("Bob");
std::cout << "Bob's age: " << g.node_prop(bob_pos) << "\n";

// Bob is actually 20
g.node_prop(bob_pos) = 20;
// Bob just had his birthday
g.node_prop(bob_pos)++;

const auto& cg = g;
// cg.node_prop("Alice") = 99;  // WON'T COMPILE! Assigning to const ref
```

### Get edge property
*Exclusive to graphs **with edge properties***
```c++
const EdgePropType& edge_prop(U&& source_iv, V&& target_iv) const;
EdgePropType& edge_prop(U&& source_iv, V&& target_iv);
```
Each of `source_iv` and `target_iv` can be either an iterator or a value.

An exception will be thrown if `(source_iv, target_iv)` does not represent an existing edge. 

```c++
Graph<std::string, void, double> g;
auto beijing_pos = g.find("Beijing");
auto shanghai_pos = g.find("Shanghai");
// the many, many ways to retrieve the commuting hours between the two cities
LOG(g.edge_prop(beijing_pos, shanghai_pos));
LOG(g.edge_prop(beijing_pos, "Shanghai"));
LOG(g.edge_prop("Beijing", shanghai_pos));
LOG(g.edge_prop("Beijing", "Shanghai"));

auto [found, b2s_edge] = g.find_neighbor(beijing_pos, "Shanghai");
assert(found);
assert(b2s_edge->first=="Shanghai");
LOG(b2s_edge->second.prop());

// with the high-speed railway, now it takes only 4.5h
g.edge_prop("Beijing", "Shanghai") = 4.5;
// or, equivalently
b2s_edge->second.prop() = 4.5;
```

## 3. Graph Editing
### Add nodes to the graph
*(1) Exclusive to graphs **without node properties***
```c++
int add_nodes(T&& new_node, Args&&... args) noexcept;
```
Adds one or more nodes to the graph. 

Re-adding an existing node results in an no-op. Returns the number of nodes actually added. 

`new_node` must have the same type as `NodeType`; this is to prevent unwanted implicit conversion during node addition. 

*(2) Exclusive to graphs **with node properties***
```c++
int add_node_with_prop(NT&& new_node, NPT&& prop) noexcept;
```
Adds a node with property to the graph. 

Re-adding an existing node results in an no-op; in particular, the property of the existing node will **not** be changed. Returns the number of nodes actually added, in this case 0 or 1.  

`new_node` must have the same type as `NodeType`. `prop` must be implicitly or explicitly convertible to `NodePropType`. 

*Examples*
```c++
Graph g;
assert(g.add_nodes(2, 3, 5, 7, 11) == 5);

Graph<std::string, int> sg;
sg.add_node_with_prop(std::string("Alice"), 21);
sg.add_node_with_prop(std::string("Bob"), 22);
// same_type requirement
// sg.add_node_with_prop("NO", 0);  // WON'T COMPILE! 
// "No" has type const char*, not std::string
```

### Remove nodes from the graph
```c++
int remove_nodes(const T& node_iv, const Args&... args) noexcept;
```
Removes one or more nodes from the graph, as well as the edges involving the nodes being removed. 

Removing a non-existent node results in a no-op. Returns the number of nodes actually removed. 

`node_iv` can be either an iterator or a value. 

>Caveat: Nodes are removed in the order they appear in the parameter list. Removing a node invalidates iterators to the node removed.

*Examples*
```c++
Graph<std::string> g;
g.add_nodes(std::string("Alice"), std::string("Bob"), std::string("Cyrus"));
assert(g.remove_nodes("Alice", "Derek") == 1);
// Bad idea!!! After the removal of "Bob", bob_pos becomes invalidated!
auto bob_pos = g.find("Bob");
g.remove_nodes("Bob", bob_pos);  // just don't
```

### Add edges to the graph
*(1) Exclusive to graphs **without edge properties***
```c++
int add_edge(U&& source_iv, V&& target_iv) noexcept;
```
Adds an edge to the graph. 

The following attempts to add an edge will result in a no-op:
- Either `source_iv` or `target_iv` not representing an existing node in the graph
- Re-adding an existing edge when `multi_edge` is `DISALLOWED` 
- Adding a self-loop when `self_loop` is `DISALLOWED`

Each of `source_iv` and `target_iv` can be either an iterator or a value. 

Returns the number of edges actually added, in this case 0 or 1.

*(2) Exclusive to graphs **with edge properties***
```c++
int add_edge_with_prop(U&& source_iv, V&& target_iv, EPT&& prop) noexcept;
```
Adds an edge with property to the graph.

The following attempts to add an edge will result in a no-op:
- Either `source_iv` or `target_iv` not representing an existing node in the graph
- Re-adding an existing edge when `multi_edge` is `DISALLOWED` 
- Adding a self-loop when `self_loop` is `DISALLOWED`

Each of `source_iv` and `target_iv` can be either an iterator or a value. `prop` must be implicitly or explicitly convertible to `EdgePropType`.

Returns the number of edges actually added, in this case 0 or 1.

*Examples*
```c++
Graph g;
g.add_nodes(1, 2, 3);
g.add_edge(1, 3);
auto pos_2 = g.find(2);
g.add_edge(1, pos_2);
g.add_edge(pos_2, 3);

Graph<std::string, void, double> sg;
g.add_nodes(std::string("NY"), std::string("LA"));
g.add_edge_with_prop("NY", "LA", 2 * 24 + 19);
```

### Remove edges from the graph
#### *(1) Remove all edges between an ordered pair of nodes*
```c++
int remove_edge(const U& source_iv, const V& target_iv) noexcept;
```
Removes all edges with the form `(source_iv, target_iv)` from the graph.

Each of `source_iv` and `target_iv` can be either an iterator or a value.

Results in an no-op if
- either `source_iv` or `target_iv` does not represent an existing node
- `(source_iv, target_iv)` does not represent an existing edge

Returns the number of edges actually removed. 0 or 1 if `multi_edge` is `DISALLOWED`; [0, +\infty) if `multi_edge` is `ALLOWED`.

#### *(2) Remove one edge from the graph by position*
```c++
int remove_edge(ConstIterator source_pos, NeighborsConstIterator target_nbr_pos) noexcept;
```
Removes exactly one edge by position. 

Especially useful when `multi_edge` is `ALLOWED` and a particular edge among the many parallel edges needs to be removed. 

Always returns 1.

*Examples*
```c++
struct Travel {
    std::string means;
    double hours;
}
int main() {
    Graph<std::string, void, Travel, EdgeDirection::UNDIRECTED, 
          MultiEdge::ALLOWED, SelfLoop::DISALLOWED, Map::MAP, Container::MULTISET> us;
    us.add_nodes(std::string("NY"), std::string("LA"));
    us.add_edge_with_prop("NY", "LA", Travel{"walk", 912});
    us.add_edge_with_prop("NY", "LA", Travel{"bike", 244});
    us.add_edge_with_prop("NY", "LA", Travel{"car", 41});
    us.add_edge_with_prop("NY", "LA", Travel{"plane", 3});
    assert(us.count_edges("NY", "LA") == 4);
    // walking simply doesn't make much sense;
    auto ny_pos = us.find("NY");
    auto [n_begin, n_end] = us.neighbors(ny_pos);  
    // to remove it we must find it first
    auto walk_pos = std::find_if(n_begin, n_end, 
                                 [](const auto& p){ 
                                     // p is a std::pair
                                     const Travel& travel = p.second.prop();
                                     return travel.means == "walk"; 
                                 });
    us.remove_edge(ny_pos, walk_pos);  // calling (2)
    assert(us.count_edges(ny_pos, "LA") == 3);
    // now suppose a lockdown happens and all forms of travelling are banned
    int num_removed = us.remove_edge(ny_pos, "LA");  // calling (1)
    assert(num_removed == 3);
    assert(us.count_edges("NY", "LA") == 0);  // confirm removal
}
```
