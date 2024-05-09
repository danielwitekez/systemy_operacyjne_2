Do zadania 2:
zadanie2.c to normalny kod modułu jądra i kompilujemy go poprzez Makefile i polecenie make. 
Natomiast zadanie2_user.c to program dla przestrzeni użytkownika, który należy kompilować przez gcc, można oczywiście dołączyć polecenie do Makefile dla modułu.

Do zadania 3:
Zapisujemy poleceniem echo liczba > /dev/device1
Odczytujemy poleceniem cat /dev/device1
Analogicznie dla drugiego urządzenia

Do zadania 5:
Zapisujemy poleceniem echo "wiadomosc" > /dev/clipboard
Odczytujemy poleceniem cat /dev/clipboard

Do zadania 6:
Zapisujemy poleceniem echo "wiadomosc" > /sys/module/zadanie6/subdir/text
Odczytujemy poleceniem cat /dev/device1