#include "graf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_LINE_LENGTH 2048

// Inicjalizacja grafu i dynamicznych struktur
void init_graph(Graph *graph) {
    graph->max_vertices = 0;
    graph->num_vertices = 0;
    graph->num_components = 0;

    // Inicjalizuj wskaźniki CSR
    graph->col_index = NULL;
    graph->row_ptr = NULL;
    graph->group_list = NULL;
    graph->group_ptr = NULL;

    // Alokuj dynamiczne tablice sąsiadów
    graph->neighbors = malloc(MAX_VERTICES * sizeof(int *));
    graph->neighbor_count = calloc(MAX_VERTICES, sizeof(int));

    // Alokuj dodatkowe tablice dynamiczne
    graph->max_distances = calloc(MAX_VERTICES, sizeof(int));
    graph->group_assignment = calloc(MAX_VERTICES, sizeof(int));
    graph->component = calloc(MAX_VERTICES, sizeof(int));

    // Inicjalizuj listy sąsiadów jako puste
    for (int i = 0; i < MAX_VERTICES; i++) {
        graph->neighbors[i] = NULL;
    }
}

// Dodanie krawędzi (graf nieskierowany) z dynamiczną alokacją listy
void add_edge(Graph *graph, int u, int v) {
    if (u < 0 || u >= MAX_VERTICES || v < 0 || v >= MAX_VERTICES) {
        fprintf(stderr, "Błąd: Indeks wierzchołka poza zakresem u=%d, v=%d\n", u, v);
        return;
    }

    // Dodaj v do sąsiadów u, jeśli jeszcze go nie ma
    for (int i = 0; i < graph->neighbor_count[u]; i++) {
        if (graph->neighbors[u][i] == v) return;
    }
    int new_count = graph->neighbor_count[u] + 1;
    graph->neighbors[u] = realloc(graph->neighbors[u], new_count * sizeof(int));
    graph->neighbors[u][new_count - 1] = v;
    graph->neighbor_count[u] = new_count;

    // Dodaj u do sąsiadów v, jeśli jeszcze go nie ma
    for (int i = 0; i < graph->neighbor_count[v]; i++) {
        if (graph->neighbors[v][i] == u) return;
    }
    new_count = graph->neighbor_count[v] + 1;
    graph->neighbors[v] = realloc(graph->neighbors[v], new_count * sizeof(int));
    graph->neighbors[v][new_count - 1] = u;
    graph->neighbor_count[v] = new_count;

    // Aktualizacja liczby wierzchołków
    if (u + 1 > graph->num_vertices) graph->num_vertices = u + 1;
    if (v + 1 > graph->num_vertices) graph->num_vertices = v + 1;
}

// Konwersja danych CSR do dynamicznej listy sąsiedztwa
void convert_csr_to_neighbors(Graph *graph) {
    // Zwolnij poprzednie listy i zainicjalizuj puste
    for (int i = 0; i < MAX_VERTICES; i++) {
        free(graph->neighbors[i]);
        graph->neighbors[i] = NULL;
        graph->neighbor_count[i] = 0;
    }
    memset(graph->group_assignment, 0, MAX_VERTICES * sizeof(int));

    // Policz liczbę grup
    int total_groups = 0;
    while (total_groups < MAX_VERTICES &&
           (total_groups == 0 || graph->group_ptr[total_groups] != 0)) {
        total_groups++;
    }

    // Dla każdej grupy: utwórz krawędzie lider -> członkowie
    for (int g = 0; g < total_groups; g++) {
        int start_idx = graph->group_ptr[g];
        int end_idx = (g < total_groups - 1)
                      ? graph->group_ptr[g + 1] - 1
                      : start_idx - 1;
        if (end_idx < start_idx) continue;

        int leader = graph->group_list[start_idx];
        graph->group_assignment[leader] = g + 1;

        for (int i = start_idx + 1; i <= end_idx; i++) {
            int member = graph->group_list[i];
            add_edge(graph, leader, member);
            graph->group_assignment[member] = g + 1;
        }
    }
}

