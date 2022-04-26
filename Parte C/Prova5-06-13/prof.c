/* Soluzione della Prova d'esame del 5 Giugno 2013 - Parte C */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

typedef int pipe_t[2];

int main(int argc, char **argv) 
{
	/* -------- Variabili locali ---------- */
	int pid;			/* process identifier per le fork() */
	int N; 				/* numero di file passati sulla riga di comando (uguale al numero di file) */
	int status;			/* variabile di stato per la wait */
	pipe_t *piped;			/* array dinamico di pipe descriptors */
	int i, j;			/* indici per i cicli */
	char ch;			/* variabile per leggere i caratteri della prima linea inviati dal figlio al padre */
	/* ------------------------------------ */
	
	/* Controllo sul numero di parametri */
	if (argc < 2) /* Meno di due parametri */  
	{
		printf("Errore nel numero dei parametri\n");
		exit(1);
	}

	/* Calcoliamo il numero di file passati */
	N = argc - 1;
	
	/* Allocazione dell'array di N pipe descriptors*/
	piped = (pipe_t *) malloc (N*sizeof(pipe_t));
	if (piped == NULL)
	{
		printf("Errore nella allocazione della memoria\n");
		exit(2);
	}
	
	/* Creazione delle N pipe padre-figli */
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
			/*
			printf("Sono il processo figlio di indice %d e pid %d sto per leggere la prima linea del file %s\n", i, getpid(), argv[i+1]);
			*/
/* ogni figlio deve simulare il piping dei comandi nei confronti del padre e quindi deve chiudere lo standard output e quindi usare la dup sul lato di scrittura della propria pipe */
		close(1);
		dup(piped[i][1]); 			
			/* Chiusura di tutte le pipe */
			for (j=0; j < N; j++)
			{
				close(piped[j][0]);
				close(piped[j][1]);
			}
	/* Ridirezione dello standard error su /dev/null (per evitare messaggi di errore a video)*/
		close(2);
		open("/dev/null", O_WRONLY);
				
	/* Il figlio diventa il comando head */				
		execlp("head", "head", "-1", argv[i+1], (char *)0);
				
	/* attenzione all'ordine dei parametri nella esecuzione di head: prima
-1 e poi nome del file e quindi terminatore della lista. Il file si trova usando
l'indice i incrementato di 1 (cioe' per il primo processo i=0 il file e'
argv[1]) */
				
		/* Non si dovrebbe mai tornare qui!!*/
		exit(1); /* torno un valore diverso da zero per indicare insuccesso*/
}
	}
	
	/* Codice del padre */
	for (i=0; i < N; i++)
		close(piped[i][1]);
	
	/* Il padre recupera le informazioni dai figli in ordine di indice */
	for (i=0; i < N; i++)
	{
		/* il padre recupera tutti i caratteri spediti da ogni figlio fino al carattere terminatore di linea '\n': ricordarsi che il figlio li scrive su standard output che pero' è stato collegato alla pipe che legge il padre */
		printf("Il figlio di indice %d ha letto dal file %s la linea:\n", i, argv[i+1]);
		do 
 		{
			read(piped[i][0], &ch, 1);
			printf("%c", ch);
		}
		while (ch != '\n');
	}
	
	/* Il padre aspetta i figli */
	for (i=0; i < N; i++)
	{
		pid = wait(&status);
		if (pid < 0)
		{
		printf("Errore in wait\n");
		exit (5);
		}

		if ((status & 0xFF) != 0)
    		printf("Figlio con pid %d terminato in modo anomalo\n", pid);
    		else
		printf("Il figlio con pid=%d ha ritornato %d\n", pid, (int)((status >> 8) &	0xFF));
	}
}