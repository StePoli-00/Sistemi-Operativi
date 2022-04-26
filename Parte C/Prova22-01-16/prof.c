/* Soluzione della Prova d'esame del 22 Gennaio 2016 - Parte C */
/* NOTA BENE: questa soluzione funziona se e solo se i figli trovano almeno una occorrenza di un carattere alfabetico maiuscolo */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

typedef int pipe_t[2];
typedef struct {
		char chr;
		long int occ;
		char proc;
		int pid;
        } T;

int main(int argc, char **argv) 
{
	/* -------- Variabili locali ---------- */
	int pid;			/* process identifier per le fork() */
	int M, N; 			/* 2N numero di file passati sulla riga di comando (uguale al numero di file) e N sarà la sua metà */
	int status;			/* variabile di stato per la wait */
	pipe_t *piped;			/* array dinamico di pipe descriptors per comunicazioni figli-padre  */
	pipe_t p1, p2;			/* due pipe per ogni coppia figlio-nipote */ 
	int fd;				/* file descriptor per figlio e nipote */
	int i, j;			/* indici per i cicli */
	char ch, AM;			/* carattere per leggere dal file e carattere da cercare */
	int primo=1; 			/* variabile che serve al figlio per sapere se deve cercare o meno il carattere AM per la prima volta */
	long int Nocc=0, Pocc=0; 	/* variabili per contare il numero di occorrenze di AM */
	T t;				/* struttura inviata dal figlio al padre */
	int ritorno; 			/* variabile che viene ritornata da ogni figlio/nipote al padre  */
	/* ------------------------------------ */
	
	/* Controllo sul numero di parametri */
	if ((argc < 5) || (argc - 1)%2 ) /* Meno di quattro parametri o non pari */  
	{
		printf("Errore nel numero dei parametri\n");
		exit(1);
	}

	/* Calcoliamo il numero di file passati corrispondente a 2N del testo */
	M = argc - 1;
	/* Calcoliamo il numero di processi figli */
	N=M/2;

	/* Allocazione dell'array di N pipe descriptor */
	piped = (pipe_t *) malloc (N*sizeof(pipe_t));
	if (piped == NULL)
	{
		printf("Errore nella allocazione della memoria\n");
		exit(2);
	}
	
	/* Creazione delle N pipe figli-padre */
	for (i=0; i < N; i++)
	{
		if(pipe(piped[i]) < 0)
		{
			printf("Errore nella creazione della pipe\n");
			exit(3);
		}
	}
		
	/* Ciclo di generazione dei figli */
	for (i=0; i < N; i++)
	{
		if ( (pid = fork()) < 0)
		{
			printf("Errore nella fork\n");
			exit(4);
		}
		
		if (pid == 0) 
		{
			/* codice del figlio */
			printf("Sono il processo figlio di indice %d e pid %d e leggero' il file %s\n", i, getpid(), argv[i+1]);

			/* Chiusura delle pipe non usate nella comunicazione con il padre  */
			for (j=0; j < N; j++)
			{
				close(piped[j][0]);
				if (i != j) close(piped[j][1]);
			}

			/* prima creiamo le due pipe di comunicazione fra nipote e figlio */
		  	if (pipe(p1) < 0)
                	{	
                        	printf("Errore nella creazione della prima pipe figlio-nipote\n");
				exit(-1); /* in caso di errori nei figli o nei nipoti decidiamo di tornare -1 che corrispondera' per il padre al valore 255 che sara' considerato alla stregua di un insuccesso nella lettura dell'ultimo carattere del file associato */
                	}

		  	if (pipe(p2) < 0)
                	{	
                        	printf("Errore nella creazione della seconda pipe nipote-figlio\n");
                        	exit(-1);
                	}
			/* printf("Sono il processo figlio di indice %d e pid %d sto per creare il nipote\n", i, getpid()); */
			if ( (pid = fork()) < 0)
			{
				printf("Errore nella fork di creazione del nipote\n");
                        	exit(-1);
			}	
			if (pid == 0) 
			{
				/* codice del nipote */
				printf("Sono il processo nipote del figlio di indice %d e pid %d e sto per leggere dal file %s\n", i, getpid(), argv[i+1+N]);
				/* chiusura della pipe rimasta aperta di comunicazione fra figlio-padre che il nipote non usa */
				close(piped[i][1]);
				/* apertura del file associato al nipote */
				if ((fd=open(argv[i+1+N], O_RDONLY)) < 0)
				{
                                	printf("Errore nella open del file %s\n", argv[i+1+N]);
                        		exit(-1);
                        	}
				/* ogni nipote deve chiudere i lati delle pipe di comunicazione con il figlio che non usa */
				close(p1[1]);
				close(p2[0]);
				/* per prima cosa il nipote deve ricevere il carattere da cercare AM */	
		        	read(p1[0], &AM, 1);
				/* printf("Sono il processo nipote del figlio di indice %d e pid %d e ho ricevuto %c da cercare nel file %s\n", i, getpid(), AM, argv[i+1+N]); */
		        	while ((ritorno=read(fd, &ch, 1)) != 0)
				{
					if (ch == AM) 
					/* trovato una occorrenza */
						Nocc++;
				}	
				/* per ultima cosa il nipote deve mandare il conteggio al figlio */	
		        	write(p2[1], &Nocc, sizeof(Nocc));
				exit(ritorno); /* torno il valore della read successo(0) oppure insuccesso (-1) */
			}
			else 
			{ 
				/* codice figlio */
				/* ogni figlio deve chiudere i lati che non usa delle pipe di comunicazione con il nipote */
				close(p1[0]);
				close(p2[1]);
				/* apertura del file associato al figlio */
				if ((fd=open(argv[i+1], O_RDONLY)) < 0)
				{
                                	printf("Errore nella open del file %s\n", argv[i+1]);
                        		exit(-1);
                        	}
				/* adesso il figlio legge dal file */
/*
				printf("Sono il processo figlio di indice %d e pid %d e sto per leggere il file %s\n", i, getpid(), argv[i+1]);
*/
 		        	while ((ritorno=read(fd, &ch, 1)) != 0) 
				{
/*
					printf("Sono il processo figlio di indice %d e pid %d e ho letto %c nel file %s\n", i, getpid(), ch, argv[i+1]);
*/
					if ((ch >= 'A') && (ch <= 'Z')) /* se e' un carattere alfabetico maiuscolo */
					{
						if (primo) /* se e' il primo */
						{
							AM = ch;
							Pocc++;
							/* bisogna mandarlo al nipote: ricordarsi di incrementare il numero di occorrenze! */
				/* printf("Sono il processo figlio di indice %d e pid %d e mando %c del file al nipote\n", i, getpid(), AM); */
							write(p1[1], &AM, 1);
							/* bisogna resettare il valore di primo */
							primo = 0;
				                }
						else 
						if (ch == AM)
						/* trovato una occorrenza */
							Pocc++;
					}	
				}	
				/* per ultima cosa il figlio deve recuperare il conteggio dal nipote */	
		        	read(p2[0], &Nocc, sizeof(Nocc));
				/* ora deve confezionare la struttura da inviare al padre */
				if (Nocc > Pocc)
                       	 	{	
					t.occ = Nocc;
					t.proc='N';
					t.pid=pid; /* pid del nipote */
                        	}
				else
                       	 	{	
					t.occ = Pocc;
					t.proc='F';
					t.pid=getpid(); /* proprio pid */
                        	}
				t.chr=AM; /* riempiamo il campo chr con il valore del carattere cercato, che e' uguale per figlio e nipote */
				/* il figlio comunica al padre */
				write(piped[i][1], &t, sizeof(t));
				
				/* il figlio deve aspettare il nipote */
				pid = wait(&status);
				if (pid < 0)
				{	
					printf("Errore in wait\n");
					exit(-1);
				}
				/* la stampa da parte del figlio non e' richiesta esplicitamente, ma implicitamente si capisce che deve essere fatta */
				if ((status & 0xFF) != 0)
    					printf("Nipote con pid %d terminato in modo anomalo\n", pid);
    				else
					printf("Il nipote con pid=%d ha ritornato %d (valore 255 significa problemi nel nipote)\n", pid, (int)((status >> 8) & 0xFF));
				exit(ritorno);
  			}	
		}
	}
	printf("Sono il processo padre ho creato %d processi \n", N);
	
	/* Codice del padre */
	/* Il padre chiude i lati delle pipe che non usa */
	for (i=0; i < N; i++)
		close(piped[i][1]);

	/* Il padre recupera le informazioni dai figli  */
	for (i=0; i < N; i++)
	{
		/* il padre recupera tutte le strutture inviate dai figli */
		read(piped[i][0], &t, sizeof(t));
		if (t.proc == 'N')
			printf("Il nipote con pid %d ha calcolato il valore di occorrenze %ld del carattere %c per il file %s\n", t.pid, t.occ, t.chr, argv[i+1+N]);
		else
			printf("Il figlio con pid %d ha calcolato il valore di occorrenze %ld del carattere %c per il file %s\n", t.pid, t.occ, t.chr, argv[i+1]);

	}	

	/* Il padre aspetta i figli */
	for (i=0; i < N; i++)
	{
		pid = wait(&status);
		if (pid < 0)
		{
			printf("Errore in wait\n");
			exit(5);
		}

		if ((status & 0xFF) != 0)
    			printf("Figlio con pid %d terminato in modo anomalo\n", pid);
    		else
		{ 
			ritorno=(int)((status >> 8) &	0xFF); 
		  	printf("Il figlio con pid=%d ha ritornato %d (valore 255 significa problemi nel figlio)\n", pid, ritorno);
		}
	}
exit(0);
}