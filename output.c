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

#include <stdio.h>
#include <stdlib.h>
#include "graf.h" 

#define MAX_FILENAME_LEN 256

int save_binary(Graph *graph, Partition *partitions, int count, char *base_name) {
    char filename[MAX_FILENAME_LEN];
    snprintf(filename, sizeof(filename), "%s", base_name);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Nie można otworzyć pliku do zapisu");
        return -1;
    }

    // Liczby do nagłówka
    int n = graph->num_vertices;  // liczba wierzchołków
    int k = count;            // liczba cięć
    int m = 0;                    // suma wierzchołków we wszystkich częściach

    for (int i = 0; i < k+1 ; i++) {
        m += partitions[i].size;
    }

    // Zapis nagłówka
    if (fwrite(&n, sizeof(int), 1, fp) != 1 ||
        fwrite(&k, sizeof(int), 1, fp) != 1 ||
        fwrite(&m, sizeof(int), 1, fp) != 1) {
        perror("Błąd zapisu nagłówka");
        fclose(fp);
        return -1;
    }

    // Zapis listy wierzchołków w każdej części
    for (int i = 0; i < k +1; i++) {
        if (fwrite(&partitions[i].size, sizeof(int), 1, fp) != 1) {
            perror("Błąd zapisu rozmiaru części");
            fclose(fp);
            return -1;
        }
        if (partitions[i].size > 0) {
            if (fwrite(partitions[i].vertices, sizeof(int), (size_t)partitions[i].size, fp) != (size_t)partitions[i].size) {
                perror("Błąd zapisu listy wierzchołków");
                fclose(fp);
                return -1;
            }
        }
    }

    fclose(fp);
    return 0;
}

int save_to_text(Graph *graph, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Nie można otworzyć pliku do zapisu");
        return -1;
    }

    // 1. Sekcja 1: Maksymalny wymiar grafu
    fprintf(fp, "%d\n", graph->max_vertices);

    // 2. Sekcja 2: col_index (rekonstrukcja z listy sąsiedztwa)
    int edge_count = 0;
    for (int u = 0; u < graph->num_vertices; u++) {
        for (int j = 0; j < graph->neighbor_count[u]; j++) {
            int v = graph->neighbors[u][j];
            if (u < v) { // Zapisz każdą krawędź tylko raz
                if (edge_count++ > 0) fprintf(fp, ";");
                fprintf(fp, "%d", v);
            }
        }
    }
    fprintf(fp, "\n");

    // 3. Sekcja 3: row_ptr (rekonstrukcja z listy sąsiedztwa)
    int offset = 0;
    fprintf(fp, "%d", offset);
    for (int i = 0; i < graph->num_vertices; i++) {
        offset += graph->neighbor_count[i];
        fprintf(fp, ";%d", offset);
    }
    fprintf(fp, "\n");

    // 4. Sekcja 4: group_list (odwrotność convert_csr_to_neighbors)
    // Znajdź liderów grup
    int leaders[MAX_VERTICES] = {0};
    int group_count = 0;
    
    for (int i = 0; i < graph->num_vertices; i++) {
        if (graph->group_assignment[i] > 0) {
            int is_leader = 1;
            // Sprawdź czy wierzchołek nie jest już w liście liderów
            for (int j = 0; j < group_count; j++) {
                if (graph->group_assignment[i] == graph->group_assignment[leaders[j]]) {
                    is_leader = 0;
                    break;
                }
            }
            if (is_leader) {
                leaders[group_count++] = i;
            }
        }
    }

    // Zapisz grupy
    for (int g = 0; g < group_count; g++) {
        if (g > 0) fprintf(fp, ";");
        fprintf(fp, "%d", leaders[g]);
        
        // Znajdź wszystkich członków grupy
        int group_id = graph->group_assignment[leaders[g]];
        for (int v = leaders[g] + 1; v < graph->num_vertices; v++) {
            if (graph->group_assignment[v] == group_id) {
                fprintf(fp, ",%d", v);
            }
        }
    }
    fprintf(fp, "\n");

    // 5. Sekcja 5: group_ptr (odwrotność convert_csr_to_neighbors)
    fprintf(fp, "0");
    int cumulative = 0;
    for (int g = 0; g < group_count; g++) {
        int group_size = 1; // Lider
        int group_id = graph->group_assignment[leaders[g]];
        
        for (int v = leaders[g] + 1; v < graph->num_vertices; v++) {
            if (graph->group_assignment[v] == group_id) {
                group_size++;
            }
        }
        
        cumulative += group_size;
        fprintf(fp, ";%d", cumulative);
    }
    fprintf(fp, "\n");

    fclose(fp);
    return 0;
}
void print_partition_terminal(Graph *graph, int successful_cuts) {
    // 1. Wypisz liczbę udanych podziałów
    printf("%d\n", successful_cuts);
    
    // 2. Sekcja 1: Maksymalny wymiar grafu
    printf("%d\n", graph->max_vertices);

    // 3. Sekcja 2: col_index (rekonstrukcja z listy sąsiedztwa)
    int edge_count = 0;
    for (int u = 0; u < graph->num_vertices; u++) {
        for (int j = 0; j < graph->neighbor_count[u]; j++) {
            int v = graph->neighbors[u][j];
            if (u < v) { // Zapisz każdą krawędź tylko raz
                if (edge_count++ > 0) printf(";");
                printf("%d", v);
            }
        }
    }
    printf("\n");

    // 4. Sekcja 3: row_ptr (rekonstrukcja z listy sąsiedztwa)
    int offset = 0;
    printf("%d", offset);
    for (int i = 0; i < graph->num_vertices; i++) {
        offset += graph->neighbor_count[i];
        printf(";%d", offset);
    }
    printf("\n");

    // 5. Sekcja 4: group_list (odwrotność convert_csr_to_neighbors)
    // Znajdź liderów grup
    int leaders[MAX_VERTICES] = {0};
    int group_count = 0;
    
    for (int i = 0; i < graph->num_vertices; i++) {
        if (graph->group_assignment[i] > 0) {
            int is_leader = 1;
            // Sprawdź czy wierzchołek nie jest już w liście liderów
            for (int j = 0; j < group_count; j++) {
                if (graph->group_assignment[i] == graph->group_assignment[leaders[j]]) {
                    is_leader = 0;
                    break;
                }
            }
            if (is_leader) {
                leaders[group_count++] = i;
            }
        }
    }

    // Zapisz grupy
    for (int g = 0; g < group_count; g++) {
        if (g > 0) printf(";");
        printf("%d", leaders[g]);
        
        // Znajdź wszystkich członków grupy
        int group_id = graph->group_assignment[leaders[g]];
        for (int v = leaders[g] + 1; v < graph->num_vertices; v++) {
            if (graph->group_assignment[v] == group_id) {
                printf(",%d", v);
            }
        }
    }
    printf("\n");

    // 6. Sekcja 5: group_ptr (odwrotność convert_csr_to_neighbors)
    printf("0");
    int cumulative = 0;
    for (int g = 0; g < group_count; g++) {
        int group_size = 1; // Lider
        int group_id = graph->group_assignment[leaders[g]];
        
        for (int v = leaders[g] + 1; v < graph->num_vertices; v++) {
            if (graph->group_assignment[v] == group_id) {
                group_size++;
            }
        }
        
        cumulative += group_size;
        printf(";%d", cumulative);
    }
    printf("\n");
}