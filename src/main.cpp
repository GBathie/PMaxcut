#include <iostream>
#include <iomanip>
#include <experimental/filesystem>

#include "graph.h"
#include "pmaxcut.h"

namespace fs = std::experimental::filesystem;
using namespace std;

/* Compares the value of the maxcut, p-maxcut and an approximation of the p-maxcut
 * for all the files (assumed to be correctly formatted dot files) in the folder
 * pointed by path.
 * For example files, see the subfolders of the test folder of this project.
 *
 * @param path Path of the folder to test
 * @param p Value of p for the tests
 * @param disp_err [optional] Display additional information about failures
 */
void test_folder(string path, int p, bool disp_err = false)
{
	cout << "Folder " << path << " " << p << "MAXCUT LP ILP" << endl;

	int failures = 0;
	int c = 0;
    for (const auto &entry : fs::directory_iterator(path))
	{
		Graph test = read_graph_from_file(entry.path(), "", "size", "");
		test = convert_to_SimpleDataFlow(test);
		vector<int> cut, s, t;
		double maxcut, LPvalue, ILPvalue;

		int err = get_maxcut_lin(test, cut, s, t, maxcut);
		if (err && disp_err)
		{
			cout << " Error in maxcut !" << endl;
		}
		err = get_p_maxcut_lin(test, p, cut, s, t, LPvalue);
		if (err)
		{
			++failures;
			if (disp_err)
			{
				cout << entry.path().string() << endl;
				cout << " Error : " << err << endl;
			}
		}
		err = get_p_maxcut_lin(test, p, cut, s, t, ILPvalue, true);
		if (err)
		{
			++failures;
			if (disp_err)
			{
				cout << entry.path().string() << endl;
				cout << " Error : " << err << endl;
			}
		}
		cout << entry.path().string() <<  fixed << setprecision(5) << " " << maxcut << " " << LPvalue << " " << ILPvalue << endl;
		++c;
	}
	if (disp_err) cout << "Failures : " << failures << " out of " << c << endl;
}


/* Test a specific set of folders for the given values of p */
void test_all_folders(vector<int> p_values)
{
	for (int p : p_values)
	{
		for (string folder : {"./tests/randomsets/completeset", 
							  "./tests/randomsets/completeset-v2", 
							  "./tests/Pegasus/50nodes",
							  "./tests/Pegasus/GENOME",
							  "./tests/Pegasus/LIGO",
							  "./tests/Pegasus/MONTAGE"
							  })
			test_folder(folder, p);
	}
}

int main()
{
	test_all_folders({1,3,5,10});
}