// Wczytanie grafu z pliku CSRRg do struktury CSR i konwersja
int load_graph_from_csrrg(Graph *graph, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Nie można otworzyć pliku");
        return -1;
    }

    init_graph(graph);
    char line[MAX_LINE_LENGTH];
    int *temp = NULL;
    int count = 0;

    // 1: max_vertices
    if (!fgets(line, sizeof(line), fp)) goto err;
    graph->max_vertices = atoi(line);

    // 2: col_index
    if (!fgets(line, sizeof(line), fp)) goto err;
    temp = malloc(MAX_LINE_LENGTH * sizeof(int));
    count = 0;
    for (char *tok = strtok(line, ";"); tok; tok = strtok(NULL, ";")) {
        temp[count++] = atoi(tok);
    }
    graph->col_index = malloc(count * sizeof(int));
    memcpy(graph->col_index, temp, count * sizeof(int));
    free(temp);
    temp = NULL;

    // 3: row_ptr
    if (!fgets(line, sizeof(line), fp)) goto err;
    temp = malloc(MAX_LINE_LENGTH * sizeof(int));
    count = 0;
    for (char *tok = strtok(line, ";"); tok; tok = strtok(NULL, ";")) {
        temp[count++] = atoi(tok);
    }
    graph->row_ptr = malloc(count * sizeof(int));
    memcpy(graph->row_ptr, temp, count * sizeof(int));
    graph->num_vertices = count - 1;
    free(temp);
    temp = NULL;

    // 4: group_list
    if (!fgets(line, sizeof(line), fp)) goto err;
    temp = malloc(MAX_LINE_LENGTH * sizeof(int));
    count = 0;
    for (char *tok = strtok(line, ";"); tok; tok = strtok(NULL, ";")) {
        temp[count++] = atoi(tok);
    }
    graph->group_list = malloc(count * sizeof(int));
    memcpy(graph->group_list, temp, count * sizeof(int));
    free(temp);
    temp = NULL;

    // 5: group_ptr
    if (!fgets(line, sizeof(line), fp)) goto err;
    temp = malloc(MAX_LINE_LENGTH * sizeof(int));
    count = 0;
    for (char *tok = strtok(line, ";"); tok; tok = strtok(NULL, ";")) {
        temp[count++] = atoi(tok);
    }
    graph->group_ptr = malloc(count * sizeof(int));
    memcpy(graph->group_ptr, temp, count * sizeof(int));
    free(temp);

    fclose(fp);
    convert_csr_to_neighbors(graph);
    return 0;

err:
    if (fp) fclose(fp);
    free_graph(graph);
    return -1;
}

// DFS na dynamicznej liście sąsiadów
void dfs(const Graph *graph, int v, bool visited[]) {
    visited[v] = true;
    for (int i = 0; i < graph->neighbor_count[v]; i++) {
        int nb = graph->neighbors[v][i];
        if (!visited[nb]) dfs(graph, nb, visited);
    }
}

bool is_connected(const Graph *graph) {
    if (graph->num_vertices == 0) return true;
    bool *visited = calloc(graph->num_vertices, sizeof(bool));
    dfs(graph, 0, visited);
    for (int i = 0; i < graph->num_vertices; i++) {
        if (!visited[i]) {
            free(visited);
            return false;
        }
    }
    free(visited);
    return true;
}

void print_graph(const Graph *graph) {
    printf("Graf (wierzcholki: %d/%d):\n", graph->num_vertices, graph->max_vertices);
    printf("\nLista sasiedztwa:\n");
    for (int i = 0; i < graph->num_vertices; i++) {
        printf("%d (grupa %d): ", i, graph->component[i]);
        for (int j = 0; j < graph->neighbor_count[i]; j++) {
            printf("%d ", graph->neighbors[i][j]);
        }
        printf("\n");
    }
    printf("liczba kompartamentow: %d\n", graph->num_components);
    for(int i = 0; i<graph->num_vertices; i++){
        printf("%d: %d\n", i, graph->component[i]);
    }
    printf("\n");
}

