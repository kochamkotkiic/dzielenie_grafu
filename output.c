#include "output.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LEN 256
#define MAX_EDGES 1000000


#include <stdio.h>
#include <stdlib.h>
#include "graf.h" 

#define MAX_FILENAME_LEN 256


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

    // 2. col_index
    int col_len = graph->row_ptr[graph->num_vertices];
    for (int i = 0; i < col_len; i++) {
        if (i != col_len - 1)
            fprintf(fp, "%d;", graph->col_index[i]);
        else
            fprintf(fp, "%d", graph->col_index[i]); // ostatni bez średnika
    }
    fprintf(fp, "\n");

    // 3. row_ptr
    for (int i = 0; i <= graph->num_vertices; i++) {
        if (i != graph->num_vertices)
            fprintf(fp, "%d;", graph->row_ptr[i]);
        else
            fprintf(fp, "%d", graph->row_ptr[i]); // ostatni bez średnika
    }
    fprintf(fp, "\n");


    // 4. neighbors (nowo wygenerowane)
    int neighbors_count = row_ptr_out[graph->num_vertices]; // ile wszystkich sąsiadów
    for (int i = 0; i < neighbors_count; i++) {
        if (i != neighbors_count - 1)
            fprintf(fp, "%d;", neighbors_out[i]);
        else
            fprintf(fp, "%d", neighbors_out[i]); // bez średnika na końcu
    }
    fprintf(fp, "\n");

    // 5. row_ptr dla neighbors
    for (int i = 0; i <= graph->num_vertices; i++) {
        if (i != graph->num_vertices)
            fprintf(fp, "%d;", row_ptr_out[i]);
        else
            fprintf(fp, "%d", row_ptr_out[i]); // bez średnika na końcu
    }
    fprintf(fp, "\n");


    fclose(fp);

    free(neighbors_out);
    free(row_ptr_out);

    
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
        //int start_idx = idx;
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
void print_partition_terminal(Graph *graph, int successful_cuts) {
    // 1. Wypisz liczbę udanych podziałów
    printf("%d\n", successful_cuts);
   

    // Tymczasowe dane na ostatnie 2 sekcje
    int *neighbors_out = NULL;
    int *row_ptr_out = NULL;
    create_neighbors_and_row_ptr_filtered(graph, &neighbors_out, &row_ptr_out);

    // 1. max_vertices
    printf("%d\n", graph->max_vertices);

    // 2. col_index
    int col_len = graph->row_ptr[graph->num_vertices];
    for (int i = 0; i < col_len; i++) {
        if (i != col_len - 1)
            printf( "%d;", graph->col_index[i]);
        else
            printf("%d", graph->col_index[i]); // ostatni bez średnika
    }
    printf( "\n");

    // 3. row_ptr
    for (int i = 0; i <= graph->num_vertices; i++) {
        if (i != graph->num_vertices)
            printf("%d;", graph->row_ptr[i]);
        else
            printf( "%d", graph->row_ptr[i]); // ostatni bez średnika
    }
    printf("\n");


    // 4. neighbors (nowo wygenerowane)
    int neighbors_count = row_ptr_out[graph->num_vertices]; // ile wszystkich sąsiadów
    for (int i = 0; i < neighbors_count; i++) {
        if (i != neighbors_count - 1)
            printf("%d;", neighbors_out[i]);
        else
            printf( "%d", neighbors_out[i]); // bez średnika na końcu
    }
    printf( "\n");

    // 5. row_ptr dla neighbors
    for (int i = 0; i <= graph->num_vertices; i++) {
        if (i != graph->num_vertices)
            printf("%d;", row_ptr_out[i]);
        else
            printf( "%d", row_ptr_out[i]); 
    }
    printf( "\n");

    free(neighbors_out);
    free(row_ptr_out);
    
}
void save_graph_to_binary(Graph *graph, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Nie mozna otworzyc pliku do zapisu");
        return;
    }

    // Utwórz neighbors_out i row_ptr_out
    int neighbors_out[MAX_EDGES];
    int row_ptr_out[MAX_VERTICES + 1];

    int neighbor_pos = 0;
    row_ptr_out[0] = 0;

    for (int i = 0; i < graph->num_vertices; i++) {
        //int start_pos = neighbor_pos;
        bool has_neighbors = false;
        for (int j = 0; j < graph->neighbor_count[i]; j++) {
            int neighbor = graph->neighbors[i][j];
            if (neighbor > i) { // Zapobiegamy podwójnym krawędziom
                if (!has_neighbors) {
                    neighbors_out[neighbor_pos++] = i;
                    has_neighbors = true;
                }
                neighbors_out[neighbor_pos++] = neighbor;
            }
        }
        row_ptr_out[i + 1] = neighbor_pos;
    }

    int neighbors_count = row_ptr_out[graph->num_vertices];

    // 1. Zapisz max_vertices
    fwrite(&(graph->max_vertices), sizeof(int), 1, fp);

    // 2. Zapisz col_index
    int col_index_size = graph->num_vertices;
    fwrite(graph->col_index, sizeof(int), col_index_size, fp);

    // 3. Zapisz row_ptr
    fwrite(graph->row_ptr, sizeof(int), graph->num_vertices + 1, fp);

    // 4. Zapisz neighbors_out
    fwrite(neighbors_out, sizeof(int), neighbors_count, fp);

    // 5. Zapisz row_ptr_out
    fwrite(row_ptr_out, sizeof(int), graph->num_vertices + 1, fp);

    fclose(fp);
}
