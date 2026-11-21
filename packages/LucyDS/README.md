# LucyDS

**LucyDS** is a data structures and algorithms library written entirely in the **Lucy programming language**. It is designed to give Lucy developers reusable implementations of common collections, trees, graphs, searching routines, and sorting algorithms through one package import.

- **Language:** Lucy
- **Target runtime:** Lucy 2.0.7
- **Package type:** Pure-Lucy package
- **Native dependencies:** None
- **License:** GPL-3.0-only

> This README documents the public API exported by `packages/lucyds/src/init.lucy` in this repository. It describes the implementation currently included here, not planned or future features.

---

## Contents

- [Features](#features)
- [Requirements](#requirements)
- [Project structure](#project-structure)
- [Getting started](#getting-started)
- [Quick example](#quick-example)
- [API reference](#api-reference)
  - [LinkedList](#linkedlist)
  - [DoublyLinkedList](#doublylinkedlist)
  - [Stack](#stack)
  - [Queue](#queue)
  - [Deque](#deque)
  - [HashTable](#hashtable)
  - [BinarySearchTree](#binarysearchtree)
  - [AVLTree](#avltree)
  - [BinaryHeap](#binaryheap)
  - [Trie](#trie)
  - [Graph](#graph)
  - [UnionFind](#unionfind)
  - [Searching algorithms](#searching-algorithms)
  - [Sorting algorithms](#sorting-algorithms)
  - [Graph algorithms](#graph-algorithms)
- [Return values and edge cases](#return-values-and-edge-cases)
- [Complexity reference](#complexity-reference)
- [Run the tests](#run-the-tests)
- [Examples](#examples)
- [Current limitations](#current-limitations)
- [Contributing](#contributing)
- [License](#license)
- [Author](#author)

---

## Features

### Data structures

| Category | Available structures |
|---|---|
| Lists | Singly linked list, doubly linked list |
| Linear collections | Stack, queue, double-ended queue (deque) |
| Hashing | Hash table with separate chaining |
| Search trees | Binary search tree, AVL tree |
| Heap | Min binary heap |
| Prefix lookup | Trie |
| Graphs | Directed and undirected weighted graph |
| Disjoint sets | Union-Find (Disjoint Set Union) |

### Algorithms

- **Searching:** linear search, binary search, lower bound
- **Sorting:** bubble sort, selection sort, insertion sort, merge sort, quick sort
- **Graph traversal:** breadth-first search (BFS), depth-first search (DFS)
- **Shortest path:** Dijkstra's algorithm

All public classes and functions are re-exported from the `lucyds` package entry point.

---

## Requirements

- Lucy **2.0.7**
- A project directory containing a root `lucy.toml`
- No C++ compiler, native extension, or third-party package is required to use LucyDS

The package is implemented in Lucy source files. It follows the project-local package discovery layout documented for Lucy 2.0.7.

---

## Project structure

```text
LucyDS/
├── lucy.toml
├── LICENSE
├── README.md
├── docs/
│   └── API.md
├── examples/
│   └── basic_usage.lucy
├── packages/
│   └── lucyds/
│       ├── lucy.toml
│       └── src/
│           ├── init.lucy
│           ├── linked_list.lucy
│           ├── doubly_linked_list.lucy
│           ├── stack.lucy
│           ├── queue.lucy
│           ├── deque.lucy
│           ├── hash_table.lucy
│           ├── binary_search_tree.lucy
│           ├── avl_tree.lucy
│           ├── binary_heap.lucy
│           ├── trie.lucy
│           ├── graph.lucy
│           ├── union_find.lucy
│           ├── searching.lucy
│           ├── sorting.lucy
│           └── graph_algorithms.lucy
└── tests/
    └── test_lucyds.lucy
```

`packages/lucyds/src/init.lucy` is the package entry point. It imports and re-exports the public classes and functions from the implementation modules.

---

## Getting started

### 1. Get the project

Download or clone the LucyDS repository and extract it. Keep the root `lucy.toml` and the `packages/` directory together.

### 2. Create a Lucy source file inside the project

For example, create `hello_lucyds.lucy` in the LucyDS project root.

### 3. Import the package

```lucy
import lucyds

let numbers = lucyds.LinkedList.new()
numbers.append(10)
numbers.append(20)

println numbers.to_array()
```

You can also import individual public names:

```lucy
from lucyds import LinkedList, Stack, Queue
from lucyds import merge_sort, binary_search
```

### 4. Run your program from the project

From the LucyDS root directory:

```bash
lucy hello_lucyds.lucy
```

Lucy 2.0.7 discovers project-local packages by locating the nearest parent directory containing the root `lucy.toml`. It then adds that project's `packages/` directory to module lookup. For this reason, keep your application source inside the project tree when using this local package layout.

---

## Quick example

This example creates a list, uses a stack, sorts an array, and performs a binary search.

```lucy
import lucyds

# Linked list
let numbers = lucyds.LinkedList.new()
numbers.append(30)
numbers.append(10)
numbers.prepend(5)

println numbers.to_array()
# [5, 30, 10]

# Stack (last in, first out)
let tasks = lucyds.Stack.new()
tasks.push("compile")
tasks.push("test")

println tasks.pop()
# test

# Sorting returns a new array
let values = [8, 3, 5, 1]
let ordered = lucyds.merge_sort(values)

println ordered
# [1, 3, 5, 8]

# Binary search expects sorted input
println lucyds.binary_search(ordered, 5)
# 2
```

---

# API reference

Every class and function below is exported by `import lucyds`. Classes are instantiated with `.new(...)`; their `initialize` method is called by the Lucy object constructor.

## LinkedList

A singly linked list. Indexes are **zero-based**.

```lucy
let list = lucyds.LinkedList.new()

list.append(20)
list.append(30)
list.prepend(10)
list.insert(1, 15)

println list.to_array()
# [10, 15, 20, 30]
```

| Method | Description | Return value |
|---|---|---|
| `append(value)` | Add `value` to the end | The list (`self`) |
| `prepend(value)` | Add `value` to the beginning | The list (`self`) |
| `insert(index, value)` | Insert at a zero-based position; `index` may equal `size()` | `true` on success, otherwise `false` |
| `get(index)` | Read the value at an index | Value, or `nil` if out of range |
| `set(index, value)` | Replace the value at an index | `true` on success, otherwise `false` |
| `remove_at(index)` | Remove the item at an index | Removed value, or `nil` if out of range |
| `remove(value)` | Remove the first matching value | `true` if removed, otherwise `false` |
| `contains(value)` | Check whether a value exists | Boolean |
| `index(value)` | Find the first matching index | Index, or `nil` if absent |
| `size()` | Get the number of items | Integer |
| `is_empty()` | Check whether the list is empty | Boolean |
| `clear()` | Remove all items | The list (`self`) |
| `to_array()` | Copy list values into an array | Array |
| `reverse()` | Reverse the list in place | The list (`self`) |

Example:

```lucy
let list = lucyds.LinkedList.new()
list.append("red")
list.append("green")
list.append("blue")

println list.get(1)          # green
println list.index("blue")   # 2
println list.contains("red") # true

list.set(1, "lime")
list.remove("red")
println list.to_array()      # [lime, blue]

list.reverse()
println list.to_array()      # [blue, lime]
```

## DoublyLinkedList

A doubly linked list. Each node links to both its previous and next node. Indexes are zero-based. It is also exported under the short alias `DoublyLinked`.

```lucy
let list = lucyds.DoublyLinkedList.new()
# The alias works too:
let another = lucyds.DoublyLinked.new()
```

| Method | Description | Return value |
|---|---|---|
| `append(value)` | Add to the end | The list (`self`) |
| `prepend(value)` | Add to the beginning | The list (`self`) |
| `insert(index, value)` | Insert at an index from `0` through `size()` | Boolean |
| `get(index)` | Read an item by index | Value, or `nil` if out of range |
| `remove_at(index)` | Remove an item by index | Removed value, or `nil` if out of range |
| `remove(value)` | Remove the first matching value | Boolean |
| `contains(value)` | Check for a value | Boolean |
| `size()` | Get item count | Integer |
| `is_empty()` | Check whether the list is empty | Boolean |
| `clear()` | Remove all items | The list (`self`) |
| `to_array()` | Return values from head to tail | Array |
| `to_array_reverse()` | Return values from tail to head | Array |

Unlike `LinkedList`, the current `DoublyLinkedList` API does **not** provide `set`, `index`, or `reverse`.

## Stack

A **last-in, first-out (LIFO)** collection. The last pushed item is the first one popped.

```lucy
let stack = lucyds.Stack.new()

stack.push("first")
stack.push("second")

println stack.peek() # second
println stack.pop()  # second
println stack.pop()  # first
```

| Method | Description | Return value |
|---|---|---|
| `push(value)` | Add an item to the top | New size |
| `pop()` | Remove and return the top item | Value; empty stack returns `nil` |
| `peek()` | Read the top item without removing it | Value; empty stack returns `nil` |
| `size()` | Get item count | Integer |
| `is_empty()` | Check whether empty | Boolean |
| `clear()` | Remove all items | The stack (`self`) |
| `to_array()` | Copy internal items, bottom to top | Array |

## Queue

A **first-in, first-out (FIFO)** collection. Items are removed in the same order they were added.

```lucy
let queue = lucyds.Queue.new()

queue.enqueue("first")
queue.enqueue("second")

println queue.peek()    # first
println queue.dequeue() # first
println queue.dequeue() # second
```

| Method | Description | Return value |
|---|---|---|
| `enqueue(value)` | Add an item at the back | New queue size |
| `dequeue()` | Remove the front item | Value; empty queue returns `nil` |
| `peek()` | Read the front item without removing it | Value; empty queue returns `nil` |
| `size()` | Get item count | Integer |
| `is_empty()` | Check whether empty | Boolean |
| `clear()` | Remove all items | The queue (`self`) |
| `to_array()` | Copy items from front to back | Array |

The queue periodically compacts its backing array after enough front removals. This is an internal implementation detail; callers use the same API.

## Deque

A **double-ended queue** allows insertion and removal at both ends.

```lucy
let deque = lucyds.Deque.new()

deque.push_back(20)
deque.push_front(10)
deque.push_back(30)

println deque.to_array() # [10, 20, 30]
println deque.pop_front() # 10
println deque.pop_back()  # 30
```

| Method | Description | Return value |
|---|---|---|
| `push_front(value)` | Add at the front | New size |
| `push_back(value)` | Add at the back | New size |
| `pop_front()` | Remove the front item | Value; empty deque returns `nil` |
| `pop_back()` | Remove the back item | Value; empty deque returns `nil` |
| `front()` | Read the front item | Value; empty deque returns `nil` |
| `back()` | Read the back item | Value; empty deque returns `nil` |
| `size()` | Get item count | Integer |
| `is_empty()` | Check whether empty | Boolean |
| `clear()` | Remove all items | The deque (`self`) |
| `to_array()` | Copy items from front to back | Array |

## HashTable

A hash table that stores key/value pairs using **separate chaining** to handle collisions.

```lucy
let table = lucyds.HashTable.new()

table.set("name", "Lucy")
table.set("version", "2.0.7")

println table.get("name")       # Lucy
println table.has("version")    # true
println table.keys()
println table.values()

table.delete("version")
```

| Method | Description | Return value |
|---|---|---|
| `set(key, value)` | Insert a key or replace its value | `true` for a valid string key; otherwise `false` |
| `get(key)` | Retrieve a value | Value, or `nil` if missing/invalid |
| `has(key)` | Check whether a key exists | Boolean |
| `delete(key)` | Remove a key/value pair | `true` if removed, otherwise `false` |
| `keys()` | Return all stored keys | Array |
| `values()` | Return all stored values | Array |
| `size()` | Get number of entries | Integer |
| `is_empty()` | Check whether there are no entries | Boolean |
| `load_factor()` | Return entry count divided by bucket capacity | Numeric value |
| `clear()` | Remove all entries while retaining current capacity | The table (`self`) |

**Important details**

- Keys must be strings. Non-string keys are rejected by `set`.
- Values can be Lucy values.
- The default initial capacity is `17`.
- If the requested capacity is not a positive integer, the constructor falls back to `17`.
- The table grows when the entry count becomes greater than three times its bucket capacity.
- `get(key)` returns `nil` both when a key is absent and when a present key stores `nil`. Use `has(key)` to tell those cases apart.
- The built-in hash calculation is portable and supports characters outside its built-in ASCII alphabet, but those characters may lead to more hash collisions.

## BinarySearchTree

A binary search tree (BST). Values must support ordering comparisons. Duplicate values are not inserted.

```lucy
let tree = lucyds.BinarySearchTree.new()

tree.insert(40)
tree.insert(20)
tree.insert(60)
tree.insert(10)
tree.insert(30)

println tree.inorder() # [10, 20, 30, 40, 60]
println tree.min()     # 10
println tree.max()     # 60
```

| Method | Description | Return value |
|---|---|---|
| `insert(value)` | Insert a value | `true` if inserted; `false` for a duplicate |
| `contains(value)` | Search for a value | Boolean |
| `remove(value)` | Remove a value | `true` if removed; otherwise `false` |
| `min()` | Get the smallest value | Value, or `nil` if empty |
| `max()` | Get the largest value | Value, or `nil` if empty |
| `inorder()` | Left, node, right traversal | Sorted array |
| `preorder()` | Node, left, right traversal | Array |
| `postorder()` | Left, right, node traversal | Array |
| `size()` | Get node count | Integer |
| `is_empty()` | Check whether empty | Boolean |
| `height()` | Get tree height in nodes | Integer; empty tree is `0` |

The BST is not self-balancing. Its performance depends on insertion order; sorted or nearly sorted input can produce a tall tree.

## AVLTree

A self-balancing binary search tree. Rotations are performed during insertion to keep the tree balanced. Duplicate values are ignored.

```lucy
let tree = lucyds.AVLTree.new()

tree.insert(30)
tree.insert(20)
tree.insert(10)

println tree.contains(20) # true
println tree.inorder()    # [10, 20, 30]
println tree.height()
```

| Method | Description | Return value |
|---|---|---|
| `insert(value)` | Insert and rebalance | `true` if inserted; `false` for a duplicate |
| `contains(value)` | Search for a value | Boolean |
| `inorder()` | Return values in ascending order | Array |
| `size()` | Get node count | Integer |
| `height()` | Get tree height in nodes | Integer; empty tree is `0` |
| `is_empty()` | Check whether empty | Boolean |

**Current limitation:** AVL deletion is not implemented. Use `BinarySearchTree` if your use case requires removing nodes.

## BinaryHeap

A binary **min-heap**. The smallest value is available at the root. Values must support numeric ordering.

```lucy
let heap = lucyds.BinaryHeap.new()

heap.push(40)
heap.push(10)
heap.push(30)

println heap.peek() # 10
println heap.pop()  # 10
println heap.pop()  # 30
```

| Method | Description | Return value |
|---|---|---|
| `push(value)` | Insert a value and restore heap order | New size |
| `peek()` | Read the smallest value | Value; empty heap returns `nil` |
| `pop()` | Remove and return the smallest value | Value; empty heap returns `nil` |
| `size()` | Get item count | Integer |
| `is_empty()` | Check whether empty | Boolean |
| `clear()` | Remove all values | The heap (`self`) |
| `to_array()` | Copy the internal heap representation | Array |

`to_array()` returns the heap's internal level-order representation. It is **not** a fully sorted array. Repeatedly call `pop()` if you need values in ascending order.

## Trie

A prefix tree for strings. It is useful for prefix checks and dictionary-like word lookup.

```lucy
let trie = lucyds.Trie.new()

trie.insert("lucy")
trie.insert("lucky")

println trie.contains("lucy")       # true
println trie.contains("luc")        # false
println trie.starts_with("luc")     # true
println trie.size()                 # 2
```

| Method | Description | Return value |
|---|---|---|
| `insert(word)` | Insert a word | `true` if new; `false` if already present |
| `contains(word)` | Check whether a complete word exists | Boolean |
| `starts_with(prefix)` | Check whether a prefix exists | Boolean |
| `size()` | Get number of stored words | Integer |
| `is_empty()` | Check whether no words are stored | Boolean |

The trie works with strings. Insertion of a duplicate word does not increase its size.

## Graph

A weighted graph with string vertex identifiers. It can be directed or undirected.

```lucy
# Undirected graph (default)
let graph = lucyds.Graph.new()

graph.add_vertex("A")
graph.add_vertex("B")
graph.add_edge("A", "B", 7)

println graph.has_edge("A", "B")       # true
println graph.edge_weight("A", "B")    # 7
println graph.neighbors("A")
```

Create a directed graph by passing `true`:

```lucy
let directed = lucyds.Graph.new(true)
directed.add_edge("start", "finish", 4)

println directed.has_edge("start", "finish") # true
println directed.has_edge("finish", "start") # false
```

| Method | Description | Return value |
|---|---|---|
| `Graph.new()` | Create an undirected graph | Graph instance |
| `Graph.new(true)` | Create a directed graph | Graph instance |
| `add_vertex(vertex)` | Add a string vertex | `true` if added; `false` if invalid or already present |
| `has_vertex(vertex)` | Check whether a vertex exists | Boolean |
| `add_edge(from, to, weight = 1)` | Add or update an edge; missing endpoint vertices are created automatically | Boolean |
| `has_edge(from, to)` | Check whether an edge exists | Boolean |
| `edge_weight(from, to)` | Read an edge's weight | Weight, or `nil` if no edge exists |
| `neighbors(vertex)` | Get adjacent vertices | Array; unknown vertex returns an empty array |
| `vertices()` | Get all vertex identifiers | Array |
| `remove_edge(from, to)` | Remove an edge | Boolean |
| `remove_vertex(vertex)` | Remove a vertex and its incident edges | Boolean |
| `vertex_count()` | Get vertex count | Integer |
| `size()` | Alias for `vertex_count()` | Integer |
| `edges_count()` | Get edge count | Integer |
| `is_directed()` | Check graph direction mode | Boolean |

**Graph constraints**

- Vertex identifiers must be strings.
- Edge weights are expected to be numeric when used with weighted algorithms.
- In an undirected graph, an edge is stored in both directions.
- Adding an edge automatically creates either endpoint if it does not already exist.
- Adding an existing edge updates its weight rather than adding another edge.

## UnionFind

Also known as **Disjoint Set Union (DSU)**. It tracks a collection of disjoint groups and supports efficient merging and connectivity checks.

```lucy
let groups = lucyds.UnionFind.new()

groups.add("A")
groups.add("B")
groups.add("C")

groups.union("A", "B")

println groups.connected("A", "B") # true
println groups.connected("A", "C") # false
println groups.components()        # 2
```

| Method | Description | Return value |
|---|---|---|
| `add(value)` | Add a new string element | `true` if added; otherwise `false` |
| `find(value)` | Find the representative of an element's set | Representative string, or `nil` if unknown |
| `union(first, second)` | Merge the sets containing both elements | `true` if two sets were merged; otherwise `false` |
| `connected(first, second)` | Check whether two known elements share a set | Boolean |
| `components()` | Get the number of disjoint sets | Integer |
| `size()` | Get the number of registered elements | Integer |

Elements must be strings. `union` returns `false` if either element is unknown or both elements are already in the same set.

## Searching algorithms

These functions accept Lucy arrays.

| Function | Description | Return value | Time |
|---|---|---|---|
| `linear_search(items, target)` | Scan from the first item to the last | First matching index, or `nil` | O(n) |
| `binary_search(items, target)` | Binary search in sorted input | Matching index, or `nil` | O(log n) |
| `lower_bound(items, target)` | Find the first position whose value is not less than `target` | Index in the range `0..items.length()` | O(log n) |

Example:

```lucy
let values = [2, 4, 4, 7, 9]

println lucyds.linear_search(values, 7) # 3
println lucyds.binary_search(values, 4) # matching index
println lucyds.lower_bound(values, 4)   # 1
println lucyds.lower_bound(values, 5)   # 3
```

**Important:** `binary_search` and `lower_bound` require the input to already be sorted in ascending order. `linear_search` does not require sorted input.

## Sorting algorithms

All sorting functions return a **new array**. They do not modify the input array.

| Function | Description | Average time | Worst time |
|---|---|---:|---:|
| `bubble_sort(items)` | Repeatedly swap adjacent out-of-order values | O(n²) | O(n²) |
| `selection_sort(items)` | Repeatedly select the smallest remaining value | O(n²) | O(n²) |
| `insertion_sort(items)` | Insert each value into its sorted position | O(n²) | O(n²) |
| `merge_sort(items)` | Divide, sort, and merge subarrays | O(n log n) | O(n log n) |
| `quick_sort(items)` | Partition around a middle-element pivot | O(n log n) | O(n²) |

Example:

```lucy
let original = [9, 2, 7, 1, 5]

let a = lucyds.bubble_sort(original)
let b = lucyds.selection_sort(original)
let c = lucyds.insertion_sort(original)
let d = lucyds.merge_sort(original)
let e = lucyds.quick_sort(original)

println d        # [1, 2, 5, 7, 9]
println original # [9, 2, 7, 1, 5]
```

Values must support the comparison operators used by the sorting implementation. The algorithms sort in ascending order.

## Graph algorithms

These functions operate on a `Graph` instance.

### `breadth_first_search(graph, start)`

Visits vertices breadth-first, exploring neighbors level by level.

```lucy
let graph = lucyds.Graph.new()
graph.add_edge("A", "B")
graph.add_edge("A", "C")
graph.add_edge("B", "D")

let order = lucyds.breadth_first_search(graph, "A")
println order
```

Returns an array containing the visitation order. If `start` is not a vertex, it returns an empty array.

### `depth_first_search(graph, start)`

Visits vertices depth-first using an explicit stack.

```lucy
let order = lucyds.depth_first_search(graph, "A")
println order
```

Returns an array containing the visitation order. If `start` is not a vertex, it returns an empty array. Neighbor iteration order depends on the graph's map key order.

### `dijkstra(graph, start)`

Calculates shortest-path distances from `start` to all vertices reachable through non-negative edges.

```lucy
let graph = lucyds.Graph.new()
graph.add_edge("A", "B", 4)
graph.add_edge("A", "C", 2)
graph.add_edge("C", "B", 1)

let distances = lucyds.dijkstra(graph, "A")
println distances.get("A") # 0
println distances.get("B") # 3
println distances.get("C") # 2
```

Returns a Lucy map keyed by vertex identifier:

- The start vertex has distance `0`.
- Reachable vertices receive their shortest known distance.
- Unreachable vertices remain mapped to `nil`.
- If a negative edge is encountered while exploring vertices reachable from the start, the function returns `nil`.
- Dijkstra's algorithm requires non-negative edge weights. Do not use it for graphs with negative edge weights.

---

## Return values and edge cases

The library uses simple return values instead of raising a custom exception for ordinary missing-item cases.

| Situation | Behavior |
|---|---|
| Read an invalid list index | `get` returns `nil` |
| Remove from an empty stack or queue | Returns `nil` |
| Peek at an empty stack or queue | Returns `nil` |
| Remove an item that does not exist | Returns `false` for `remove`/`delete` methods |
| Search for a missing list value | `index` returns `nil`; `contains` returns `false` |
| Read a missing hash-table key | `get` returns `nil`; use `has` to check presence |
| Search for a missing tree value | `contains` returns `false` |
| Traverse a graph from an unknown start vertex | BFS/DFS return an empty array |
| Run Dijkstra from an unknown start vertex | Returns a map whose vertex distances are `nil` |
| Insert duplicate values into BST/AVL | Returns `false` and leaves the tree unchanged |
| Insert a duplicate word into Trie | Returns `false` and does not increase its size |

The library does not currently define a single custom error type for invalid arguments. Check return values where an operation can fail.

---

## Complexity reference

The following table describes the expected algorithmic complexity of the implemented operations. For linked structures, `n` is the number of nodes; for arrays and trees, it is the number of stored elements.

| Structure / operation | Typical time complexity |
|---|---:|
| LinkedList append | O(1) |
| LinkedList prepend | O(1) |
| LinkedList get / set / search | O(n) |
| LinkedList insert / remove_at | O(n) |
| DoublyLinkedList append / prepend | O(1) |
| DoublyLinkedList get | O(n), traverses from the nearer end |
| Stack push / pop / peek | O(1) amortized |
| Queue enqueue / dequeue / peek | O(1) amortized |
| Deque front/back access | O(1) |
| Deque insertion/removal at either end | Depends on Lucy array operation costs |
| HashTable set / get / has / delete | O(1) average, O(n) worst case |
| BinarySearchTree search / insert / remove | O(log n) average, O(n) worst case |
| AVLTree search / insert | O(log n) |
| BinaryHeap push / pop | O(log n) |
| BinaryHeap peek | O(1) |
| Trie insert / lookup / prefix check | O(k), where k is the string length |
| UnionFind find / union | Near O(1) amortized |
| Linear search | O(n) |
| Binary search / lower bound | O(log n) |
| Bubble / selection / insertion sort | O(n²) |
| Merge sort | O(n log n) |
| Quick sort | O(n log n) average, O(n²) worst case |
| BFS / DFS | O(V + E) |
| Dijkstra in this implementation | O(V² + E) |

Complexities are algorithmic expectations, not runtime benchmarks. Actual performance also depends on Lucy runtime behavior and the input data.

---

## Run the tests

Run the test file from the project root:

```bash
lucy tests/test_lucyds.lucy
```

The repository's test file exercises the exported API, including collection operations, boundary conditions, hash-table resizing, tree operations, graph traversal, and algorithms.

If you add or change a public method, add a corresponding test and update this README or `docs/API.md`.

---

## Examples

A runnable example is provided at:

```text
examples/basic_usage.lucy
```

Run it from the project root:

```bash
lucy examples/basic_usage.lucy
```

For detailed method documentation, see [`docs/API.md`](docs/API.md).

---

## Current limitations

The following are limitations of the implementation in this release:

1. **No generic Set:** a standalone `Set` data structure is not included.
2. **AVL deletion:** `AVLTree` supports insertion and lookup, but not node deletion.
3. **Min-heap only:** `BinaryHeap` is a min-heap; there is no max-heap mode or custom comparator.
4. **Dijkstra only:** the graph algorithm module currently provides BFS, DFS, and Dijkstra. Dijkstra does not support negative edge weights.
5. **String identifiers:** `Graph` vertices, `UnionFind` elements, and `HashTable` keys must be strings.
6. **Ordering requirements:** sorting and search-tree operations require values that can be compared with the relevant ordering operators.
7. **No guaranteed traversal order for maps:** graph vertex and neighbor order follows the underlying map's key enumeration behavior.

These limitations are documented intentionally so users can distinguish available functionality from possible future additions.

---

## Contributing

Contributions are welcome. To keep the library consistent:

1. Keep implementations in the relevant module under `packages/lucyds/src/`.
2. Export only intended public classes and functions through `packages/lucyds/src/init.lucy`.
3. Prefix implementation-only names with `_` and keep helpers private where appropriate.
4. Add tests in `tests/test_lucyds.lucy` for new behavior and edge cases.
5. Update this README and `docs/API.md` whenever the public API changes.
6. Verify changes with Lucy 2.0.7 before submitting them.

Please avoid documenting an API as available until it is implemented, exported, and covered by tests.

---

## License

LucyDS is distributed under the **GNU General Public License v3.0 only (GPL-3.0-only)**. See the [`LICENSE`](LICENSE) file for the complete license text.

---

## Author

**Develop by Abolfazl ilka.**
