#pragma once

#include <vector>
#include <string>


class Edge;

class Vertex
{
public:
	int id;
	double time;
	double memory;
	std::vector<Edge*> outgoing_edges;
	std::vector<Edge*> incoming_edges;

	Vertex(int i, double t = 0, double m = 0);
	void add_in_edge(Edge* e);
	void add_out_edge(Edge* e);
	
	inline int out_degree()
	{
		return outgoing_edges.size();
	}

	inline int in_degree()
	{
		return incoming_edges.size();
	}
	
};

class Edge
{
public:
	int id;
	int id_from, id_to;
	bool red;
	double weight;
	Edge(int i, int from, int to, double w=1, bool r=false);
	std::string to_string();
};


/* Class for directed graphs
 * 
 */
class Graph
{
private:


public:
	std::vector<Edge*> edges;
	std::vector<Vertex> vertices;
	
	Graph();
	Graph(const Graph &graph);
	~Graph();

	Graph& operator=(const Graph &graph);

	int source_id, target_id; // -1 if not set
	int find_source();
	int find_target();

	bool path_exists(int a, int b);

	int add_vertex(double time = 0, double memory = 0);
	int add_edge(int id_from, int id_to, double weight = 0, bool red = false);
	void make_single_source_target();
	void write_to_file(std::string filename);
	std::string to_string();

	inline int n_vertices() const
	{
		return vertices.size();
	}

	inline int n_edges() const
	{
		return edges.size();
	}

};


Graph read_graph_from_file(std::string filename, std::string time_label, std::string weight_label, std::string computation_label);
Graph convert_to_SimpleDataFlow(const Graph &graph);
Graph generate_dag_ss(int n, double connectedness, double w_max, double t_max);