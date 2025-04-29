#include "partitions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

// ====================== FUNKCJE POMOCNICZE ======================

void dfs_mark(Graph *graph, int v, bool visited[], int component[], int current_component) {
    visited[v] = true;
    component[v] = current_component;
    for (int i = 0; i < graph->neighbor_count[v]; i++) {
        int neighbor = graph->neighbors[v][i];
        if (!visited[neighbor]) {
            dfs_mark(graph, neighbor, visited, component, current_component);
        }
    }
}

// ====================== ANALIZA SKŁADOWYCH ======================

void find_connected_components(Graph *graph) {
    bool visited[MAX_VERTICES] = {false};
    graph->num_components = 0;

    for (int i = 0; i < graph->num_vertices; i++) {
        if (!visited[i]) {
            dfs_mark(graph, i, visited, graph->component, graph->num_components);
            graph->num_components++;
        }
    }
}

// ====================== DIJKSTRA ======================

void dijkstra(Graph *graph, int start) {
    int dist[MAX_VERTICES];
    bool visited[MAX_VERTICES] = {false};

    for (int i = 0; i < graph->num_vertices; i++) {
        dist[i] = INF;
    }
    dist[start] = 0;

    for (int count = 0; count < graph->num_vertices - 1; count++) {
        int u = -1;
        int min_dist = INF;
        for (int v = 0; v < graph->num_vertices; v++) {
            if (!visited[v] && dist[v] < min_dist) {
                min_dist = dist[v];
                u = v;
            }
        }

        if (u == -1) break;

        visited[u] = true;
        for (int i = 0; i < graph->neighbor_count[u]; i++) {
            int v = graph->neighbors[u][i];
            if (!visited[v] && dist[u] + 1 < dist[v]) {
                dist[v] = dist[u] + 1;
            }
        }
    }

    int max_dist = 0;
    for (int i = 0; i < graph->num_vertices; i++) {
        if (dist[i] != INF && dist[i] > max_dist) {
            max_dist = dist[i];
        }
    }
    graph->max_distances[start] = max_dist;
}

// ====================== FUNKCJE DO SPÓJNOŚCI ======================

bool is_component_connected(Graph *graph, const bool in_component[]) {
    int start = -1;
    for (int i = 0; i < graph->num_vertices; i++) {
        if (in_component[i]) {
            start = i;
            break;
        }
    }
    if (start == -1) return true;

    bool visited[MAX_VERTICES] = {false};
    int stack[MAX_VERTICES];
    int stack_size = 0;
    int visited_count = 0;
    
    stack[stack_size++] = start;
    visited[start] = true;
    
    while (stack_size > 0) {
        int current = stack[--stack_size];
        visited_count++;
        
        for (int i = 0; i < graph->neighbor_count[current]; i++) {
            int neighbor = graph->neighbors[current][i];
            if (in_component[neighbor] && !visited[neighbor]) {
                visited[neighbor] = true;
                stack[stack_size++] = neighbor;
            }
        }
    }
    
    int component_size = 0;
    for (int i = 0; i < graph->num_vertices; i++) {
        if (in_component[i]) component_size++;
    }
    
    return visited_count == component_size;
}

// ====================== BALANSOWANIE GRUP ======================

