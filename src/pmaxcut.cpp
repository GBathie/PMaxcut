#include "gurobi_c++.h"
#include "pmaxcut.h"
#include <vector>
#include <iostream>
#include <cstdio>
#include <algorithm>

using namespace std;

/**
 * @private
 *  Compute the maximum topological cut of a DAG stored as a Graph, as described in IPDPS'18
 * 
 * @param graph	the DAG in Graph format
 * @param cut 	vector that will contain the edges of the cut
 * @param S 	vector that will contain the S set after the cut
 * @param T		vector that will contain the T set after the cut
 * @param res 	double where the max cut value will be stored
 *
 *
 * @return 0 if everything went well, then the result is in the last arg. 1 if there was an error in gurobi.
 */
int get_maxcut_lin(const Graph &graph,
		vector<int> &cut, vector<int> &S, vector<int> &T, double & res)
{
	int source = graph.source_id, target = graph.target_id;
	if (source == -1 || target == -1) return 3;

	cut.clear();
	S.clear();
	T.clear();

	int n = graph.n_vertices();

	try
	{
		GRBEnv env = GRBEnv(true);
		env.set("LogFile", "gurobi.log");
		env.set(GRB_IntParam_OutputFlag, 0);
		env.start();
		GRBModel model = GRBModel(env);
		vector<GRBVar> p;
		for (int i = 0; i < n; ++i)
		{
			p.push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "p_" + to_string(i)));
		}

		GRBLinExpr obj(0);
		for (auto e : graph.edges)
		{
			int a = e->id_from;
			int b = e->id_to;
			auto tmp = p[a] - p[b];
			obj += tmp * e->weight;
			model.addConstr(tmp >= 0);
		}
		model.setObjective(obj, GRB_MAXIMIZE);

		model.addConstr(p[source] == 1);
		model.addConstr(p[target] == 0);

		model.optimize();

		vector<double> pi_values(n,0);
		for (int i = 0; i < n; ++i)
		{
			pi_values[i] = p[i].get(GRB_DoubleAttr_X);
		}
		

		// any rounding in ]0,1[ is OK
		// See paper by Marchal &al
		double w = 0.5; 
		res = 0;
		for (auto e : graph.edges)
		{
			int a = e->id_from;
			int b = e->id_to;
			if (pi_values[a] > w && pi_values[b] <= w)
			{
				res += e->weight;
				cut.push_back(e->id);
			}
		}
		for (int i = 0; i < n; ++i)
		{
			if (pi_values[i] > w)
			{
				S.push_back(i);
			}
			else
			{
				T.push_back(i);
			}
		}
	}
	catch (GRBException e)
	{
		std::cerr << "GRB Error : " << e.getMessage() << '\n';
		return 1;
	}

	return 0;
}

/**
 * @private
 *  Compute the p-maximum topological cut of a DAG stored as a Graph, as described in my APCDM
 * 
 * @param graph	the DAG in Graph format
 * @param p_max 	value of p
 * @param cut 	vector that will contain the edges of the cut
 * @param S 	vector that will contain the S set after the cut
 * @param T		vector that will contain the T set after the cut
 * @param res 	double where the max cut value will be stored
 * @param integral 	true iff we want to solve the ILP, otherwise solve fractional relaxation
 *
 *
 * @return 0 if everything went well, then the result is in the last arg. 1 if there was an error in gurobi.
 */
int get_p_maxcut_lin(const Graph &graph, int p_max,
		vector<int> &cut, vector<int> &S, vector<int> &T, double & res, bool integral)
{
	int source = graph.source_id, target = graph.target_id;
	if (source == -1 || target == -1) return 3;

	cut.clear();
	S.clear();
	T.clear();

	res = -1;
	int n = graph.n_vertices();

	try
	{
		GRBEnv env = GRBEnv(true);
		env.set("LogFile", "gurobi.log");
		env.set(GRB_IntParam_OutputFlag, 0);
		env.start();
		GRBModel model = GRBModel(env);
		vector<GRBVar> p;
		for (int i = 0; i < n; ++i)
		{
			p.push_back(model.addVar(0.0, 1.0, 0.0, (integral) ? GRB_BINARY : GRB_CONTINUOUS, "p_" + to_string(i)));
		}

		GRBLinExpr obj(0);
		GRBLinExpr proc_count(0);
		vector<GRBLinExpr> tmps;
		for (auto e : graph.edges)
		{
			int a = e->id_from;
			int b = e->id_to;
			auto tmp = p[a] - p[b];
			tmps.push_back(tmp);
			obj += tmp * (e->weight);
			if (e->red)
			{
				proc_count += tmp;
			}
			model.addConstr(tmp >= 0);
		}
		model.setObjective(obj, GRB_MAXIMIZE);

		model.addConstr(proc_count <= p_max);
		model.addConstr(p[source] == 1);
		model.addConstr(p[target] == 0);

		model.optimize();

		vector<double> pi_values(n,0);
		for (int i = 0; i < n; ++i)
		{
			pi_values[i] = p[i].get(GRB_DoubleAttr_X);
		}
		
		double opt = model.get(GRB_DoubleAttr_ObjVal);

		S.clear();
		T.clear();
		res = -1;
		if (integral)
		{
			// All values are 0 or 1
			// Get the values of the cut
			res = opt;
			double w = 0.5;
			for (int i = 0; i < n; ++i)
			{
				if (pi_values[i] > w)
					S.push_back(i);
				else
					T.push_back(i);
			}
			for (auto e : graph.edges)
			{
				int a = e->id_from;
				int b = e->id_to;
				if (pi_values[a] > w && pi_values[b] <= w)
				{
					cut.push_back(e->id);
				}
			}
		}
		else
		{
			// Find the rounding that yields the best cut
			// Only keeps these with less than $p$ red edges
			double ma = 0; 
	
			pi_values.push_back(0 + __DBL_EPSILON__);
			pi_values.push_back(1 - __DBL_EPSILON__);
			for	(double w : pi_values)
			{
				w -= 1e-6; // avoid rounding errors.
				cut.clear();
				int n_proc = 0;
				double tmp = 0;

				for (auto e : graph.edges)
				{
					int a = e->id_from;
					int b = e->id_to;
					if (pi_values[a] > w && pi_values[b] <= w)
					{
						tmp += e->weight;
						cut.push_back(e->id);
						if (e->red)
						{
							++n_proc;
						}
					}
				}
				if (tmp > ma && n_proc <= p_max)
				{
					S.clear();
					T.clear();
					ma = tmp;
					for (int i = 0; i < n; ++i)
					{
						if (pi_values[i] > w)
						{
							S.push_back(i);
						}
						else
						{
							T.push_back(i);
						}
					}
				}
				
			}
			res = ma;
		}
	}
	catch (GRBException e)
	{
		std::cerr << "GRB Error : " << e.getMessage() <<  e.getErrorCode() << " int ? " << integral << '\n';
		return 1;
	}
	return 0;
}