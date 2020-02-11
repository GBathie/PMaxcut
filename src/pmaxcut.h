#pragma once

#include "graph.h"
#include <vector>

/* The following definition allows this code to be used in C code */
#ifdef __cplusplus
extern "C" {  
#endif 


int get_maxcut_lin(const Graph &graph,
		std::vector<int> &cut, std::vector<int> &S, std::vector<int> &T, double &res);


int get_p_maxcut_lin(const Graph &graph, int p_max, 
		std::vector<int> &cut, std::vector<int> &S, std::vector<int> &T, double &res, bool integer = false);

#ifdef __cplusplus  
} // extern "C"  
#endif
