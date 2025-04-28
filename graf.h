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
    int **neighbors;       // każdy wierzchołek ma dynamiczną tablicę sąsiadów
    int *neighbor_count;    // dynamiczna tablica liczby sąsiadów

    
    // Dodatkowe dane
    int *max_distances;
    int *group_assignment;
    int *component; // Przynależność do składowej spójnej
    int num_components;          // Liczba składowych spójnych
    
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
void save_graph_to_csrrg(Graph *graph, const char *filename);
void create_neighbors_and_row_ptr_filtered(Graph *graph, int **neighbors_out, int **row_ptr_out);

#endif // GRAF_H