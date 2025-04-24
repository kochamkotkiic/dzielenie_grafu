#ifndef PARTITIONS_H
#define PARTITIONS_H

#include "graf.h"

// Dodaj nowe definicje
#define MAX_ITERATIONS 1000
#define INF 1000000


void dijkstra(Graph *graph, int start);
int find_central_vertex(Graph *graph);
void dfs_select_group1(Graph *graph, int start, int group1[], int *group1_size);
bool is_group_connected(const Graph *graph, int group_id);
void partition_graph(Graph *graph, int group1[], int *group1_size, int group2[], int *group2_size, int margin);
bool balance_groups(Graph *graph, int group1[], int *group1_size, int group2[], int *group2_size, int margin);

#endif // PARTITIONS_H