bool balance_groups(Graph *graph, int group1[], int group1_size, 
                   int group2[], int group2_size, int margin) {
    typedef struct {
        int vertex;
        int max_dist;
    } VertexInfo;
    
    // Przygotuj posortowaną listę wierzchołków z grupy 1
    VertexInfo vertices[MAX_VERTICES];
    int count = 0;
    
    for (int i = 0; i < group1_size; i++) {
        int v = group1[i];
        vertices[count].vertex = v;
        vertices[count].max_dist = graph->max_distances[v];
        count++;
    }
    
    // Sortowanie przez wstawianie
    for (int i = 1; i < count; i++) {
        VertexInfo key = vertices[i];
        int j = i - 1;
        while (j >= 0 && vertices[j].max_dist > key.max_dist) {
            vertices[j+1] = vertices[j];
            j--;
        }
        vertices[j+1] = key;
    }

    // Próbuj przenosić wierzchołki od najmniejszego max_distance
    for (int i = 0; i < count; i++) {
        int v = vertices[i].vertex;
        
        // Sprawdź czy wierzchołek nadal jest w grupie 1
        bool in_group1 = false;
        for (int j = 0; j < group1_size; j++) {
            if (group1[j] == v) {
                in_group1 = true;
                break;
            }
        }
        if (!in_group1) continue;
        
        // Tymczasowo przenieś do grupy 2
        graph->group_assignment[v] = 2;
        
        // Sprawdź spójność grupy 1 bez tego wierzchołka
        bool group1_connected = true;
        if (group1_size > 1) {
            bool group1_included[MAX_VERTICES] = {false};
            for (int j = 0; j < group1_size; j++) {
                if (group1[j] != v) {
                    group1_included[group1[j]] = true;
                }
            }
            group1_connected = is_component_connected(graph, group1_included);
        }
        
        // Sprawdź spójność grupy 2 z nowym wierzchołkiem
        bool group2_connected = true;
        if (group2_size > 0) {
            bool group2_included[MAX_VERTICES] = {false};
            for (int j = 0; j < group2_size; j++) {
                group2_included[group2[j]] = true;
            }
            group2_included[v] = true;
            group2_connected = is_component_connected(graph, group2_included);
        }
        
        // Przywróć oryginalne przypisanie przed podjęciem decyzji
        graph->group_assignment[v] = 1;
        
        if (group1_connected && group2_connected) {
            // Wykonaj rzeczywiste przeniesienie
            graph->group_assignment[v] = 2;
            group2[(group2_size)++] = v;
            
            // Usuń z grupy 1
            for (int j = 0; j < group1_size; j++) {
                if (group1[j] == v) {
                    for (int k = j; k < group1_size - 1; k++) {
                        group1[k] = group1[k+1];
                    }
                    (group1_size)--;
                    break;
                }
            }
            
            // Sprawdź warunek marginesu
            int size_diff = abs(group1_size - group2_size);
            if (size_diff <= margin) {
                return true;
            }
            
            // Kontynuuj balansowanie
            return balance_groups(graph, group1, group1_size, group2, group2_size, margin);
        }
    }
    
    return false;
}

// ====================== GŁÓWNA LOGIKA PODZIAŁU ======================

