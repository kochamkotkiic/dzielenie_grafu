#include "graf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_LINE_LENGTH 2048

void init_graph(Graph *graph) {
    graph->max_vertices = 0;
    graph->num_vertices = 0;
    
    // Inicjalizacja listy sąsiedztwa
    for (int i = 0; i < MAX_VERTICES; i++) {
        graph->neighbor_count[i] = 0;
        graph->max_distances[i] = 0;
        graph->group_assignment[i] = 0; // 0 - nieprzypisany
        for (int j = 0; j < MAX_NEIGHBORS; j++) {
            graph->neighbors[i][j] = -1;
        }
    }
    
    // Inicjalizacja struktur CSR
    graph->col_index = NULL;
    graph->row_ptr = NULL;
    graph->group_list = NULL;
    graph->group_ptr = NULL;
}

void add_edge(Graph *graph, int u, int v) {
    // Sprawdź czy można dodać krawędź
    if (u >= MAX_VERTICES || v >= MAX_VERTICES) {
        fprintf(stderr, "Błąd: Przekroczono maksymalną liczbę wierzchołków u=%d, v=%d\n", u, v);
        return;
    }

    // Dodaj v do sąsiadów u (jeśli jest miejsce)
    if (graph->neighbor_count[u] < MAX_NEIGHBORS) {
        bool exists = false;
        for (int i = 0; i < graph->neighbor_count[u]; i++) {
            if (graph->neighbors[u][i] == v) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            graph->neighbors[u][graph->neighbor_count[u]++] = v;
        }
    } else {
        fprintf(stderr, "Ostrzeżenie: Osiągnięto maksymalną liczbę sąsiadów dla wierzchołka %d\n", u);
    }

    // Dodaj u do sąsiadów v (graf nieskierowany)
    if (graph->neighbor_count[v] < MAX_NEIGHBORS) {
        bool exists = false;
        for (int i = 0; i < graph->neighbor_count[v]; i++) {
            if (graph->neighbors[v][i] == u) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            graph->neighbors[v][graph->neighbor_count[v]++] = u;
        }
    }

    // Aktualizacja liczby wierzchołków
    if (u + 1 > graph->num_vertices) graph->num_vertices = u + 1;
    if (v + 1 > graph->num_vertices) graph->num_vertices = v + 1;
}
void convert_csr_to_neighbors(Graph *graph) {
    // Wyczyszczenie listy sąsiedztwa
    for (int i = 0; i < MAX_VERTICES; i++) {
        graph->neighbor_count[i] = 0;
        for (int j = 0; j < MAX_NEIGHBORS; j++) {
            graph->neighbors[i][j] = -1;
        }
        graph->group_assignment[i] = 0; // reset przypisania do grupy
    }
    
    // Przetwarzanie grup na podstawie group_ptr i group_list
    for (int g = 0; g < MAX_VERTICES - 1; g++) {
        int start_idx = graph->group_ptr[g];
        int end_idx = graph->group_ptr[g + 1] - 1;

        // Sprawdzenie poprawności zakresu
        if (end_idx < start_idx || (g > 0 && graph->group_ptr[g] == 0)) break;

        int leader = graph->group_list[start_idx];

        // Przypisz lidera do grupy g+1
        graph->group_assignment[leader] = g + 1;

        // Dodaj połączenia lidera z innymi i przypisz im tę samą grupę
        for (int i = start_idx + 1; i <= end_idx; i++) {
            int neighbor = graph->group_list[i];
            add_edge(graph, leader, neighbor);

            // Przypisz również sąsiada do grupy
            graph->group_assignment[neighbor] = g + 1;
        }
    }
}


