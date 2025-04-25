#include "graf.h"
#include "partitions.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uzycie: %s <plik_z_grafem>\n", argv[0]);
        return 1;
    }

    Graph graph;
    init_graph(&graph);
    
    // Wczytaj graf
    if (load_graph_from_csrrg(&graph, argv[1]) != 0) {
        fprintf(stderr, "Blad wczytywania grafu\n");
        return 1;
    }
    
    // Sprawdź spójność
    //printf("Graf jest %s\n", is_connected(&graph) ? "spojny" : "niespojny");
    print_graph(&graph);
    // Podział grafu
    int group1[MAX_VERTICES], group2[MAX_VERTICES];
    int group1_size = 0, group2_size = 0;
    int margin = 2; // Dopuszczalna różnica wielkości grup
    
    partition_graph(&graph, group1, &group1_size, group2, &group2_size, margin);
    
    // Wypisz wynik podziału
    printf("\nWynik podzialu:\n");
    printf("Grupa 1 (%d wierzcholkow): ", group1_size);
    for (int i = 0; i < group1_size; i++) {
        printf("%d ", group1[i]);
    }
    
    printf("\nGrupa 2 (%d wierzcholkow): ", group2_size);
    for (int i = 0; i < group2_size; i++) {
        printf("%d ", group2[i]);
    }
    
    printf("\n\nSpojnosc grup:\n");
    //printf("Grupa 1 jest %s\n", is_group_connected(&graph, 1) ? "spójna" : "niespójna");
    //printf("Grupa 2 jest %s\n", is_group_connected(&graph, 2) ? "spójna" : "niespójna");
    print_graph(&graph);
    free_graph(&graph);
    return 0;
}