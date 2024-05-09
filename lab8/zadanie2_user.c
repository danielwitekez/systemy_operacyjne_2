#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int main() {	
	int file, pozycja;
	unsigned long long odczytane;

	printf("Podaj numer wyrazu do odczytania: ");
	scanf("%d", &pozycja);
	pozycja--;

	file = open("/dev/fibdev", O_RDONLY);

	if (file < 0) {
		printf("Nie można otworzyć pliku urządzenia /dev/fibdev\n");
		return -1;
	}

	if (pozycja >= 0) {
		odczytane = lseek(file, pozycja, SEEK_SET);

		if (odczytane == -1) {
			printf("Wystąpił błąd odczytu\n");
		}
		else {
			printf("Odczytany wyraz: %llu\n", odczytane);
		}
	} else {
		printf("Podano nieprawidłowy numer wyrazu\n");
	}
	
	close(file);
	
    return 0;
}