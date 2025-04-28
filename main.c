#include "graf.h"
#include "partitions.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uzycie: %s <plik_z_grafem> <margines> <liczba_podzialow>\n", argv[0]);
        return 1;
    }

    Graph graph;
    init_graph(&graph);
    
    // Wczytaj graf
    if (load_graph_from_csrrg(&graph, argv[1]) != 0) {
        fprintf(stderr, "Blad wczytywania grafu\n");
        return 1;
    }
    
    // Wczytaj margines i liczbe podziałów
    int margin = atoi(argv[2]);
    int num_partitions = atoi(argv[3]);
    
    printf("Wczytano graf z pliku: %s\n", argv[1]);
    printf("Dopuszczalny margines: %d\n", margin);
    printf("Liczba podziałów do wykonania: %d\n", num_partitions);

    // Zmienna do liczenia wykonanych podziałów
    int partition_count = 0;
    bool czy_podzial = true;
    // Pętla wykonująca podziały
    find_connected_components(&graph);
    while(czy_podzial && partition_count < num_partitions) {
        printf("\nPodzial #%d:\n", partition_count);

        // Tablice na grupy
        int group1[MAX_VERTICES], group2[MAX_VERTICES];
        int group1_size = 0, group2_size = 0;

        // Wykonaj podział grafu
        czy_podzial = partition_graph(&graph, group1, &group1_size, group2, &group2_size, margin);
        print_graph(&graph);
        // Wypisz wynik podziału
        
        printf("Grupa 1 (%d wierzcholkow): ", group1_size);
        for (int j = 0; j < group1_size; j++) {
            printf("%d ", group1[j]);
        }
        
        printf("\nGrupa 2 (%d wierzcholkow): ", group2_size);
        for (int j = 0; j < group2_size; j++) {
            printf("%d ", group2[j]);
        }
        
        
        //printf("\n\nSpojnosc grup:\n");
        // Zakładając, że masz funkcję `is_group_connected`, sprawdzamy spójność grup
        // printf("Grupa 1 jest %s\n", is_group_connected(&graph, 1) ? "spójna" : "niespójna");
        // printf("Grupa 2 jest %s\n", is_group_connected(&graph, 2) ? "spójna" : "niespójna");

        // Zwiększamy licznik wykonanych podziałów
        partition_count++;
    }
    
    // Wypisz podsumowanie
    printf("\nWykonano %d podzialow.\n", partition_count);
    
    // Zwolnienie pamięci
    free_graph(&graph);
    
    return 0;
}