// Zwalnianie całej pamięci grafu
void free_graph(Graph *graph) {
    if (graph->neighbors) {
        for (int i = 0; i < MAX_VERTICES; i++) {
            free(graph->neighbors[i]);
        }
        free(graph->neighbors);
    }
    free(graph->neighbor_count);
    free(graph->max_distances);
    free(graph->group_assignment);
    free(graph->component);

    free(graph->col_index);
    free(graph->row_ptr);
    free(graph->group_list);
    free(graph->group_ptr);

    graph->neighbors = NULL;
    graph->neighbor_count = NULL;
    graph->max_distances = NULL;
    graph->group_assignment = NULL;
    graph->component = NULL;
    graph->col_index = NULL;
    graph->row_ptr = NULL;
    graph->group_list = NULL;
    graph->group_ptr = NULL;
    graph->num_vertices = 0;
    graph->max_vertices = 0;
    graph->num_components = 0;
}

void save_graph_to_csrrg(Graph *graph, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Blad otwierania pliku do zapisu: %s\n", filename);
        return;
    }

    // Tymczasowe dane na ostatnie 2 sekcje
    int *neighbors_out = NULL;
    int *row_ptr_out = NULL;
    create_neighbors_and_row_ptr_filtered(graph, &neighbors_out, &row_ptr_out);

    // 1. max_vertices
    fprintf(fp, "%d\n", graph->max_vertices);

    // 2. col_index (oryginalne z pliku)
    for (int i = 0; i < graph->row_ptr[graph->num_vertices]; i++) {
        fprintf(fp, "%d;", graph->col_index[i]);
    }
    fprintf(fp, "\n");

    // 3. row_ptr (oryginalne z pliku)
    for (int i = 0; i <= graph->num_vertices; i++) {
        fprintf(fp, "%d;", graph->row_ptr[i]);
    }
    fprintf(fp, "\n");

    // 4. neighbors (nowo wygenerowane)
    int neighbors_count = row_ptr_out[graph->num_vertices]; // ile elementów
    for (int i = 0; i < neighbors_count; i++) {
        fprintf(fp, "%d;", neighbors_out[i]);
    }
    fprintf(fp, "\n");

    // 5. row_ptr dla neighbors
    for (int i = 0; i <= graph->num_vertices; i++) {
        fprintf(fp, "%d;", row_ptr_out[i]);
    }
    fprintf(fp, "\n");

    fclose(fp);

    free(neighbors_out);
    free(row_ptr_out);

    printf("Zapisano graf do pliku: %s\n", filename);
}

void create_neighbors_and_row_ptr_filtered(Graph *graph, int **neighbors_out, int **row_ptr_out) {
    int n = graph->num_vertices;
    int estimated_size = 0;

    // Najpierw policz, ile będzie potrzebne pamięci (przybliżenie)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < graph->neighbor_count[i]; j++) {
            int neighbor = graph->neighbors[i][j];
            if (neighbor > i) {
                estimated_size++;
            }
        }
        if (estimated_size > 0) {
            estimated_size++; // bo dodamy też indeks wierzchołka na początku grupy
        }
    }

    *neighbors_out = (int *)malloc(estimated_size * sizeof(int));
    *row_ptr_out = (int *)malloc((n + 1) * sizeof(int)); // +1 standardowo

    if (*neighbors_out == NULL || *row_ptr_out == NULL) {
        fprintf(stderr, "Blad alokacji pamieci w create_neighbors_and_row_ptr_filtered\n");
        exit(1);
    }

    int idx = 0;
    (*row_ptr_out)[0] = 0; // początek

    for (int i = 0; i < n; i++) {
        int start_idx = idx;
        int neighbors_added = 0;

        // Sprawdź sąsiadów i filtruj
        for (int j = 0; j < graph->neighbor_count[i]; j++) {
            int neighbor = graph->neighbors[i][j];
            if (neighbor > i) { // zapisz tylko jeśli neighbor ma większy indeks
                if (neighbors_added == 0) {
                    (*neighbors_out)[idx++] = i; // dodaj wierzchołek tylko raz na początek
                }
                (*neighbors_out)[idx++] = neighbor;
                neighbors_added++;
            }
        }

        if (neighbors_added == 0) {
            // wierzchołek "pusty", nic nie dodano
            (*row_ptr_out)[i + 1] = (*row_ptr_out)[i];
        } else {
            // zapisano coś
            (*row_ptr_out)[i + 1] = idx;
        }
    }
}
