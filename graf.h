#ifndef GRAF_H
#define GRAF_H

#include <stdbool.h>

#define MAX_VERTICES 1000
#define MAX_NEIGHBORS 50
//#define INF INT_MAX

typedef struct {
    int max_vertices;
    int num_vertices;
    
    // Reprezentacja listy sąsiedztwa
    int neighbors[MAX_VERTICES][MAX_NEIGHBORS];
    int neighbor_count[MAX_VERTICES];
    
    // Dodatkowe dane
    int max_distances[MAX_VERTICES];
    int group_assignment[MAX_VERTICES];
    
    // Reprezentacja CSR
    int* col_index;
    int* row_ptr;
    int* group_list;
    int* group_ptr;
   
} Graph;

void init_graph(Graph *graph);
void add_edge(Graph *graph, int u, int v);
void convert_csr_to_neighbors(Graph *graph);
int load_graph_from_csrrg(Graph *graph, const char *filename);
bool is_connected(const Graph *graph);
void print_graph(const Graph *graph);
void free_graph(Graph *graph);

#endif // GRAF_H