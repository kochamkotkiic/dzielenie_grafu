#include "graf.h"
#include "partitions.h"
#include "output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage() {
    printf("Użycie: -i <input.csrrg> -o <output_base> -p <liczba_przecięć> -m <margines%%> [-b] [-t]\n");
    printf("   -i : Plik wejściowy w formacie CSR (.csrrg)\n");
    printf("   -o : Bazowa nazwa plików wyjściowych\n");
    printf("   -p : Liczba przecięć (tworzy N+1 części, domyślnie 1)\n");
    printf("   -m : Maksymalny %% różnicy wielkości części (domyślnie 10%%)\n");
    printf("   -b : Zapis binarny (domyślnie tekstowy)\n");
    printf("   -t : Wypisz wynik w terminalu\n");
}

int main(int argc, char **argv) {
    char *input_file = NULL;
    char *output_base = NULL;
    int num_cuts = 1;
    float margin_percent = 10.0;
    int binary_output = 0;
    int terminal_output = 0;
    //print_usage();
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
        

        // Wykonaj podział grafu
        partition_success = partition_graph(&graph, margin_percent);
        if(partition_success){
            successful_cuts++;
        
        }else {
            fprintf(stderr, "Błąd: Nie udało się wykonać podziału %d\n", successful_cuts+1);
            break;  // Przerwij pętlę jeśli podział się nie udał
        } 
 
    }

    if (terminal_output) {
        print_partition_terminal(&graph, successful_cuts);
    }
    
    if (binary_output) {
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "%s.bin", output_base);
        save_graph_to_binary(&graph, output_file);
        
    } else {
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "%s.csrrg", output_base);
        save_graph_to_csrrg(&graph, output_file);
    }
    
    free_graph(&graph);
    return 0;
}