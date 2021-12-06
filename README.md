Programowanie równolegle i rozproszone - projekt 1 labolatorium 7 Symulacja Sklepu Internetowego w środowisku MPI
Na potrzeby testowania czas realizacji zamówienia jest znacznie szybszy niż w rzeczywistości.

Import bibliotek ( w tym biblioteki MPI umożliwiającej pisanie programów wykonywanych równolegle):

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <time.h>
    #include "mpi.h"
  
Definiowanie stanów (jako stałych) w jakich może znajdować się zamowienie:

    #define WERYFIKACJA 1
    #define KOMPLETOWANIE 2
    #define PAKOWANIE 3
    #define PRZEKAZANIEDOSTAWCY 4
    #define WYSYLKAWDRODZE 5
    #define WYSYLKANAMIEJSCU 6
  
Definicja potrzebnych zmiennych:

    int nr_zamowienia;	//numer zamówienia
    int stan;	//stan zamówienia
    int ilosc_zamowien;

    int PRZYJMUJ=1, NIE_PRZYJMUJ=0; // zmienne do określania tego czy przyjąć zamówienie
    int BRAK_AWIZO=2, AWIZO=3;      // zmienne do określania tego czy dostawa miała awizo

    //int ilosc_zamowien = 1000;
    int ilosc_pracownikow = 7;
    int ilosc_zajetych_pracownikow = 0;

    int liczba_procesow;
    int nr_procesu;
    int tag=2115;
    int wyslij[2];
    int odbierz[2];

    MPI_Status mpi_status;

Definicja funkcji Wyslij do wysyłanai stanu zamówienia (jako argumenty wejściowe przyjmuje numer zamówienia - czyli numer procesu oraz stan tego zamówienia/procesu):

    void Wyslij(int nr_zamowienia, int stan)
    {
      wyslij[0]=nr_zamowienia;
      wyslij[1]=stan;
      MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
      sleep(1);
    }

