# coding: utf-8

# Implementation of community detection and link prediction algorithms using Facebook "like" data.
#
# The file `edges.txt.gz` indicates like relationships between facebook users. This was collected using snowball sampling: beginning with the user "Bill Gates", I crawled all the people he "likes", then, for each newly discovered user, I crawled all the people they liked.
#
# Involves clustering the resulting graph into communities, as well as recommend friends for Bill Gates.


from collections import Counter, defaultdict, deque
import copy
import math
import time
import networkx as nx
import urllib.request


## Community Detection

def example_graph():
    """
        Example Graph for testing
    """
    g = nx.Graph()
    g.add_edges_from(
                     [('A', 'B'), ('A', 'C'), ('B', 'C'), ('B', 'D'), ('D', 'E'), ('D', 'F'), ('D', 'G'), ('E', 'F'), ('G', 'F')])
    return g


def bfs(graph, root, max_depth):
    """
        Perform breadth-first search to compute the shortest paths from a root node to all
        other nodes in the graph. To reduce running time, the max_depth parameter ends
        the search after the specified depth.
        E.g., if max_depth=2, only paths of length 2 or less will be considered.
        This means that nodes greather than max_depth distance from the root will not
        appear in the result.
        
        Params:
        graph.......A networkx Graph
        root........The root node in the search graph (a string). We are computing
        shortest paths from this node to all others.
        max_depth...An integer representing the maximum depth to search.
        
        Returns:
        node2distances...dict from each node to the length of the shortest path from
        the root node
        node2num_paths...dict from each node to the number of shortest paths from the
        root node that pass through this node.
        node2parents.....dict from each node to the list of its parents in the search
        tree
        
        In the doctests below, we first try with max_depth=5, then max_depth=2.
        
        >>> node2distances, node2num_paths, node2parents = bfs(example_graph(), 'E', 5)
        >>> sorted(node2distances.items())
        [('A', 3), ('B', 2), ('C', 3), ('D', 1), ('E', 0), ('F', 1), ('G', 2)]
        >>> sorted(node2num_paths.items())
        [('A', 1), ('B', 1), ('C', 1), ('D', 1), ('E', 1), ('F', 1), ('G', 2)]
        >>> sorted((node, sorted(parents)) for node, parents in node2parents.items())
        [('A', ['B']), ('B', ['D']), ('C', ['B']), ('D', ['E']), ('F', ['E']), ('G', ['D', 'F'])]
        >>> node2distances, node2num_paths, node2parents = bfs(example_graph(), 'E', 2)
        >>> sorted(node2distances.items())
        [('B', 2), ('D', 1), ('E', 0), ('F', 1), ('G', 2)]
        >>> sorted(node2num_paths.items())
        [('B', 1), ('D', 1), ('E', 1), ('F', 1), ('G', 2)]
        >>> sorted((node, sorted(parents)) for node, parents in node2parents.items())
        [('B', ['D']), ('D', ['E']), ('F', ['E']), ('G', ['D', 'F'])]
    """
    d = deque()
    node2num_paths = defaultdict(list)
    nodes2distances = defaultdict(list)
    node2parents = defaultdict(list)
    visited = defaultdict(list)
    
    visited[root] = 'Y'
    nodes2distances[root] = 0
    
    for nodes in graph.nodes():
        if nodes != root:
            visited[nodes] = 'N'
            nodes2distances[nodes] = float("inf")
            node2parents[nodes] = []

    d.append((root, 0))
    while len(d) > 0:
            front = d.popleft()
            u = front[0]
            level = front[1]
            if level >= max_depth:
                break
            for v in graph.neighbors(u):
                if visited[v] == 'N':
                    if nodes2distances[u] + 1 <= nodes2distances[v]:
                        nodes2distances[v] = nodes2distances[u] + 1
                        node2parents[v].append(u)
                        d.append((v, level + 1))
            visited[u] = 'Y'

    nodes2distances = {k: v for k, v in nodes2distances.items() if v != float("inf")}
    node2parents = {k: v for k, v in node2parents.items() if len(v) != 0}
    
    node2num_paths[root] = 1
    for nodes in graph.nodes():
        if nodes != root and nodes in node2parents:
            node2num_paths[nodes] = len(node2parents[nodes])

    return (nodes2distances, node2num_paths, node2parents)