int load_graph_from_csrrg(Graph *graph, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Nie można otworzyć pliku");
        return -1;
    }

    init_graph(graph); // Reset grafu przed wczytaniem

    char line[MAX_LINE_LENGTH];
    int *temp_array = NULL;
    int count = 0;

    // Sekcja 1: Maksymalna liczba wierzchołków
    if (!fgets(line, sizeof(line), fp)) goto error;
    graph->max_vertices = atoi(line);

    // Sekcja 2: col_index
    if (!fgets(line, sizeof(line), fp)) goto error;
    temp_array = malloc(MAX_LINE_LENGTH * sizeof(int));
    count = 0;
    char *token = strtok(line, ";");
    while (token && count < MAX_LINE_LENGTH) {
        temp_array[count++] = atoi(token);
        token = strtok(NULL, ";");
    }
    graph->col_index = malloc(count * sizeof(int));
    memcpy(graph->col_index, temp_array, count * sizeof(int));
    free(temp_array);
    temp_array = NULL;

    // Sekcja 3: row_ptr
    if (!fgets(line, sizeof(line), fp)) goto error;
    temp_array = malloc(MAX_LINE_LENGTH * sizeof(int));
    count = 0;
    token = strtok(line, ";");
    while (token && count < MAX_LINE_LENGTH) {
        temp_array[count++] = atoi(token);
        token = strtok(NULL, ";");
    }
    graph->row_ptr = malloc(count * sizeof(int));
    memcpy(graph->row_ptr, temp_array, count * sizeof(int));
    graph->num_vertices = count - 1; // row_ptr ma n+1 elementów
    free(temp_array);
    temp_array = NULL;

    // Sekcja 4: group_list
    if (!fgets(line, sizeof(line), fp)) goto error;
    temp_array = malloc(MAX_LINE_LENGTH * sizeof(int));
    count = 0;
    token = strtok(line, ";");
    while (token && count < MAX_LINE_LENGTH) {
        temp_array[count++] = atoi(token);
        token = strtok(NULL, ";");
    }
    graph->group_list = malloc(count * sizeof(int));
    memcpy(graph->group_list, temp_array, count * sizeof(int));
    free(temp_array);
    temp_array = NULL;

    // Sekcja 5: group_ptr
    if (!fgets(line, sizeof(line), fp)) goto error;
    temp_array = malloc(MAX_LINE_LENGTH * sizeof(int));
    count = 0;
    token = strtok(line, ";");
    while (token && count < MAX_LINE_LENGTH) {
        temp_array[count++] = atoi(token);
        token = strtok(NULL, ";");
    }
    graph->group_ptr = malloc(count * sizeof(int));
    memcpy(graph->group_ptr, temp_array, count * sizeof(int));
    free(temp_array);

    fclose(fp);
    
    // Konwersja formatu CSR do listy sąsiedztwa
    convert_csr_to_neighbors(graph);
    return 0;

error:
    if (fp) fclose(fp);
    if (temp_array) free(temp_array);
    free_graph(graph);
    return -1;
}

void dfs(const Graph *graph, int v, bool visited[]) {
    visited[v] = true;
    for (int i = 0; i < graph->neighbor_count[v]; i++) {
        int neighbor = graph->neighbors[v][i];
        if (!visited[neighbor]) {
            dfs(graph, neighbor, visited);
        }
    }
}

bool is_connected(const Graph *graph) {
    if (graph->num_vertices == 0) return true;

    bool visited[MAX_VERTICES] = {false};
    dfs(graph, 0, visited);

    for (int i = 0; i < graph->num_vertices; i++) {
        if (!visited[i]) return false;
    }
    return true;
}

void print_graph(const Graph *graph) {
    printf("Graf (wierzchołki: %d/%d):\n", graph->num_vertices, graph->max_vertices);
    printf("Struktura CSR:\n");
    printf("col_index: ");
    for (int i = 0; i < graph->row_ptr[graph->num_vertices]; i++) {
        printf("%d ", graph->col_index[i]);
    }
    printf("\nrow_ptr: ");
    for (int i = 0; i <= graph->num_vertices; i++) {
        printf("%d ", graph->row_ptr[i]);
    }
    
    printf("\nLista sąsiedztwa:\n");
    for (int i = 0; i < graph->num_vertices; i++) {
        printf("%d (grupa %d): ", i, graph->group_assignment[i]);
        for (int j = 0; j < graph->neighbor_count[i]; j++) {
            printf("%d ", graph->neighbors[i][j]);
        }
        printf("\n");
    }
}

void free_graph(Graph *graph) {
    if (graph->col_index) free(graph->col_index);
    if (graph->row_ptr) free(graph->row_ptr);
    if (graph->group_list) free(graph->group_list);
    if (graph->group_ptr) free(graph->group_ptr);
    
    graph->col_index = NULL;
    graph->row_ptr = NULL;
    graph->group_list = NULL;
    graph->group_ptr = NULL;
}