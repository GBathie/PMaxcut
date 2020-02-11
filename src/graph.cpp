#include "graph.h"
#include <fstream>
#include <iostream>
#include <map>
#include <ctime>
#include <random>
#include <sstream>

#include <gvc.h>

using namespace std;

/* This file contains 
 *
 */
Vertex::Vertex(int i, double t/*=0*/, double m/*=0*/)
{
	id = i;
	time = t;
	memory = m;
}

void Vertex::add_in_edge(Edge *e)
{
   incoming_edges.push_back(e); 
}

void Vertex::add_out_edge(Edge *e)
{
	outgoing_edges.push_back(e);
}

Edge::Edge(int i, int from, int to, double w/*=1*/, bool r/*=false*/)
{
	id = i;
	id_from = from;
	id_to   = to;
	weight = w;
	red = r;
}

string Edge::to_string()
{
	stringstream res;

	res << id_from << " -> " << id_to << "[id=" << id << ", weight="<< weight <<  ", red="<< ((red)?1:0) << "];\n";

	return res.str();
}

Graph::Graph()
{
	source_id = -1;
	target_id = -1;
}

Graph::Graph(const Graph &graph)
{
	*this = graph;
}

Graph& Graph::operator=(const Graph &graph)
{
	vertices.clear();
	for (int i = 0; i < graph.n_vertices(); i++)
		add_vertex(graph.vertices[i].time, graph.vertices[i].memory);
	
	for (auto e : edges)
		delete e;

	edges.clear();
	for (int i = 0; i < graph.n_edges(); i++)
		add_edge(graph.edges[i]->id_from, graph.edges[i]->id_to, graph.edges[i]->weight, graph.edges[i]->red);

	source_id = graph.source_id;
	target_id = graph.target_id;
	
	return *this;
}

Graph::~Graph()
{
	for (auto e : edges)
		delete e;
	
}


int Graph::add_vertex(double time /*= 0*/, double memory /*= 0*/)
{
	int new_id = vertices.size();
	vertices.push_back(Vertex(new_id, time, memory));
	return new_id;
}

int Graph::add_edge(int id_from, int id_to, double weight/*=1*/, bool red/*=false*/)
{
	int new_id = edges.size();
	Edge* e = new Edge(new_id, id_from, id_to, weight, red);
	edges.push_back(e);
	vertices[id_from].add_out_edge(e); 
	vertices[id_to  ].add_in_edge(e);
	return new_id;
}

bool Graph::path_exists(int a, int b)
{
	vector<bool> seen(n_vertices());
	vector<int> stack;
	
	stack.push_back(a);
	seen[a] = true;

	int u, v;
	while (!stack.empty())
	{
		u = stack.back();
		stack.pop_back();
		for (auto e : vertices[u].outgoing_edges)
		{
			v = e->id_to;
			if (v == b) return true;
			if (!seen[v])
			{
				seen[v] = true;
				stack.push_back(v);
			}
		}		
	}

	return false;
}

// If there is no source node, find one, update and return source_id
int Graph::find_source()
{
	if (source_id != -1) return source_id;
	for (auto &v : vertices)
	{
		if (source_id == -1 && v.in_degree() == 0 && v.id != target_id) //avoid a problem with isolated nodes
		{
			source_id = v.id;
		}
	}
	return source_id;
}

int Graph::find_target()
{
	if (target_id != -1) return target_id;
	// find a source and a target
	for (auto &v : vertices)
	{
		if (target_id == -1 && v.out_degree() == 0 && v.id != source_id)
		{
			target_id = v.id;
		}
	}
	return target_id;
}

/* Makes the graph single source and single target
 * Does not add nodes : it choses an arbitrary source/sink node
 * that becomes the new unique source / target of the graph
 */
void Graph::make_single_source_target()
{

	find_source();
	find_target();

	// link it to the rest of the graph
	for (auto &v : vertices)
	{
		if (v.id != source_id && v.in_degree() == 0)
		{
			add_edge(source_id, v.id);
		}

		if (v.id != target_id && v.out_degree() == 0)
		{
			add_edge(v.id, target_id);
		}
	}
}

string Graph::to_string()
{
	stringstream res;
	res << "strict digraph {\n";
	for (auto e : edges)
	{
		res << e->id_from << " -> " << e->id_to << "[id=" << e->id << ", weight="<< e->weight <<  ", red="<< ((e->red)?1:0) << "];\n";
	}
	

	res << "}\n";
	return res.str();
}

/* Writes the graph to a file, in dot format
 *
 * @param filename Name of the output file. If it does not exist, it is created.  
 */
void Graph::write_to_file(string filename)
{
	ofstream f;
	f.open(filename);
	f << to_string() << endl;
	f.close();
}