Funkcja Sklep przyjmuje jako argument wejściowy liczbę procesów. Każdy pracownik może obsługiwać dokładnie 1 zamówienie na raz. Dlatego zamówienie jest odbierane wraz z jego statusem za pomocą funkji MPI_Recv. Nastęnie jest odpowiednio przetwarzane a odpowiednie komunikaty są wyświetlane w konsoli. Najpierw zamowienie jest weryfikowane , gdzie na podstawie ilości zajętych pracowników określa się czy je przyjąć ustawiając jako zmienną PRZYJETE lub NIE_PRZYJETE do wysyłki za pomocą funkcji MPI_Send. Następnie przechodzi proces Kompletowania oraz Pakowania zamówienia po czym przekazywane jest dostawcy(wtedy pracownik zostaje oznaczony jako gotowy do przyjęcia kolejnego zamówienia). Dostawca wysyłając przesyłkę ustawia jej stan na PRZESYŁKA W DRODZE , a następnie podejmuje probę dostawy, która może się nie udać(wówczas ustawiana jest zmienna AWIZO i wysyłana przez MPI_Send):

    void Sklep(int liczba_procesow){
      int nr_zamowienia, status;
      ilosc_zamowien = liczba_procesow - 1;
      printf("Jest %d pracownikow w sklepie\n", ilosc_pracownikow);
      sleep(2);
      while(ilosc_pracownikow<=ilosc_zamowien){
        MPI_Recv(&odbierz,2,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &mpi_status);
        nr_zamowienia=odbierz[0];
        status=odbierz[1];
        if(status==1){
          printf("Weryfikacja zamowienia o numerze %d\n", nr_zamowienia);
          //sleep(5);
          if(ilosc_zajetych_pracownikow < ilosc_pracownikow){
            ilosc_zajetych_pracownikow++;
            MPI_Send(&PRZYJETE, 1, MPI_INT, nr_zamowienia, tag, MPI_COMM_WORLD);
          }
          else{
            MPI_Send(&NIE_PRZYJETE, 1, MPI_INT, nr_zamowienia, tag, MPI_COMM_WORLD);
        }
        if(status==2){
          printf("Zamowienie o numerze %d jest kompletowane na stanowisku nr %d\n", nr_zamowienia, ilosc_zajetych_pracownikow);
          //sleep(10);
        }
        if(status==3){
          printf("Zamowienie o numerze %d jest pakowane\n", nr_zamowienia);
          //sleep(7);
        }
        if(status==4){
          printf("Zamowienie o numerze %d zostalo przekazane dostawcy\n", nr_zamowienia);
          ilosc_zajetych_pracownikow--;
        }
        if(status==5){
          printf("Zamowienie o numerze %d zostalo wyslane (jest w drodze)\n", nr_zamowienia);
          //sleep(10);
          if(rand()%2==1){
            MPI_Send(&BRAK_AWIZO, 2, MPI_INT, nr_zamowienia, tag, MPI_COMM_WORLD);
          }
          else{
            MPI_Send(&AWIZO, 3, MPI_INT, nr_zamowienia, tag, MPI_COMM_WORLD);
          }
        }
        if(status==6){
          ilosc_zamowien--;
          printf("Ilosc zamowien %d\n", ilosc_zamowien);
        }
      }
      printf("Program zakonczyl dzialanie\n");
    }

W funkcji Zamowienie() odczytywane są kolejne stany zamowienia i wysyłane zmiany tych stanów, a także wypisywanie ich w konsoli:

    void Zamowienie(){
      int cena;
      stan=WERYFIKACJA;
      while(1){
        if(stan==1){
          int temp;
          MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
          if(temp == PRZYJETE){
            stan = KOMPLETOWANIE;
            cena = rand()%10000;
            printf("Zamowienie o numerze %d zostalo przyjete, cena: %d\n", nr_procesu, cena);
            Wyslij(nr_procesu,stan);
          }
          else{
            printf("Odrzucono zamowienie o numerze %d \n",nr_procesu);
            Wyslij(nr_procesu,stan);
          }
        }
        else if(stan==2){
          printf("Zamowienie o numerze %d jest kompletowane\n",nr_procesu);
          stan=PAKOWANIE;
          Wyslij(nr_procesu,stan);
        }
        else if(stan==3){
          printf("Zamowienie o numerze %d jest pakowane\n",nr_procesu);
          stan=PRZEKAZANIEDOSTAWCY;
          Wyslij(nr_procesu,stan);
        }
        else if(stan==4){
          printf("Zamowienie o numerze %d jest w drodze do klienta\n",nr_procesu);
          stan = WYSYLKAWDRODZE;
          Wyslij(nr_procesu,stan);
        }
        else if(stan == 5){
          int temp;
          MPI_Recv(&temp, 5, MPI_INT, 2, tag, MPI_COMM_WORLD, &mpi_status);
          if(temp == BRAK_AWIZO){
            stan = WYSYLKANAMIEJSCU;
            printf("Zamowienie o numerze %d dotarlo do klienta(zostalo zakonczone))\n",nr_procesu);
            Wyslij(nr_procesu,stan);
          }
          else{
            printf("Zamowienie o numerze %d nie dotarlo do klienta(AWIZO))\n",nr_procesu);
            Wyslij(nr_procesu,stan);
            return;
          }
        }
      }
    }

W mainie najpierw inicjalizujemy wszystkie potrzebne zmienne ( w tym funkcję MPI_Init - ma ona za zadanie zainicjalizowanie środowiska MPI dla procesu, który ją wywołuje Inicjalizacja informuje środowisko uruchomieniowe MPI o nowym procesie). Jeżeli to pierwszy proces to inicjujemy nasz Sklep, a każdy następny jest już Zamówieniem:


	int main(int argc, char *argv[]) {

		MPI_Init(&argc, &argv);    //inicjalizacja środowiska MPI
		MPI_Comm_rank(MPI_COMM_WORLD,&nr_procesu);      // okresla numer aktualnego procesu
		MPI_Comm_size(MPI_COMM_WORLD,&liczba_procesow); // okresla liczbę uruchomionych procesow tworzących sieć
		srand(time(NULL));
		if(nr_procesu == 0)
			Sklep(liczba_procesow);
		else 
			Zamowienie();
		MPI_Finalize(); //funkcja kończy pracę w trybie MPI
		return 0;

	}
