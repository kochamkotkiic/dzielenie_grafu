#include "graf.h"
#include "partitions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_PARTITIONS 32
#define MAX_FILENAME_LEN 256

typedef struct {
    int *vertices;
    int size;
    int id;
} Partition;

void print_usage(char *prog_name) {
    printf("Użycie: %s -i <input.csrrg> -o <output_base> -p <liczba_przecięć> -m <margines%%> [-b] [-t]\n", prog_name);
    printf("   -i : Plik wejściowy w formacie CSR (.csrrg)\n");
    printf("   -o : Bazowa nazwa plików wyjściowych\n");
    printf("   -p : Liczba przecięć (tworzy N+1 części, domyślnie 1)\n");
    printf("   -m : Maksymalny %% różnicy wielkości części (domyślnie 10%%)\n");
    printf("   -b : Zapis binarny (domyślnie tekstowy)\n");
    printf("   -t : Wypisz wynik w terminalu\n");
}

void copy_graph_subset(Graph *src, Graph *dst, int *vertices, int count) {
    init_graph(dst);
    if (count > MAX_VERTICES) {
        fprintf(stderr, "Przekroczono maksymalną liczbę wierzchołków\n");
        return;
    }
    
    dst->num_vertices = count;
    dst->max_vertices = src->max_vertices;

    // Mapowanie indeksów z zabezpieczeniem
    int *index_map = malloc(src->num_vertices * sizeof(int));
    if (!index_map) {
        fprintf(stderr, "Błąd alokacji pamięci\n");
        return;
    }

    for (int i = 0; i < src->num_vertices; i++) index_map[i] = -1;
    for (int i = 0; i < count; i++) {
        if (vertices[i] >= src->num_vertices) {
            fprintf(stderr, "Nieprawidłowy indeks wierzchołka: %d\n", vertices[i]);
            free(index_map);
            return;
        }
        index_map[vertices[i]] = i;
    }

    // Bezpieczne kopiowanie sąsiadów
    for (int i = 0; i < count; i++) {
        int original_v = vertices[i];
        dst->neighbor_count[i] = 0;
        
        for (int j = 0; j < src->neighbor_count[original_v] && dst->neighbor_count[i] < MAX_NEIGHBORS; j++) {
            int original_nb = src->neighbors[original_v][j];
            if (original_nb < src->num_vertices && index_map[original_nb] != -1) {
                dst->neighbors[i][dst->neighbor_count[i]++] = index_map[original_nb];
            }
        }
    }
    free(index_map);
}

int recursive_partition(Graph *original, int current_cuts, int max_cuts, float margin, Partition *partitions, int *partition_count) {
    if (current_cuts >= max_cuts || *partition_count >= MAX_PARTITIONS) 
        return 0;
    
    int new_partitions = 0;
    int initial_count = *partition_count;
    
    for (int i = 0; i < initial_count; i++) {
        if (partitions[i].size < 2) continue;

        Graph subgraph;
        copy_graph_subset(original, &subgraph, partitions[i].vertices, partitions[i].size);

        int allowed_diff = (int)ceil((margin / 100.0) * partitions[i].size);
        int g1[MAX_VERTICES], g2[MAX_VERTICES];
        int g1_size = 0, g2_size = 0;
        
        if (!partition_graph(&subgraph, g1, &g1_size, g2, &g2_size, allowed_diff)) {
            free_graph(&subgraph);
            continue;
        }

        // Sprawdź poprawność nowych rozmiarów
        if (g1_size <= 0 || g2_size <= 0 || 
            g1_size > partitions[i].size || g2_size > partitions[i].size) {
            free_graph(&subgraph);
            continue;
        }

        // Mapowanie indeksów z zabezpieczeniem
        int *new_g1 = malloc(g1_size * sizeof(int));
        int *new_g2 = malloc(g2_size * sizeof(int));
        if (!new_g1 || !new_g2) {
            free(new_g1);
            free(new_g2);
            free_graph(&subgraph);
            continue;
        }

        for (int j = 0; j < g1_size; j++) {
            if (g1[j] >= partitions[i].size) {
                free(new_g1);
                free(new_g2);
                free_graph(&subgraph);
                continue;
            }
            new_g1[j] = partitions[i].vertices[g1[j]];
        }

        for (int j = 0; j < g2_size; j++) {
            if (g2[j] >= partitions[i].size) {
                free(new_g1);
                free(new_g2);
                free_graph(&subgraph);
                continue;
            }
            new_g2[j] = partitions[i].vertices[g2[j]];
        }

        free(partitions[i].vertices);
        partitions[i].vertices = new_g1;
        partitions[i].size = g1_size;

        if (*partition_count < MAX_PARTITIONS) {
            partitions[*partition_count].vertices = new_g2;
            partitions[*partition_count].size = g2_size;
            partitions[*partition_count].id = (*partition_count) + 1;
            (*partition_count)++;
            new_partitions++;
        } else {
            free(new_g2);
        }
        
        free_graph(&subgraph);
    }
    
    return new_partitions + recursive_partition(original, current_cuts + 1, max_cuts, margin, partitions, partition_count);
}