/********************* Non member functions *********************************/
/* Convert a graph into SimpleDataFlowModel (see article)
 * Each vertex becomes an edge in the new graph.
 *
 * @param graph Graph G to transform
 *
 * @return a graph corresponding to G in SimpleDataFlowModel 
 */
Graph convert_to_SimpleDataFlow(const Graph &graph)
{
	Graph res;
	map<int,pair<int,int>> old_new_id_map;
	int idf, idt;
	double w;
	// create all vertices
	for (auto &v :  graph.vertices)
	{
		idf = res.add_vertex();	
		idt = res.add_vertex();
		old_new_id_map[v.id] = make_pair(idf, idt);
		w = v.memory;
		for (auto e : v.incoming_edges)
		{
			w += e->weight;
		}
		for (auto e : v.outgoing_edges)
		{
			w += e->weight;
		}
		
		res.add_edge(idf, idt, w, true);		
	}

	// Add edges
	for (auto &e : graph.edges)
	{
		res.add_edge(old_new_id_map[e->id_from].second, old_new_id_map[e->id_to].first, e->weight, false);
	}

	// Make graph single source
	if (graph.source_id != -1)
		res.source_id = old_new_id_map[graph.source_id].first;		
	if (graph.target_id != -1)
		res.target_id = old_new_id_map[graph.target_id].second;
	res.make_single_source_target();

	return res;
}

/* Generate a random DAG and converts it to SDFM
 *
 * @param n 			Number of vertices
 * @param connectedness Each edge (i,j), i < j exists with probability connectedness
 * @param w_max 		Memory weights are drawn uniformly in [0, w_max)
 * @param t_max 		Time weights are drawn uniformly in [0, t_max)
 * 
 * @return a random DAG generated as stated above, converted to SimpleDataFlowModel
 */
Graph generate_dag_ss(int n, double connectedness, double w_max, double t_max)
{
	Graph res;

	for (int i = 0; i < n; ++i)
	{
		res.add_vertex((((double)random()) / RAND_MAX) * t_max, (((double)random()) / RAND_MAX) * w_max);
	}

	for (int i = 0; i < n; ++i)
	{
		for (int j = i+1; j < n; ++j)
		{
			double d = ((double)random()) / RAND_MAX;
			if (d < connectedness)
			{
				res.add_edge(i, j, (((double)random()) / RAND_MAX) * w_max);
			}
		}
	}

	res = convert_to_SimpleDataFlow(res);
	return res;
}

/* Read a graph in dot file from a file.
 * 
 * This functions assumes that the "agnameof(n)" (that is, the name of each node) can be cast into an integer.
 * If that's not the case, transform the map into a <string, int> map, and use the agnameof without casting it.
 * If a label is the empty string, it is ignored.
 *
 * @param filename			Name of the file to read the graph from
 * @param time_label 		Name of the dot propery that contains the time weight of the nodes
 * @param weight_label 		Name of the dot propery that contains the memory weight of the nodes and edges
 * @param computation_label Name of the dot propery that contains the information on wether an edge is a computation edge
 *
 * @return parsed graph
 */
Graph read_graph_from_file(string filename, string time_label, string weight_label, string computation_label)
{
	Agraph_t *g;
	FILE *f = fopen(filename.c_str(), "r");
	g = agread(f, 0);

	int a, b;
	double w = 0, t = 0;
	bool r = false;
	char *tmp;
	map<int, int> file_to_graph_id;

	Graph res;
	for (auto n = agfstnode(g); n; n = agnxtnode(g,n))
	{
		w = t = 0;

		a = stoi(agnameof(n));
		
		tmp = agget(n, time_label.data());
		if (tmp != NULL)
		{
			t = stod(tmp);
		}

		tmp = agget(n, weight_label.data());
		if (tmp != NULL)
		{
			w = stod(tmp);
		}

		file_to_graph_id[a] = res.add_vertex(t, w);
	}
	
	for (auto n = agfstnode(g); n; n = agnxtnode(g,n)) 
	{
		a = stoi(agnameof(n));
		for (auto e = agfstout(g,n); e; e = agnxtout(g,e)) 
		{
			w = 0;
			r = false;
			b = stoi(agnameof(aghead(e)));
			tmp = agget(e, weight_label.data());
			if (tmp != NULL)
			{
				w = stod(tmp);
			}

			tmp = agget(e, computation_label.data());
			if (tmp != NULL)
			{
				r = stoi(tmp);
			}
			res.add_edge(file_to_graph_id[a], file_to_graph_id[b], w, r);
		}
	}

	res.find_source();
	res.find_target();

	agclose(g);
	fclose(f);
	return res;
}  
