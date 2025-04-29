#ifndef OUTPUT_H
#define OUTPUT_H

#include "graf.h"  
#include <stdbool.h>


void save_graph_to_csrrg(Graph *graph, const char *filename);
void create_neighbors_and_row_ptr_filtered(Graph *graph, int **neighbors_out, int **row_ptr_out);
void save_graph_to_binary(Graph *graph, const char *filename);
void print_partition_terminal(Graph *graph, int successful_cuts);

#endif // OUTPUT_H