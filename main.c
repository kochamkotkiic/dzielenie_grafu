#include "graf.h"
#include "partitions.h"
#include "output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void print_partition_terminal(Graph *graph, int cut_count);
void print_usage() {
    printf("Użycie: -i <input.csrrg> -o <output_base> -p <liczba_przecięć> -m <margines%%> [-b] [-t]\n");
    printf("   -i : Plik wejściowy w formacie CSR (.csrrg)\n");
    printf("   -o : Bazowa nazwa plików wyjściowych\n");
    printf("   -p : Liczba przecięć (tworzy N+1 części, domyślnie 1)\n");
    printf("   -m : Maksymalny %% różnicy wielkości części (domyślnie 10%%)\n");
    printf("   -b : Zapis binarny (domyślnie tekstowy)\n");
    printf("   -t : Wypisz wynik w terminalu\n");
}
#include <stdio.h>

void read_binary(char *folder, char *filename) {
    // Tworzymy pełną ścieżkę do pliku
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", folder, filename);
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        perror("Nie można otworzyć pliku");
        return;
    }

    int n, k, m;
    // Odczyt nagłówka
    if (fread(&n, sizeof(int), 1, fp) != 1 || 
        fread(&k, sizeof(int), 1, fp) != 1 || 
        fread(&m, sizeof(int), 1, fp) != 1) {
        perror("Błąd odczytu nagłówka");
        fclose(fp);
        return;
    }

    printf("Nagłówek:\n");
    printf("n = %d, k = %d, m = %d\n", n, k, m);

    // Odczyt listy wierzchołków w każdej części
    for (int i = 0; i < k + 1; i++) {
        int size;
        if (fread(&size, sizeof(int), 1, fp) != 1) {
            perror("Błąd odczytu rozmiaru części");
            fclose(fp);
            return;
        }

        printf("Część %d, rozmiar = %d\n", i, size);

        if (size > 0) {
            int *vertices = malloc(size * sizeof(int));
            if (fread(vertices, sizeof(int), size, fp) != size) {
                perror("Błąd odczytu wierzchołków");
                fclose(fp);
                free(vertices);
                return;
            }

            printf("Wierzchołki: ");
            for (int j = 0; j < size; j++) {
                printf("%d ", vertices[j]);
            }
            printf("\n");
            free(vertices);
        }
    }

    fclose(fp);
}



