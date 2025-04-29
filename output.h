#ifndef OUTPUT_H
#define OUTPUT_H

#include "graf.h"  
#include <stdbool.h>

// Struktura przechowująca partycję grafu
typedef struct {
    int *vertices;    // Tablica wierzchołków w partycji
    int size;         // Liczba wierzchołków w partycji
} Partition;

int save_binary(Graph *graph, Partition *partitions, int count, char *base_name);

void save_graph_to_csrrg(Graph *graph, const char *filename);
void create_neighbors_and_row_ptr_filtered(Graph *graph, int **neighbors_out, int **row_ptr_out);
void save_graph_to_binary(Graph *graph, const char *filename);


#endif // OUTPUT_H