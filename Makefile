# Makefile dla projektu podziału grafu

# Kompilator
CC = gcc
# Flagi kompilacji
CFLAGS = -Wall -Wextra -std=c11 -g
# Biblioteki
LIBS = -lm

# Nazwa programu
TARGET = graph_partition
# Pliki źródłowe
SRCS = main.c graf.c partitions.c output.c
# Pliki nagłówkowe
HDRS = graf.h partitions.h output.h
# Pliki obiektowe
OBJS = $(SRCS:.c=.o)

# Domyślna reguła
all: $(TARGET)

# Kompilacja programu
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Generowanie plików obiektowych
%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Czyszczenie plików tymczasowych
clean:
	rm -f $(OBJS) $(TARGET) *.bin *.csrrg test_output/*

# Testowanie programu
test: $(TARGET)
	@echo "Uruchamianie testów..."
	@mkdir -p test_output
	@./$(TARGET) -i test_data/grafTest.csrrg -o test_output/wynik -m 10 
	@echo "Testy zakończone. Wyniki w test_output/"

# Test jednostkowy funkcji podziału
test_partition: $(OBJS)
	$(CC) $(CFLAGS) -DTEST_PARTITION partitions.c -o test_partition $(LIBS)
	./test_partition

# Pomoc
help:
	@echo "Dostępne polecenia:"
	@echo "  make           - kompiluje program"
	@echo "  make clean     - usuwa pliki tymczasowe"
	@echo "  make test      - uruchamia testy integracyjne"
	@echo "  make test_partition - uruchamia test jednostkowy funkcji podziału"
	@echo "  make help      - wyświetla tę pomoc"

.PHONY: all clean test test_partition help