int save_binary(Partition *partitions, int count, char *base_name) {
    char filename[MAX_FILENAME_LEN];
    snprintf(filename, sizeof(filename), "%s.bin", base_name);
    
    FILE *fp = fopen(filename, "wb");
    if (!fp) return -1;

    // Nagłówek: liczba części
    fwrite(&count, sizeof(int), 1, fp);
    
    for (int i = 0; i < count; i++) {
        fwrite(&partitions[i].size, sizeof(int), 1, fp);
        fwrite(partitions[i].vertices, sizeof(int), partitions[i].size, fp);
    }
    
    fclose(fp);
    return 0;
}

int save_text(Partition *partitions, int count, Graph *original, char *base_name) {
    for (int i = 0; i < count; i++) {
        char filename[MAX_FILENAME_LEN];
        snprintf(filename, sizeof(filename), "%s_part%d.csrrg", base_name, partitions[i].id);
        
        FILE *fp = fopen(filename, "w");
        if (!fp) return -1;

        // Nagłówek z liczbą wierzchołków
        fprintf(fp, "%d\n", partitions[i].size);
        
        // Lista wierzchołków
        for (int j = 0; j < partitions[i].size; j++) {
            fprintf(fp, "%d", partitions[i].vertices[j]);
            if (j < partitions[i].size - 1) fprintf(fp, " ");
        }
        fprintf(fp, "\n");
        
        fclose(fp);
    }
    return 0;
}

void print_partitions(Partition *partitions, int count, Graph *graph) {
    printf("Liczba części: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("Partycja %d (%d wierzchołków, %.1f%%):\n", 
               partitions[i].id, 
               partitions[i].size,
               (partitions[i].size * 100.0) / graph->num_vertices);
        
        printf("Wierzchołki: [");
        for (int j = 0; j < partitions[i].size; j++) {
            printf("%d", partitions[i].vertices[j]);
            if (j < partitions[i].size - 1) printf(", ");
        }
        printf("]\n");
        
        // Sprawdź spójność
        Graph subgraph;
        copy_graph_subset(graph, &subgraph, partitions[i].vertices, partitions[i].size);
        bool connected = is_connected(&subgraph);
        printf("Spójność: %s\n", connected ? "TAK" : "NIE");
        free_graph(&subgraph);
    }
}

int main(int argc, char **argv) {
    char *input_file = NULL;
    char *output_base = NULL;
    int num_cuts = 1;
    float margin = 10.0;
    int binary_output = 0;
    int terminal = 0;
    
    // Parsowanie argumentów
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i+1 < argc) input_file = argv[++i];
        else if (strcmp(argv[i], "-o") == 0 && i+1 < argc) output_base = argv[++i];
        else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) num_cuts = atoi(argv[++i]);
        else if (strcmp(argv[i], "-m") == 0 && i+1 < argc) margin = atof(argv[++i]);
        else if (strcmp(argv[i], "-b") == 0) binary_output = 1;
        else if (strcmp(argv[i], "-t") == 0) terminal = 1;
        else {
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (!input_file || !output_base || num_cuts < 0 || margin < 0 || margin > 100) {
        print_usage(argv[0]);
        return 1;
    }

    // Wczytaj główny graf
    Graph main_graph;
    init_graph(&main_graph);
    
    if (load_graph_from_csrrg(&main_graph, input_file) != 0) {
        fprintf(stderr, "Błąd wczytywania pliku: %s\n", input_file);
        return 1;
    }

    // Inicjalizacja partycji (początkowo cały graf)
    Partition partitions[MAX_PARTITIONS];
    int partition_count = 1;
    partitions[0].vertices = malloc(main_graph.num_vertices * sizeof(int));
    for (int i = 0; i < main_graph.num_vertices; i++)
        partitions[0].vertices[i] = i;
    partitions[0].size = main_graph.num_vertices;
    partitions[0].id = 0;

    // Rekurencyjny podział
    int total_cuts = recursive_partition(&main_graph, 0, num_cuts, margin, partitions, &partition_count);
    
    if (terminal) {
        printf("\n=== Wynik podziału ===\n");
        printf("Liczba wykonanych przecięć: %d\n", total_cuts);
        printf("Liczba uzyskanych części: %d\n", partition_count);
        print_partitions(partitions, partition_count, &main_graph);
    }

    // Zapis wyników
    if (binary_output) {
        if (save_binary(partitions, partition_count, output_base) != 0) {
            fprintf(stderr, "Błąd zapisu do pliku binarnego\n");
        }
    } else {
        if (save_text(partitions, partition_count, &main_graph, output_base) != 0) {
            fprintf(stderr, "Błąd zapisu do pliku tekstowego\n");
        }
    }

    // Zwolnienie pamięci
    for (int i = 0; i < partition_count; i++) 
        free(partitions[i].vertices);
    free_graph(&main_graph);
    
    return 0;
}