#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

typedef int pipe_t[2];
typedef char lin[250];
int main(int argc, char **argv){
	/*---variabili----*/
	int N; /*numero di processi da crear*/
	int n; /*indice per i figli*/
	int i; /*indice per scorrere*/
	int fcreato; /*file creato*/
	int fd; /*file descriptor per i figli*/
	pipe_t *pipes; /*array di comunicazione a ring: ogni figlio legge dalla propria pipe e scrive su (n+1)%Q*/
	int pid; /*pid per fork*/
	lin linea; /*linea corrente*/
	lin *tutteLinee; /*array con tutte le linee*/
	int nw, nr; /*per valori di ritorno*/
	int status, ritorno, pidFiglio; /*per valori di ritorno*/
	/*----------------*/
	
	/*almeno due file*/
	if(argc < 3){
		printf("Errore parametri\n");
		exit(1);
	}
	
	N = argc - 1; 
	printf("numero di processi da creare %d\n",N);
	
	
	/*creo il file come da specifica*/
	if((fcreato = creat("Palazzo",0644)) < 0){
		printf("Errore creazione file fcreato\n");
		exit(2);
	}
	
	/*allocazione memoria*/
	pipes=(pipe_t *)malloc(N*sizeof(pipe_t));
	if(pipes == NULL){
		printf("Errore allocazione memoria\n");
		exit(3);
	}
	/*creazione pipe*/
	for(n = 0; n < N; n++){
		if(pipe(pipes[n]) != 0){
			printf("Errore pipe\n");
			exit(4);
		}
	}
	
	/*allocazione memoria per tutteLinee*/
	tutteLinee=(lin *)malloc(N*sizeof(lin));
	/*ciclo di generazione dei figli*/
	for(n = 0; n < N; n++){
		if((pid = fork()) < 0){
			printf("Errore fork\n");
			exit(5);
		}		
		if(pid == 0){
			/*codice figlio*/
			/*chiusura delle pipe non utilizzate*/
			for( i = 0; i < N; i++){
				if(n != i) 
					close(pipes[i][0]);
				if(i != (n+1)%N)
					close(pipes[i][1]);
			}
			
			/*apertura del file associato*/
			if((fd = open(argv[n+1],O_RDONLY)) < 0){
				printf("Errore apertura file\n");
				exit(-1); /*VALORE NON AMMISSIBILE DAL PADRE*/
			}
			
			i = 0;
			while(read(fd,&linea[i],1)){
				if(linea[i] == '\n'){
					/*se abbiamo trovato una linea*/
					i++;
					/*per sicurezza il controllo su lettura*/
					
					nr = read(pipes[n][0],tutteLinee,N*sizeof(lin));
					if(nr != N*sizeof(lin)){
						printf("Errore lettuar per figlio %d\n",n);
						exit(-1);
					}
						
					/*copio la linea corrente nel posto giusto*/
					memcpy(tutteLinee[n],linea,i);
					/*non richiesto: debugging*/
					for(i = 0; i < N; i++){
						printf("%s",tutteLinee[i]);
					}
					nw=write(pipes[(n+1)%N][1],&tutteLinee,N*sizeof(lin));
					if(nw != N*sizeof(lin)){
						printf("Errore scrittura su pipe[%d]",n);
						exit(-1);			
					}
					
					if(n == N-1){
					/*se siamo all'ultimo figlio*/
					nw=write(fcreato,tutteLinee,N*sizeof(lin));
					/*controllo per sicurezza*/
					if(nw != N*sizeof(lin)){
						printf("Errore scrittura su fcreato\n");
						exit(-1);
					}
					
				}
					ritorno = i; /*salvo il valore di ritorno se è l'ultima*/
					i = 0;
				}else i++;
				
				
			}
			exit(ritorno);
		}
	}
	
	/*codice padre*/
	for(n=1;n < N;n++)
	{
		close(pipes[n][0]);
		close(pipes[n][1]); 
	}
	
	/*scrittura la primo figlio cme innesco*/
	nw=
	write(pipes[0][1],tutteLinee,N*sizeof(lin));
	/*non richiesta*/
	/*printf("Il padre scrive %d caratteri e N*sizeof(lin) = %ld\n",nw,N*sizeof(lin));*/
	if(nw != N*sizeof(lin)){
		printf("Errore scritura padre al primo figlio\n");
		exit(6);
	}

	close(pipes[0][1]);
	/*il padre aspetta i figli*/
	for (n=0; n < N; n++)
{
        pidFiglio = wait(&status);
        if (pidFiglio < 0)
        {
                printf("Errore in wait\n");
                exit(7);
        }
        if ((status & 0xFF) != 0)
                printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else
        { 
		ritorno=(int)((status >> 8) & 0xFF);
        	printf("Il figlio con pid=%d ha ritornato %d (se 255 %d problemi)\n", pidFiglio, ritorno, N-1);
        } 
}
exit(0);
}
