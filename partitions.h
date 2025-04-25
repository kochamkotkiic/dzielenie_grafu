#ifndef PARTITIONS_H
#define PARTITIONS_H

#include "graf.h"

// Dodaj nowe definicje
#define MAX_ITERATIONS 1000
#define INF 1000000


static void dfs_mark(Graph *graph, int v, bool visited[], int component[], int current_component);
void find_connected_components(Graph *graph);
void dijkstra(Graph *graph, int start);
bool is_component_connected(Graph *graph, const bool in_component[]);
bool balance_groups(Graph *graph, int group1[], int *group1_size, int group2[], int *group2_size, int margin);
bool partition_graph(Graph *graph, int group1[], int *group1_size, int group2[], int *group2_size, int margin);


#endif // PARTITIONS_H