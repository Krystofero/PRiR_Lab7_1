#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define WERYFIKACJA 1
#define KOMPLETOWANIE 2
#define PAKOWANIE 3
#define PRZEKAZANIEDOSTAWCY 4
#define WYSYLKAWDRODZE 5
#define WYSYLKANAMIEJSCU 6

int nr_zamowienia;	//numer zamówienia
int stan;	//stan zamówienia
int ilosc_zamowien;

int PRZYJMUJ=1, NIE_PRZYJMUJ=0;
int BRAK_AWIZO=2, AWIZO=3;

//int ilosc_zamowien = 1000;
int ilosc_pracownikow = 7;
int ilosc_zajetych_pracownikow = 0;

int liczba_procesow;
int nr_procesu;
int tag=2115;
int wyslij[2];
int odbierz[2];

MPI_Status mpi_status;


void Wyslij(int nr_zamowienia, int stan)
{
	wyslij[0]=nr_zamowienia;
	wyslij[1]=stan;
	MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
	sleep(1);
}


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


int main(int argc, char *argv[]) {
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&nr_procesu);
	MPI_Comm_size(MPI_COMM_WORLD,&liczba_procesow);
	srand(time(NULL));
	if(nr_procesu == 0)
		Sklep(liczba_procesow);
	else 
		Zamowienie();
	MPI_Finalize();
	return 0;
	
}