bool partition_graph(Graph *graph, int margin) {
    // Inicjalizacja

    
    // Dla każdej składowej próbuj podziału
    for (int comp = 0; comp < graph->num_components; comp++) {
        // Znajdź centralny wierzchołek w składowej
        int center = -1;
        int min_max_dist = INT_MAX;
        for (int i = 0; i < graph->num_vertices; i++) {
            if (graph->component[i] == comp) {
                dijkstra(graph, i);
                if (graph->max_distances[i] < min_max_dist) {
                    min_max_dist = graph->max_distances[i];
                    center = i;
                }
            }
        }
        if (center == -1) continue;
        int group1[MAX_VERTICES], group2[MAX_VERTICES];
        int group1_size = 0, group2_size = 0;
        // Przydziel wierzchołki do grup DFS-em
        bool visited[MAX_VERTICES] = {false};
        int stack[MAX_VERTICES];
        int stack_size = 0;
        int target_size = 0;
        
        // Policz wierzchołki w tej składowej
        for (int i = 0; i < graph->num_vertices; i++) {
            if (graph->component[i] == comp) {
                target_size++;
            }
        }

        if(target_size < 2) continue;
        margin = margin * target_size / 100;
        
        target_size /= 2;
        
        stack[stack_size++] = center;
        visited[center] = true;
        group1[(group1_size)++] = center;
        graph->group_assignment[center] = 1;
        
        while (stack_size > 0 && group1_size < target_size) {
            int current = stack[--stack_size];
            
            int max_dist = -1;
            int next_vertex = -1;
            for (int i = 0; i < graph->neighbor_count[current]; i++) {
                int neighbor = graph->neighbors[current][i];
                if (!visited[neighbor] && graph->component[neighbor] == comp) {
                    if (graph->max_distances[neighbor] > max_dist) {
                        max_dist = graph->max_distances[neighbor];
                        next_vertex = neighbor;
                    }
                }
            }
            
            if (next_vertex != -1) {
                stack[stack_size++] = current;
                stack[stack_size++] = next_vertex;
                visited[next_vertex] = true;
                group1[(group1_size)++] = next_vertex;
                graph->group_assignment[next_vertex] = 1;
            }
        }
        
        // Reszta do grupy 2
        for (int i = 0; i < graph->num_vertices; i++) {
            if (graph->component[i] == comp && !visited[i]) {
                group2[(group2_size)++] = i;
                graph->group_assignment[i] = 2;
            }
        }
        
        // Sprawdź spójność grupy 2
        bool group2_included[MAX_VERTICES] = {false};
        for (int i = 0; i < group2_size; i++) {
            group2_included[group2[i]] = true;
        }
        
        if (!is_component_connected(graph, group2_included)) {
            // Znajdź największą spójną część w grupie 2
            bool largest_component[MAX_VERTICES] = {false};
            int largest_size = 0;
            bool processed[MAX_VERTICES] = {false};
            
            for (int i = 0; i < group2_size; i++) {
                int v = group2[i];
                if (!processed[v]) {
                    bool current_component[MAX_VERTICES] = {false};
                    int current_size = 0;
                    int component_stack[MAX_VERTICES];
                    int cstack_size = 0;
                    
                    component_stack[cstack_size++] = v;
                    current_component[v] = true;
                    processed[v] = true;
                    
                    while (cstack_size > 0) {
                        int cv = component_stack[--cstack_size];
                        current_size++;
                        
                        for (int j = 0; j < graph->neighbor_count[cv]; j++) {
                            int neighbor = graph->neighbors[cv][j];
                            if (graph->group_assignment[neighbor] == 2 && !current_component[neighbor]) {
                                current_component[neighbor] = true;
                                processed[neighbor] = true;
                                component_stack[cstack_size++] = neighbor;
                            }
                        }
                    }
                    
                    if (current_size > largest_size) {
                        largest_size = current_size;
                        memcpy(largest_component, current_component, sizeof(largest_component));
                    }
                }
            }
            
            // Przenieś wierzchołki spoza największej składowej do grupy 1
            int new_group2_size = 0;
            for (int i = 0; i < group2_size; i++) {
                int v = group2[i];
                if (largest_component[v]) {
                    group2[new_group2_size++] = v;
                } else {
                    group1[(group1_size)++] = v;
                    graph->group_assignment[v] = 1;
                }
            }
            group2_size = new_group2_size;
        }
        
       
        // Sprawdź warunek marginesu i ewentualnie balansuj
        int size_diff = abs(group1_size - group2_size);
        if (size_diff > margin) {
            if (balance_groups(graph, group1, group1_size, group2, group2_size, margin)) {
                split_graph(graph);
              
                return true;
            }
        } else {
            split_graph(graph);
          
            return true;
        }
    }
   
    return false;
}

void split_graph(Graph *graph) {
    int new_component_id = graph->num_components; // nowa składowa
    // Aktualizacja przypisań komponentów
    for (int i = 0; i < graph->num_vertices; i++) {
        if (graph->group_assignment[i] == 2) {
            graph->component[i] = new_component_id;
        }
    }
    graph->num_components++;

    // Usuwanie krawędzi między grupami
    for (int i = 0; i < graph->num_vertices; i++) {
        int new_neighbors[MAX_NEIGHBORS];
        int new_neighbor_count = 0;

        for (int j = 0; j < graph->neighbor_count[i]; j++) {
            int neighbor = graph->neighbors[i][j];
            // Zostaw tylko krawędzie wewnątrz tej samej grupy
            if (graph->group_assignment[i] == graph->group_assignment[neighbor]) {
                new_neighbors[new_neighbor_count++] = neighbor;
            }
        }
        
        // Nadpisanie nowej listy sąsiadów
        for (int j = 0; j < new_neighbor_count; j++) {
            graph->neighbors[i][j] = new_neighbors[j];
        }
        graph->neighbor_count[i] = new_neighbor_count;
    }
    for(int i=0; i<graph->num_vertices; i++){
        graph->group_assignment[i] = 0;
    }
}
