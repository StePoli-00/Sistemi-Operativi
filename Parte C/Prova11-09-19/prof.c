/* soluzione parte C esame dell'11 Settembre 2019: questa soluzione considera che la comunicazione in ogni coppia sia dal processo pari al processo dispari e che il processo dispari deve creare il file con terminazione ".MAGGIORE" */
#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/wait.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 

#define PERM 0644

typedef int pipe_t[2]; 		/* tipo di dato per contenere i file descriptors di una pipe */

int main(int argc, char **argv) 
{
	int N; 			/* numero di file: i processi figli saranno il doppio! */
	int pid;		/* variabile per fork */
	pipe_t *pipe_f;		/* array di pipe per la comunicazione dai figli pari e figli dispari */
	int fd;			/* variabile per open */
	char *Fcreato;		/* variabile per nome file da creare da parte dei processi dispari */
	int fdw;		/* variabile per creat */
	char ch;		/* variabile per leggere dai figli */
	char ch1;		/* variabile per ricevere dal processo-coppia */
	int letti;		/* variabile per tenere traccia del numero di caratteri letti */
	int status, pidFiglio, ritorno;	/* per wait */
	int i, j;		/* indici per cicli */
	int nr, nw;		/* per controlli read e write su/da pipe */

/* Controllo sul numero di parametri */
if ( (argc < 4) && (argc - 1) % 2 !=0)
{
	printf("Errore numero parametri %d\n", argc);
	exit(1);
}

/* calcoliamo il numero dei file */
N = argc - 1; 

printf("Numero processi da creare %d\n", N);

/* allocazione memoria dinamica per pipe_fi. NOTA BENE: servono un numero di pipe che e' la meta' del numero di figli! */
pipe_f=malloc(N/2*sizeof(pipe_t));
if (pipe_f == NULL) 
{
	printf("Errore nelle malloc\n");
	exit(2);
}

/* creazione delle pipe: ATTENZIONE VANNO CREATE N/2 pipe */
for (i=0; i < N/2; i++) 
{
	if (pipe(pipe_f[i])!=0) 
	{
		printf("Errore creazione delle pipe\n"); exit(3);
	}
}

/* creazione dei processi figli: ne devono essere creati N */
for (i=0; i < N; i++) 
{
	pid=fork();
 	if (pid < 0)  /* errore */
   	{
		printf("Errore nella fork con indice %d\n", i);
      		exit(4);
   	}
	if (pid == 0) 
	{
 		/* codice del figlio */
		/* stampa di debugging */
		printf("Figlio di indice %d e pid %d associato al file %s\n",i,getpid(),argv[i+1]);
		/* ogni figlio deve aprire il suo file associato */
		fd=open(argv[i+1], O_RDONLY);
		if (fd < 0) 
		{
			printf("Impossibile aprire il file %s\n", argv[i+1]);
			exit(0); /* in caso di errore torniamo 0 che non e' un valore accettabile (per quanto risulta dalla specifica della parte shell) */
		}

		/* inizializziamo letti */
		letti=0;
      		if (i % 2 == 0) /* siamo nel codice dei figli pari */
		{
      			/* chiudiamo le pipe che non servono */
      			/* ogni figlio pari scrive solo su pipe_f[i] */
      			for (j=0;j<N/2;j++)
      			{
        			close(pipe_f[j][0]);
        			if (j!=i/2) 	/* ATTENZIONE ALL'INDICE CHE DEVE ESSERE USATO */
        			{
        				close(pipe_f[j][1]);
        			}
      			}

	 		while (read(fd, &ch, 1)) 
			{
				/* incrementiamo letti */
				letti++;
				/* ad ogni carattere letto da un processo di indice pari, bisogna mandare il carattere al processo-coppia: l'indice della pipe da usare si trova dividendo i per 2  */
				nw=write(pipe_f[i/2][1], &ch, sizeof(ch));
				if (nw != sizeof(ch))
    				{		
                        		printf("Impossibile scrivere sulla pipe per il processo di indice %d\n", i);
                        		exit(0); 
				}
			}
		}
		else /* siamo nel codice dei figli dispari */
		{
                        /* chiudiamo le pipe che non servono */
                        /* ogni figlio dispari legge solo da pipe_f[i] */
                        for (j=0;j<N/2;j++)
                        {
                                close(pipe_f[j][1]);
                                if (j!= i/2)	 /* ATTENZIONE ALL'INDICE CHE DEVE ESSERE USATO */
                                {
                                        close(pipe_f[j][0]);
                                }
                        }

			/* i figli dispari devono creare il file specificato */
			Fcreato=(char *)malloc(strlen(argv[i+1]) + 10); /* bisogna allocare una stringa lunga come il nome del file + il carattere '.' + i caratteri della parola MAGGIORE (8) + il terminatore di stringa */
			if (Fcreato == NULL) 
			{
				printf("Errore nelle malloc\n");
				exit(0);
			}
			/* copiamo il nome del file associato al figlio dispari */
			strcpy(Fcreato, argv[i+1]);
			/* concateniamo la stringa specificata dal testo */
			strcat(Fcreato,".MAGGIORE");
			fdw=creat(Fcreato, PERM);
 			if (fdw < 0)
                	{	
                        	printf("Impossibile creare il file %s\n", Fcreato);
                        	exit(0); 
                	}

                        while (read(fd, &ch, 1))
                        {
				/* incrementiamo letti */
				letti++;
				/* ad ogni carattere letto da un processo di indice dispari, bisogna ricevere il carattere letto dal processo-coppia: l'indice della pipe da usare si trova dividendo i per 2  */
                                nr=read(pipe_f[i/2][0], &ch1, sizeof(ch1));
                                if (nr != sizeof(ch1))
    				{		
                        		printf("Impossibile leggere dalla pipe per il processo di indice %d\n", i);
                        		exit(0); 
         			}		
				/* printf("Caratteri letti da processo di indice %d: ch = %c e ch1 = %c\n", i, ch, ch1); */
				if (ch <= ch1)  	/* se il carattere letto e' minore o uguale al carattere ricevuto, deve essere scritto il carattere ricevuto, altrimenti il carattere letto */
				{
					ch = ch1;
					/* printf("Carattere che verra' scritto da processo di indice %d: ch = %c\n", i, ch); */
				}
				write(fdw, &ch, 1);

        		}
		}	
		exit(letti); /* torniamo il numero di caratteri letti (supposto dal testo della parte shell <= di 255) */
	}
}

/*codice del padre*/
/* chiudiamo tutte le pipe, dato che le usano solo i figli */
for (i=0;i<N;i++)
{
   close(pipe_f[i][0]);
   close(pipe_f[i][1]);
}

/* Attesa della terminazione dei figli */
for(i=0;i<N;i++)
{
   pidFiglio = wait(&status);
   if (pidFiglio < 0)
   {
      printf("Errore wait\n");
      exit(5);
   }
   if ((status & 0xFF) != 0)
                printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
   else
   {
                ritorno=(int)((status >> 8) & 0xFF);
                printf("Il figlio con pid=%d ha ritornato %d (se 0 problemi!)\n", pidFiglio, ritorno);
   }
}
exit(0);
}/* fine del main */