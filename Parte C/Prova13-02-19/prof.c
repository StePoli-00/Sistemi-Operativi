/* Soluzione della parte C del compito del 13 Febbraio 2019 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

typedef int pipe_t[2];
typedef struct{
	long int tot; 		/* numero massimo di linee (campo c1 del testo) */
        int id; 		/* indice figlio (campo c2 del testo) */
		} s_tot;

int main (int argc, char **argv)
{
int N; 			/* numero di file */
int *pid;		/* array di pid per fork */
pipe_t *pipes;		/* array di pipe usate a pipeline da primo figlio, a secondo figlio .... ultimo figlio e poi a padre: ogni processo (a parte il primo) legge dalla pipe i-1 e scrive sulla pipe i */
int i,j; 		/* indici */
int fd; 		/* file descriptor */
int pidFiglio, status, ritorno;	/* per valore di ritorno figli */
char ch;        	/* carattere letto dai figli */
long int cur_tot; 	/* conteggio delle linee calcolate da ogni figlio */
s_tot letta; 		/* struttura usata dai figli e dal padre */
int nr,nw;              /* variabili per salvare valori di ritorno di read/write da/su pipe */

/* controllo sul numero di parametri almeno 1 file */
if (argc < 2)
{
	printf("Errore numero di parametri\n");
	exit(1);
}

N = argc-1;
printf("Numero di processi da creare %d\n", N);

/* allocazione pid */
if ((pid=(int *)malloc(N*sizeof(int))) == NULL)
{
	printf("Errore allocazione pid\n");
	exit(2); 
}

/* allocazione pipe */
if ((pipes=(pipe_t *)malloc(N*sizeof(pipe_t))) == NULL)
{
	printf("Errore allocazione pipe\n");
	exit(3); 
}

/* creazione pipe */
for (i=0;i<N;i++)
	if(pipe(pipes[i])<0)
	{
		printf("Errore creazione pipe\n");
		exit(4);
	}

/* creazione figli */
for (i=0;i<N;i++)
{
	if ((pid[i]=fork())<0)
	{
		printf("Errore creazione figlio\n");
		exit(5);
	}
	else if (pid[i]==0)
	{ /* codice figlio */
	printf("Sono il figlio %d\n", getpid());
	/* nel caso di errore in un figlio decidiamo di ritornare un valore via via crescente rispetto al massimo valore di i (che e' N-1) */
	/* chiusura pipes inutilizzate */
	for (j=0;j<N;j++)
	{
		if (j!=i)
			close (pipes[j][1]);
		if ((i == 0) || (j != i-1))
			close (pipes[j][0]);
	}
 
	
	/* apertura file */
	if ((fd=open(argv[i+1],O_RDONLY))<0)
	{
		printf("Impossibile aprire il file %s\n", argv[i+1]);
		exit(N);
	}
	
	/* inizializziamo il contatore di linee */
	cur_tot = 0L;
	 /* con un ciclo leggiamo tutte le linee e ne calcoliamo la lunghezza */
        while(read(fd,&ch,1) != 0)
        {
          	if (ch == '\n') /* siamo arrivati alla fine di una linea */
                {
			cur_tot++; /* incrementiamo il numero di linee */
                        }
        }
	if (i == 0)
	{  /* il figlio di indice 0 deve preparare la struttura da mandare al figlio seguente */
		letta.id = 0;
		letta.tot = cur_tot;
    	}
	else
 	{ /* tutti gli altri figli devono leggere dal figlio precedente una struttura */	
		nr=read(pipes[i-1][0],&letta,sizeof(s_tot));
	/* per sicurezza controlliamo il risultato della lettura da pipe */
		if (nr != sizeof(s_tot))
        	{	
        		printf("Figlio %d ha letto un numero di byte sbagliati %d\n", i, nr);
        		exit(N+1);
        	}
 /*
		printf("HO ricevuto da figlio di indice %d che ha calcolato %ld linee dal file %s\n", letta.id, letta.tot,  argv[i+1]);
  */
		if (letta.tot < cur_tot)
	        {  /* il figlio di indice i ha calcolato un numero di linee maggiore e quindi bisogna aggiornare i valori di letta */
        		letta.id = i;
        		letta.tot = cur_tot;
        	}	
	}
	/* tutti i figli mandano in avanti, l'ultimo figlio manda al padre una struttura */
	nw=write(pipes[i][1],&letta,sizeof(s_tot));
	/* anche in questo caso controlliamo il risultato della scrittura */
	if (nw != sizeof(s_tot))
        {
              	printf("Figlio %d ha scritto un numero di byte sbagliati %d\n", i, nw);
                exit(N+2);
        }
	/* ogni figlio deve tornare il proprio indice d'ordine */
	exit(i);
	}
} /* fine for */

/* codice del padre */
/* chiusura pipe: tutte meno l'ultima in lettura */
for(i=0;i<N;i++)
{
	close (pipes[i][1]);
	if (i != N-1) close (pipes[i][0]);
}

/* il padre deve leggere la struttura che gli arriva dall'ultimo figlio */
nr=read(pipes[N-1][0],&letta,sizeof(s_tot));
/* come prima effettuaiamo il controllo sulla lettura */
if (nr != sizeof(s_tot))
{
        printf("Padre ha letto un numero di byte sbagliati %d\n", nr);
        exit(6);
}
/* il padre deve stampare i campi della struttura ricevuta insieme con l'indicazione del pid del figlio e il file cui si riferiscono le informazioni ricevute */
printf("Il figlio di indice %d e pid %d ha trovato il numero massimo di linee %ld nel file %s\n", letta.id, pid[letta.id], letta.tot, argv[letta.id+1]);

/* Il padre aspetta i figli */
for (i=0; i < N; i++)
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
        	printf("Il figlio con pid=%d ha ritornato %d (se > di %d problemi)\n", pidFiglio, ritorno, N-1);
        } 
}
exit(0);
}