def bottom_up(root, node2distances, node2num_paths, node2parents):
    """
        Compute the final step of the Girvan-Newman algorithm.
        
        Params:
        root.............The root node in the search graph (a string). We are computing
        shortest paths from this node to all others.
        node2distances...dict from each node to the length of the shortest path from
        the root node
        node2num_paths...dict from each node to the number of shortest paths from the
        root node that pass through this node.
        node2parents.....dict from each node to the list of its parents in the search
        tree
        Returns:
        A dict mapping edges to credit value. Each key is a tuple of two strings
        representing an edge (e.g., ('A', 'B')). Make sure each of these tuples
        are sorted alphabetically (so, it's ('A', 'B'), not ('B', 'A')).
        
        Any edges excluded from the results in bfs should also be exluded here.
        
        >>> node2distances, node2num_paths, node2parents = bfs(example_graph(), 'E', 5)
        >>> result = bottom_up('E', node2distances, node2num_paths, node2parents)
        >>> sorted(result.items())
        [(('A', 'B'), 1.0), (('B', 'C'), 1.0), (('B', 'D'), 3.0), (('D', 'E'), 4.5), (('D', 'G'), 0.5), (('E', 'F'), 1.5), (('F', 'G'), 0.5)]
    """
    
    betweeness_edges = defaultdict(list)
    betweeness_nodes_labels = {w:1 for w in node2distances.keys()}
    node2distances = sorted(node2distances.items(),key=lambda k_v: k_v[1],reverse=True)
    for key in node2distances:
        if key[0]!=root:
            for val in node2parents[key[0]]:
                t = (key[0], val)
                t = tuple(sorted(t))
                betweeness_edges[t] =  betweeness_nodes_labels[key[0]] / (node2num_paths[val] * len(node2parents[key[0]]))
                betweeness_nodes_labels[val] = betweeness_nodes_labels[val] + (betweeness_nodes_labels[key[0]] / len(node2parents[key[0]]))


    return betweeness_edges




def approximate_betweenness(graph, max_depth):
    """
        Compute the approximate betweenness of each edge, using max_depth to reduce
        computation time in breadth-first search.
        
        Params:
        graph.......A networkx Graph
        max_depth...An integer representing the maximum depth to search.
        
        Returns:
        A dict mapping edges to betweenness. Each key is a tuple of two strings
        representing an edge (e.g., ('A', 'B')). Make sure each of these tuples
        are sorted alphabetically (so, it's ('A', 'B'), not ('B', 'A')).
        
        >>> sorted(approximate_betweenness(example_graph(), 2).items())
        [(('A', 'B'), 2.0), (('A', 'C'), 1.0), (('B', 'C'), 2.0), (('B', 'D'), 6.0), (('D', 'E'), 2.5), (('D', 'F'), 2.0), (('D', 'G'), 2.5), (('E', 'F'), 1.5), (('F', 'G'), 1.5)]
    """

    approx_betweenness = defaultdict(tuple)
    for node in graph.nodes():
        node2distances, node2num_paths, node2parents = bfs(graph, node, max_depth)
        result = bottom_up(node, node2distances, node2num_paths, node2parents)
        for key, value in result.items():
            if key in approx_betweenness:
                approx_betweenness[key] = (approx_betweenness[key] + result[key])
            else:
                approx_betweenness[key] = result[key]

    for key, value in approx_betweenness.items():
        approx_betweenness[key] = approx_betweenness[key] / 2
    
    return (approx_betweenness)


def partition_girvan_newman(graph, max_depth):
    """
        Using approximate_betweenness implementation to partition a graph.
        
        Returns:
        A list of networkx Graph objects, one per partition.
        
        >>> components = partition_girvan_newman(example_graph(), 5)
        >>> components = sorted(components, key=lambda x: sorted(x.nodes())[0])
        >>> sorted(components[0].nodes())
        ['A', 'B', 'C']
        >>> sorted(components[1].nodes())
        ['D', 'E', 'F', 'G']
    """
    copy_graph = graph.copy()
    result = sorted(approximate_betweenness(graph, max_depth).items(), key=lambda k: (-k[1],k[0][0],k[0][1]))
    components = [comp for comp in nx.connected_component_subgraphs(copy_graph)]
    for i in result:
        if (len(components) > 1):
            break
        copy_graph.remove_edge(i[0][0],i[0][1])
        components = [comp for comp in nx.connected_component_subgraphs(copy_graph)]
    
    
    return components


def get_subgraph(graph, min_degree):
    """
        Return a subgraph containing nodes whose degree is
        greater than or equal to min_degree.
        
        Params:
        graph........a networkx graph
        min_degree...degree threshold
        Returns:
        a networkx graph, filtered as defined above.
        
        >>> subgraph = get_subgraph(example_graph(), 3)
        >>> sorted(subgraph.nodes())
        ['B', 'D', 'F']
        >>> len(subgraph.edges())
        2
    """
    sub_graph = []
    for node in graph.nodes():
        if graph.degree(node) >= min_degree:
            sub_graph.append(node)


    return graph.subgraph(sub_graph)




def volume(nodes, graph):
    """
        Compute the volume for a list of nodes, which
        is the number of edges in `graph` with at least one end in
        nodes.
        Params:
        nodes...a list of strings for the nodes to compute the volume of.
        graph...a networkx graph
        
        >>> volume(['A', 'B', 'C'], example_graph())
        4
    """
    return len(graph.edges(nodes))