int main(int argc, char **argv) {
    char *input_file = NULL;
    char *output_base = NULL;
    int num_cuts = 1;
    float margin_percent = 10.0;
    int binary_output = 0;
    int terminal_output = 1;
    print_usage();
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i+1 < argc) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i+1 < argc) {
            output_base = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
            num_cuts = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-m") == 0 && i+1 < argc) {
            margin_percent = atof(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0) {
            binary_output = 1;
            terminal_output = 0;
        } else if (strcmp(argv[i], "-t") == 0) {
            terminal_output = 1;
        } else {
            print_usage();
            return 1;
        }
    }
    
   // Walidacja argumentów
   if (!input_file || !output_base) {
    fprintf(stderr, "Błąd: Brak wymaganych argumentów\n");
    print_usage();
    return 1;
    }

    if (num_cuts < 1) {
        fprintf(stderr, "Błąd: Liczba przecięć musi być >= 1\n");
        return 1;
    }

    if (margin_percent < 0 || margin_percent > 100) {
        fprintf(stderr, "Błąd: Margines musi być w zakresie 0-100%%\n");
        return 1;
    }

    Graph graph;
    init_graph(&graph);
    
    // Wczytaj graf
    if (load_graph_from_csrrg(&graph, input_file) != 0) {
        fprintf(stderr, "Blad wczytywania grafu\n");
        return 1;
    }

    int successful_cuts = 0;
    find_connected_components(&graph);
    bool partition_success = true;

    while(successful_cuts < num_cuts && partition_success) {
        printf("\nPodzial #%d:\n", successful_cuts);

        // Tablice na grupy
        int group1[MAX_VERTICES], group2[MAX_VERTICES];
        int group1_size = 0, group2_size = 0;

        // Wykonaj podział grafu
        partition_success = partition_graph(&graph, group1, &group1_size, group2, &group2_size, margin_percent);
        if(partition_success){
            successful_cuts++;
            // Wypisz wynik podziału
            printf("Grupa 1 (%d wierzcholkow): ", group1_size);
            for (int j = 0; j < group1_size; j++) {
                printf("%d ", group1[j]);
            }
            
            printf("\nGrupa 2 (%d wierzcholkow): ", group2_size);
            for (int j = 0; j < group2_size; j++) {
                printf("%d ", group2[j]);
            }
        }

        //printf("\n\nSpojnosc grup:\n");
        // Zakładając, że masz funkcję `is_group_connected`, sprawdzamy spójność grup
        // printf("Grupa 1 jest %s\n", is_group_connected(&graph, 1) ? "spójna" : "niespójna");
        // printf("Grupa 2 jest %s\n", is_group_connected(&graph, 2) ? "spójna" : "niespójna");

        // Zwiększamy licznik wykonanych podziałów
    }
    /*Partition *partitions = malloc((num_cuts + 1) * sizeof(Partition));
    if (!partitions) {
        fprintf(stderr, "Błąd alokacji pamięci dla partitions\n");
        return 1;
    }
    for (int i = 0; i < num_cuts + 1; i++) {
        partitions[i].size = 0;
        partitions[i].vertices = NULL;
    }
    /// Wykonaj podziały
    int successful_cuts = 0;
    for (int i = 0; i < num_cuts; i++) {
        int *group1 = malloc(graph.num_vertices * sizeof(int));
        int *group2 = malloc(graph.num_vertices * sizeof(int));
        if (!group1 || !group2) {
            fprintf(stderr, "Błąd alokacji pamięci\n");
            free(group1);
            free(group2);
            break;
        }

        int group1_size = 0, group2_size = 0;
        if (partition_graph(&graph, group1, &group1_size, group2, &group2_size, margin_percent)) {
            successful_cuts++;
            
            // Aktualizacja przypisań grup
            for (int j = 0; j < group1_size; j++) {
                graph.group_assignment[group1[j]] = 1;
            }
            for (int j = 0; j < group2_size; j++) {
                graph.group_assignment[group2[j]] = 2;
            }
        }

        free(group1);
        free(group2);
    }

    
    // Tworzenie tablicy Partition na podstawie aktualnego przypisania grup
    for (int i = 0; i < successful_cuts + 1; i++) {
        partitions[i].size = 0;
    }

    // Najpierw policz ile wierzchołków przypada na każdą część
    for (int i = 0; i < graph.num_vertices; i++) {
        int group = graph.group_assignment[i];
        if (group >= 1 && group <= successful_cuts + 1) {
            partitions[group - 1].size++;
        }
    }

    // Teraz alokuj pamięć na wierzchołki dla każdej części
    for (int i = 0; i < successful_cuts + 1; i++) {
        partitions[i].vertices = malloc(partitions[i].size * sizeof(int));
        partitions[i].size = 0; // zresetuj do 0 żeby wstawić w kolejnym kroku
    }

    // Ponowne przypisanie wierzchołków do partitions
    for (int i = 0; i < graph.num_vertices; i++) {
        int group = graph.group_assignment[i];
        if (group >= 1 && group <= successful_cuts + 1) {
            partitions[group - 1].vertices[partitions[group - 1].size++] = i;
        }
    }

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
/*
    if (terminal_output) {
        print_partition_terminal(&graph, successful_cuts);
    }
    */
   
    if (binary_output) {
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "%s.bin", output_base);
        //save_binary(&graph, partitions, successful_cuts, output_file);
        read_binary("test_output","wynik.bin");
        for (int i = 0; i < successful_cuts + 1; i++) {
            //free(partitions[i].vertices);
        }
    } else {
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "%s.csrrg", output_base);
        save_to_text(&graph, output_file);
    }
    free_graph(&graph);
    return 0;
    
}