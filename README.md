lhttp
=====

Praca na zaliczenie

Serwer http działający w trybie demona.


Opis działania
--------------

Po uruchomieniu serwer zacznie nasułuchiwać na podanym adresie.
Wpisanie w przeglądarkę tego adresu wyświetli listę odstępnych plików
z katalogu roboczego. Jeżeli w katalogu znajduje się plik
o nazwie `index.html`, to on zostaje przesłany. Podobnie dzieje się
z katalogami podrzędnymi.


Uruchomienie
------------

`lhttpd [-h] [-v] [-f] [-t temp_dir] [-l log_file] [-d working_directory] [ip[:port]]`

lhttpd nasłuchuje na podanym ip i porcie (`[ip[:port]]`, domyślnie jest to
`0.0.0.0:80`) zapytań i serwuje pliki z folderu roboczego podanego w
`working_directory` (`flaga -d`, domyślnie `.`).

Flaga `-l` wskazuje na plik logów (domyślnie `/tmp/lhttpd.log`), w którym
zawierają się informacje z ważniejszymi wydarzeniami opatrzone datą i godziną.

By zwiększyć szczegółowość informacji należy dopisać użyć flagi `-v`.
`-v` spowoduje, że logi będą zawierały wiadomości na temat zapytań.
`-vv` i `-vvv` doda komunikatów do debugowania, jak np. nagłówki w zapytaniu,
czy sposób obsługi zapytania.

`temp_dir` to miejsce, w którym lhttpd będzie tworzyć swoje pliki tymczasowe
(domyślnie `/tmp`).

`-f` sprawi, że lhttpd nie przejdzie w tryb demona, a logi wyświetli w konsoli
bez względu na flagę `-l`.


Przygotowanie
-------------

Z pozycji katalogu z plikami źródłowymi należy uruchomić komendę

    $ make


Szybki test
-----------

By szybko przetestwać działanie serwera należy wpisać

    $ ./lhttpd -fv -d ./test localhost:8080

a następnie w przeglądarce wpisać `localhost:8080`.
