W linuxie należy zainstalować odpowiednie pakiety opisane na stronie 3 instrukcji

OGÓLNY OPIS:
Mamy dwa programy. Jeden to działający z przestrzeni użytkownika (nl), a drugi to moduł jądra (netlink). Moduł jądra służy do nasłuchiwania i odbierania komunikatów, a program dla użytkownika nadaje komunikaty. Po kompilacji za pomocą zawartego w instrukcji Makefile ładujemy moduł jądra (np. insmod netlink.ko). Następnie uruchamiamy program przeznaczony dla użytkownika (np. ./nl). Uruchomienie programu jest równoznaczne z nadaniem komunikatu. Załadowany moduł go odbiera i zapisuje informacje o nim w pliku /proc/netlink. Możemy je odczytać poleceniem cat /proc/netlink lub cat /proc/genetlink. Dla genetlink działamy analogicznie do tego co tutaj

Zadanie 2 jest zrobione nie do końca jak w poleceniu, a zadanie 6 nie działa poprawnie.