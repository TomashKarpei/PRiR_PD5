#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define REZERWA 500
#define HANGAR 1
#define START 2
#define LOT 3
#define KONIEC_LOTU 4
#define WYPADEK 5
#define TANKUJ 5000

int paliwo = 5000;
int STOJ=1, NIE_STOJ=0;
int liczba_procesow;
int nr_procesu;
int ilosc_samolotow;
int ilosc_lotow=5;
int ilosc_zajetych_lotow=0;
int tag=1;
int wyslij[2];
int odbierz[2];
MPI_Status mpi_status;

void Wyslij(int nr_samolotu, int stan)
{
    wyslij[0]=nr_samolotu;
    wyslij[1]=stan;
    MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    sleep(1);
}

void Hangar(int liczba_procesow)
{
    int nr_samolotu,status;
    ilosc_samolotow = liczba_procesow - 1;

    printf("Początek lotu \n");
    printf("Lotow w tym momencie %d\n", ilosc_lotow);
    sleep(2);

    while(ilosc_lotow<=ilosc_samolotow)
        {
            MPI_Recv(&odbierz,2,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &mpi_status);
            nr_samolotu=odbierz[0];
            status=odbierz[1];
            if(status==1)
            {
                printf("Samolot %d stoi w hangaru\n", nr_samolotu);
            }
            if(status==2)
            {
                printf("Samolot %d leci na trase nr %d\n", nr_samolotu, ilosc_zajetych_lotow);
                ilosc_zajetych_lotow--;
            }
            if(status==3)
            {
                printf("Samolot %d leci\n", nr_samolotu);
            }
            if(status==4)
            {
                if(ilosc_zajetych_lotow<ilosc_lotow)
                {
                    ilosc_zajetych_lotow++;
                    MPI_Send(&STOJ, 1, MPI_INT, nr_samolotu, tag, MPI_COMM_WORLD);
                }
                else
                {
                    MPI_Send(&NIE_STOJ, 1, MPI_INT, nr_samolotu, tag, MPI_COMM_WORLD);
                }
            }
            if(status==5)
            {
                ilosc_samolotow--;
                printf("Ilosc samolotow %d\n", ilosc_samolotow);
            }
        }
    printf("Koniec programuu\n");
}

void Samolot()
{
    int stan,suma,i;
    stan=LOT;
    while(1)
    {
        if(stan==1)
        {
            if(rand()%2==1)
            {
                stan=START;
                paliwo=TANKUJ;
                printf("Samolot czeka na pozwolenie na lot %d",nr_procesu);
                printf("\n");
                Wyslij(nr_procesu,stan);
            }
            else
            {
                Wyslij(nr_procesu,stan);
            }
        }
        else if(stan==2)
        {
            printf("Samolot wylatuje %d\n",nr_procesu);
            stan=LOT;
            Wyslij(nr_procesu,stan);
        }
        else if(stan==3)
        {
            paliwo-=rand()%500;
            if(paliwo<=REZERWA)
            {
                stan=KONIEC_LOTU;
                printf("Prosze o pozwolenie na zjazd do zajezdzni\n");
                Wyslij(nr_procesu,stan);
            }

            else
            {
                for(i=0; rand()%10000;i++);
            }
        }
        else if(stan==4)
        {
            int temp;
            MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
            if(temp==STOJ)
            {
                stan=HANGAR;
                printf("Samolot %d zakonczyl lot\n", nr_procesu);
            }
            else
            {
                paliwo-=rand()%500;
                if(paliwo>0)
                {
                    Wyslij(nr_procesu,stan);
                }
                else
                {
                    stan=WYPADEK;
                    printf("Wypadek\n");
                    Wyslij(nr_procesu,stan);
                return;
                }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&nr_procesu);
    MPI_Comm_size(MPI_COMM_WORLD,&liczba_procesow);
    srand(time(NULL));
    if(nr_procesu == 0)
    Hangar(liczba_procesow);
    else
    Samolot();
    MPI_Finalize();
    return 0;
}
