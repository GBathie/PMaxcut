#include <iostream>
#include <iomanip>
#include <experimental/filesystem>

#include "graph.h"
#include "pmaxcut.h"
#include <gvc.h>
#include <map>

namespace fs = std::experimental::filesystem;
using namespace std;

Graph read_graph_from_pegasus(string filename, string time_label="size", string weight_label="size");

/* Compares the value of the maxcut, p-maxcut and an approximation of the p-maxcut
 * on N randomly generated DAGs
 */
void test_n_random(int N, int n_vtx, double connectedness, double w_max, double w_e_max, int p, bool disp_err=false)
{
	cout << "FQty /" << N << " " << p << " MAXCUT LP ILP" << endl;

	int failures = 0;
    for (int i = 0; i < N; ++i)
	{
		Graph test = generate_dag_ss(n_vtx, connectedness, w_max, 0, w_e_max);
		vector<int> cut, s, t;
		double maxcut, LPvalue, ILPvalue;
		
		cerr << test.to_string() << endl;

		int err = get_maxcut_lin(test, cut, s, t, maxcut);
		if (err && disp_err)
			cout << " Error in maxcut !" << endl;


		err = get_p_maxcut_lin(test, p, cut, s, t, LPvalue);
		if (err)
		{
			++failures;
			if (disp_err)
			{
				cout << " Error : " << err << endl;
			}
		}
		cout << "s : ";
		for (int ii:s)
			cout << " " << ii;
		cout << endl;
		cout << "t : ";
		for (int ii:t)
			cout << " " << ii;
		cout << endl;

		cout << "cut : ";
		for (int e:cut)
			cout << " " << e;
		cout << endl;

		err = get_p_maxcut_lin(test, p, cut, s, t, ILPvalue, true);
		if (err)
		{
			++failures;
			if (disp_err)
			{
				cout << " Error : " << err << endl;
			}
		}
		cout << i <<  fixed << setprecision(5) << " " << maxcut << " " << LPvalue << " " << ILPvalue << endl;
	}
	if (disp_err) cout << "Failures : " << failures << " out of " << N << endl;
}

/* Compares the value of the maxcut, p-maxcut and an approximation of the p-maxcut
 * for all the files (assumed to be correctly formatted dot files) in the folder
 * pointed by path.
 * For example files, see the subfolders of the test folder of this project.
 *
 * @param path Path of the folder to test
 * @param p Value of p for the tests
 */
void test_folder_pegasus(string path, int p)
{
	cout << "Folder " << path << " " << p << " MAXCUT LP ILP" << endl;

	int failures = 0;
	int c = 0;
    for (const auto &entry : fs::directory_iterator(path))
	{
		Graph test = read_graph_from_pegasus(entry.path());

		vector<int> cut, s, t;
		double maxcut, LPvalue, ILPvalue;

		int err = get_maxcut_lin(test, cut, s, t, maxcut);

		err = get_p_maxcut_lin(test, p, cut, s, t, LPvalue);
		if (err)
			++failures;

		err = get_p_maxcut_lin(test, p, cut, s, t, ILPvalue, true);
		if (err)
			++failures;

		cout << entry.path().string() <<  fixed << setprecision(5) << " " << maxcut << " " << LPvalue << " " << ILPvalue << endl;
		++c;
	}
}

void test_folder_convert(string path, int p)
{
	cout << "Folder " << path << " " << p << " MAXCUT LP ILP" << endl;

	int failures = 0;
	int c = 0;
    for (const auto &entry : fs::directory_iterator(path))
	{
		Graph test = read_graph_from_file(entry.path(), "", "size", "");
		test = convert_to_SimpleDataFlow(test);

		vector<int> cut, s, t;
		double maxcut, LPvalue, ILPvalue;

		int err = get_maxcut_lin(test, cut, s, t, maxcut);

		err = get_p_maxcut_lin(test, p, cut, s, t, LPvalue);
		if (err)
			++failures;

		err = get_p_maxcut_lin(test, p, cut, s, t, ILPvalue, true);
		if (err)
			++failures;

		cout << entry.path().string() <<  fixed << setprecision(5) << " " << maxcut << " " << LPvalue << " " << ILPvalue << endl;
		++c;
	}
}

/* Test a specific set of folders for the given values of p */
void test_all_folders(vector<int> p_values)
{
	for (int p : p_values)
	{
		// These dataset are already in SDFM
		for (string folder : {"./tests/Pegasus/GENOME",
							  "./tests/Pegasus/LIGO",
							  "./tests/Pegasus/MONTAGE"
							  })
			test_folder_pegasus(folder, p);
		// These dataset need to be converted in SDFM
		for (string folder : {"./tests/randomsets/completeset", 
							  "./tests/randomsets/completeset-v2",
							  "./tests/Pegasus/qr-mumps-trees"
							  })
			test_folder_convert(folder, p);
	}
}

int main()
{
	/*for (int p :{1,3,5,10})
	{
		srand(0);
		test_n_random(1, 10, 0.5, 500, 500, p);
	}*/
	test_all_folders({1,3,5,10});
}


Graph read_graph_from_pegasus(string filename, string time_label, string weight_label)
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
	// Parse nodes : these graphs are already in SDFM, don't care about their weight
	for (auto n = agfstnode(g); n; n = agnxtnode(g,n))
	{
		a = stoi(agnameof(n));
		file_to_graph_id[a] = res.add_vertex(0, 0);
	}
	
	for (auto n = agfstnode(g); n; n = agnxtnode(g,n)) 
	{
		a = stoi(agnameof(n));
		for (auto e = agfstout(g,n); e; e = agnxtout(g,e)) 
		{
			w = 0;
			t = 1;
			r = false;
			b = stoi(agnameof(aghead(e)));

			// Get weight info
			tmp = agget(e, weight_label.data());
			if (tmp != NULL)
				w = stod(tmp);

			tmp = agget(aghead(e), time_label.data());
			if (tmp != NULL)
				t = stod(tmp);

			r = (abs(t) < 1e-6) && (b%2 == 0);
			
			res.add_edge(file_to_graph_id[a], file_to_graph_id[b], w, r);
		}
	}

	res.find_source();
	res.find_target();

	agclose(g);
	fclose(f);
	return res;
}  