def cut(S, T, graph):
    """
        Compute the cut-set of the cut (S,T), which is
        the set of edges that have one endpoint in S and
        the other in T.
        Params:
        S.......set of nodes in first subset
        T.......set of nodes in second subset
        graph...networkx graph
        Returns:
        An int representing the cut-set.
        
        >>> cut(['A', 'B', 'C'], ['D', 'E', 'F', 'G'], example_graph())
        1
    """
    no_of_cuts = 0
    for i in S:
        for j in T:
            if graph.has_edge(i, j):
                no_of_cuts += 1


    return no_of_cuts


def norm_cut(S, T, graph):
    """
        The normalized cut value for the cut S/T. (See lec06.)
        Params:
        S.......set of nodes in first subset
        T.......set of nodes in second subset
        graph...networkx graph
        Returns:
        An float representing the normalized cut value
        
    """
    return cut(S, T, graph) / volume(S, graph) + cut(S, T, graph) / volume(T, graph)


def score_max_depths(graph, max_depths):
    """
        In order to assess the quality of the approximate partitioning method
        we've developed, we will run it with different values for max_depth
        and see how it affects the norm_cut score of the resulting partitions.
        Recall that smaller norm_cut scores correspond to better partitions.
        
        Params:
        graph........a networkx Graph
        max_depths...a list of ints for the max_depth values to be passed
        to calls to partition_girvan_newman
        
        Returns:
        A list of (int, float) tuples representing the max_depth and the
        norm_cut value obtained by the partitions returned by
        partition_girvan_newman. See Log.txt for an example.
    """

    score = []
    for i in max_depths:
        components = partition_girvan_newman(graph, i)
        components = sorted(components, key=lambda x: sorted(x.nodes())[0])
        n = norm_cut(components[0].nodes(), components[1].nodes(),graph)
        score.append((i,n))
    
    return score

## Link prediction

# Next, we'll consider the link prediction problem. In particular,
# we will remove 5 of the accounts that Bill Gates likes and
# compute our accuracy at recovering those links.

def make_training_graph(graph, test_node, n):
    """
        To make a training graph, we need to remove n edges from the graph.
        As in lecture, we'll assume there is a test_node for which we will
        remove some edges. Remove the edges to the first n neighbors of
        test_node, where the neighbors are sorted alphabetically.
        E.g., if 'A' has neighbors 'B' and 'C', and n=1, then the edge
        ('A', 'B') will be removed.
        
        Be sure to *copy* the input graph prior to removing edges.
        
        Params:
        graph.......a networkx Graph
        test_node...a string representing one node in the graph whose
        edges will be removed.
        n...........the number of edges to remove.
        
        Returns:
        A *new* networkx Graph with n edges removed.
        
        In this doctest, we remove edges for two friends of D:
        >>> g = example_graph()
        >>> sorted(g.neighbors('D'))
        ['B', 'E', 'F', 'G']
        >>> train_graph = make_training_graph(g, 'D', 2)
        >>> sorted(train_graph.neighbors('D'))
        ['F', 'G']
    """
    neighbours = sorted(graph.neighbors(test_node))
    copy_graph = graph.copy()
    for i in range(n):
        copy_graph.remove_edge(test_node, neighbours[i])
    
    return copy_graph


def jaccard(graph, node, k):
    """
        Compute the k highest scoring edges to add to this node based on
        the Jaccard similarity measure.
        Note that we don't return scores for edges that already appear in the graph.
        
        Params:
        graph....a networkx graph
        node.....a node in the graph (a string) to recommend links for.
        k........the number of links to recommend.
        
        Returns:
        A list of tuples in descending order of score representing the
        recommended new edges. Ties are broken by
        alphabetical order of the terminal node in the edge.
        
        In this example below, we remove edges (D, B) and (D, E) from the
        example graph. The top two edges to add according to Jaccard are
        (D, E), with score 0.5, and (D, A), with score 0. (Note that all the
        other remaining edges have score 0, but 'A' is first alphabetically.)
        
        >>> g = example_graph()
        >>> train_graph = make_training_graph(g, 'D', 2)
        >>> jaccard(train_graph, 'D', 2)
        [(('D', 'E'), 0.5), (('D', 'A'), 0.0)]
    """
    neighbors = set(graph.neighbors(node))
    scores = []
    jaccard_val = defaultdict(tuple)
    for n in graph.nodes():
        if (n != node):
            if (n not in neighbors):
                neighbors2 = set(graph.neighbors(n))
                scores.append((n, 1. * len(neighbors & neighbors2) / len(neighbors | neighbors2)))
            else:
                scores.append((n, 0.0))

    scores = sorted(sorted(scores, key=lambda x: x[0]), key=lambda x: x[1], reverse=True)
    
    for i in range(k):
        jaccard_val[(node, scores[i][0])] = scores[i][1]

    return sorted(jaccard_val.items(), key=lambda x: x[1], reverse=True)


