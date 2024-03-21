#include <stdio.h>
#include <string.h>

#define BUF_SIZE 256

int main() {
    char input[BUF_SIZE];
    printf("Program uruchomiony. Wpisz 'q' aby zakończyć.\n");
    do {
        scanf("%s", input);
    } while (strcmp(input, "q") != 0);
    printf("Program zakończony.\n");
    return 0;
}
