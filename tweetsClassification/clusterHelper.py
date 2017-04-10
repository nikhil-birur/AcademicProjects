"""
cluster.py: This should read the data collected in the previous steps and use any community detection algorithm to cluster users into communities.
You may write any files you need to save the results.
"""
import networkx as nx
import matplotlib.pyplot as plt

class cluster():

    def __init__(self,follower_ids,networkFileName,G):
       self.networkFileName = networkFileName
       self.follower_ids = follower_ids
       self.G = G

    def draw_network(self):
        """
        Draws the network to the file
        :return:
         The graph before clustering
        """
        follower_ids = self.follower_ids
        self.G.add_nodes_from(follower_ids.keys())
        for k, v in follower_ids.items():
            for i in v:
                if (i not in self.G.nodes()):
                    self.G.add_node(i)
                    self.G.add_edge(k, i)
                else:
                    self.G.add_edge(k, i)
        self.plot_graph(self.G, self.networkFileName)
        return self.G

    def plot_graph(self, G,file):
        """
        Plots the graph drawn in the draw_network method
        :param G: The graph drawn in draw_network
        :param file: The file where the network is saved
        :return:
         Nill
        """

        pos = nx.spring_layout(G, scale=5)
        plt.axis("off")
        nx.draw_networkx_nodes(G, pos, alpha=0.5, node_size=22, node_color='red')
        nx.draw_networkx_edges(G, pos, alpha=0.3, width=0.15)
        plt.savefig(file, dpi=100)

    def _without_most_central_edges(self,G, most_valuable_edge):
        """Returns the connected components of the graph that results from
        repeatedly removing the most "valuable" edge in the graph.

        `G` must be a non-empty graph. This function modifies the graph `G`
        in-place; that is, it removes edges on the graph `G`.

        `most_valuable_edge` is a function that takes the graph `G` as input
        (or a subgraph with one or more edges of `G` removed) and returns an
        edge. That edge will be removed and this process will be repeated
        until the number of connected components in the graph increases.

        """
        original_num_components = nx.number_connected_components(G)
        num_new_components = original_num_components
        while num_new_components <= original_num_components:
            edge = most_valuable_edge(G)
            G.remove_edge(*edge)
            new_components = tuple(nx.connected_components(G))
            num_new_components = len(new_components)
        return new_components


    def girvan_newman(self, G, most_valuable_edge=None):
        """Finds communities in a graph using the Girvan–Newman method.

        Parameters
        ----------
        G : NetworkX graph

        most_valuable_edge : function

            Function that takes a graph as input and outputs an edge. The
            edge returned by this function will be recomputed and removed at
            each iteration of the algorithm.

            If not specified, the edge with the highest
            :func:`networkx.edge_betweenness_centrality` will be used.

        Returns
        -------
        iterator

            Iterator over tuples of sets of nodes in `G`. Each set of node
            is a community, each tuple is a sequence of communities at a
            particular level of the algorithm.

        """

        if G.number_of_edges() == 0:
            yield tuple(nx.connected_components(G))
            return
        if most_valuable_edge is None:
            def most_valuable_edge(G):
                betweenness = nx.edge_betweenness_centrality(G)
                return max(betweenness, key=betweenness.get)
        g = G.copy().to_undirected()
        g.remove_edges_from(g.selfloop_edges())
        while g.number_of_edges() > 0:
            yield self._without_most_central_edges(g, most_valuable_edge)