# One limitation of Jaccard is that it only has non-zero values for nodes two hops away.
#
# Implement a new link prediction function that computes the similarity between two nodes $x$ and $y$  as follows:
#
# $$
# s(x,y) = \beta^i n_{x,y,i}
# $$
#
# where
# - $\beta \in [0,1]$ is a user-provided parameter
# - $i$ is the length of the shortest path from $x$ to $y$
# - $n_{x,y,i}$ is the number of shortest paths between $x$ and $y$ with length $i$


def path_score(graph, root, k, beta):
    """
        Compute a new link prediction scoring function based on the shortest
        paths between two nodes, as defined above.
        
        Params:
        graph....a networkx graph
        root.....a node in the graph (a string) to recommend links for.
        k........the number of links to recommend.
        beta.....the beta parameter in the equation above.
        
        Returns:
        A list of tuples in descending order of score. Ties are broken by
        alphabetical order of the terminal node in the edge.
        
        In this example below, we remove edge (D, F) from the
        example graph. The top two edges to add according to path_score are
        (D, F), with score 0.5, and (D, A), with score .25. (Note that (D, C)
        is tied with a score of .25, but (D, A) is first alphabetically.)
        
        >>> g = example_graph()
        >>> train_graph = g.copy()
        >>> train_graph.remove_edge(*('D', 'F'))
        >>> path_score(train_graph, 'D', k=4, beta=.5)
        [(('D', 'F'), 0.5), (('D', 'A'), 0.25), (('D', 'C'), 0.25)]
    """
    neighbors = graph.neighbors(root)
    path_scores = defaultdict(tuple)
    nodes2distances, node2num_paths, node2parents = bfs(graph, root, k)
    for v in graph.nodes():
        if v not in neighbors and v != root:
            path_scores[(root, v)] = (beta ** nodes2distances[v]) * node2num_paths[v]

    return sorted(sorted(path_scores.items(), key=lambda x: x[0]), key=lambda x: x[1], reverse=True)[:k]


def evaluate(predicted_edges, graph):
    """
        Return the fraction of the predicted edges that exist in the graph.
        
        Args:
        predicted_edges...a list of edges (tuples) that are predicted to
        exist in this graph
        graph.............a networkx Graph
        
        Returns:
        The fraction of edges in predicted_edges that exist in the graph.
        
        In this doctest, the edge ('D', 'E') appears in the example_graph,
        but ('D', 'A') does not, so 1/2 = 0.5
        
        >>> evaluate([('D', 'E'), ('D', 'A')], example_graph())
        0.5
    """
    sum = 0
    for i in predicted_edges:
        if graph.has_edge(*i):
            sum += 1
    return sum / len(predicted_edges)


"""
    Next, we'll download a real dataset to see how our algorithm performs.
"""


def download_data():
    urllib.request.urlretrieve('', '')#Specify the path and the file name here


def read_graph():
    return nx.read_edgelist('', delimiter='\t')#Specify the file name here


def main():
    """
        FYI: This takes ~10-15 seconds to run on my laptop.
        """
    download_data()
    graph = read_graph()
    print('graph has %d nodes and %d edges' %
    (graph.order(), graph.number_of_edges()))
    subgraph = get_subgraph(graph, 2)
    print('subgraph has %d nodes and %d edges' %
    (subgraph.order(), subgraph.number_of_edges()))
    print('norm_cut scores by max_depth:')
    print(score_max_depths(subgraph, range(1, 5)))
    clusters = partition_girvan_newman(subgraph, 3)
    print('first partition: cluster 1 has %d nodes and cluster 2 has %d nodes' %
    (clusters[0].order(), clusters[1].order()))
    print('cluster 2 nodes:')
    print(clusters[1].nodes())
          
    test_node = 'Bill Gates'
    train_graph = make_training_graph(subgraph, test_node, 5)
    print('train_graph has %d nodes and %d edges' %
    (train_graph.order(), train_graph.number_of_edges()))
          
    jaccard_scores = jaccard(train_graph, test_node, 5)
    print('\ntop jaccard scores for Bill Gates:')
    print(jaccard_scores)
    print('jaccard accuracy=%g' %
            evaluate([x[0] for x in jaccard_scores], subgraph))
          
    path_scores = path_score(train_graph, test_node, k=5, beta=.1)
    print('\ntop path scores for Bill Gates for beta=.1:')
    print(path_scores)
    print('path accuracy for beta .1=%g' %
            evaluate([x[0] for x in path_scores], subgraph))


if __name__ == '__main__':
    main()
