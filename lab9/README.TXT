Dotyczy każdego zadania z tego tematu:
1. Najpierw kompilujemy i ładujemy moduł jądra.
2. Następnie tworzymy system plików: mkfs.ext2 /dev/sbd
3. Dla łatwego sprawdzania poprawności działania polecam w katalogu /mnt zrobić podkatalog, np. test.
4. Teraz należy zamontować ten katalog: mount /dev/sbd /mnt/test
5. Po wejściu w /mnt/test możemy zapisywać i odczytywać z niego pliki i foldery
6. Po zakończeniu pracy trzeba odmontować katalog. Robimy to poleceniem umount /mnt/test
7. Teraz możemy odładować moduł.