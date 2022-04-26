/* Soluzione della Prova d'esame del 17 Giugno 2020 - Parte C */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ctype.h>
#define PERM 0644

typedef int pipe_t[2];

int main(int argc, char **argv)
{
        /* -------- Variabili locali ---------- */
	int *pid;               	/* array di pid per fork */
        int L;                          /* lunghezza del file */
        int B;                          /* numero di processi da creare */
        int pidFiglio, status;          /* variabili per la wait */
	pipe_t *p;			/* array dinamico di pipe per la comunicazione fra figli e padre */
	char *file;	 	   	/* array dinamico per nome file da creare (per chi faceva solo parte C) o aprire (per chi faceva tutto) */
	char *buff; 	   		/* array dinamico per il blocco corrente */
	char ch; 	   		/* carattere usato dal padre per leggere dalle pipe */
	int fd;                         /* file descriptor che serve ai figli */
	int fdw;                        /* file descriptor che serve al padre */
        int q, k;                       /* indici per i cicli */
        int ritorno=0;                  /* variabile che viene ritornata da ogni figlio al padre */
       	/* ------------------------------------ */

	/* controllo parametri: devono essere esattamente 1 file e 2 numeri e quindi argc deve essere uguale a 4 */
	if (argc != 4) 
        {
                printf("Errore nel numero dei parametri %d\n", argc);
                exit(1);
        }

        /* convertiamo la seconda stringa in un numero */
        L = atoi(argv[2]);
        /* controlliamo il numero L: deve essere strettamente positivo */
        if (L <= 0) 
        {
                printf("Errore nel primo numero passato %d\n", L);
                exit(2);
        }

        /* convertiamo la terza stringa in un numero */
        B = atoi(argv[3]);
        /* controlliamo il numero B: deve essere strettamente positivo */
        if (B <= 0) 
        {
                printf("Errore nel secondo numero passato %d\n", B);
                exit(3);
        }

	printf("Numero processi da creare %d\n", B);
	
	/* allocazione pid */
	if ((pid=(int *)malloc(B*sizeof(int))) == NULL)
	{
        	printf("Errore allocazione pid\n");
        	exit(4);
	}

	/* allocazione dinamica memoria per pipe */
	p=(pipe_t*)malloc(B*sizeof(pipe_t));
	if (p==NULL)
	{
		printf("Errore nelle malloc\n");
		exit(5);
	}
		
	/* creazione pipe */
	for (q=0; q < B; q++)
	{
		if(pipe(p[q])!=0)
	        {
                	printf("Errore creazione delle pipe\n"); 
			exit(6);
        	}
	}
	
	/* allocazione dinamica memoria per file: +1 per carattere '.' e +1 per terminatpre di stringa */
	file=(char*)malloc((strlen(argv[1])+1+strlen("Naomi")+1)*sizeof(char));
	if (file==NULL)
	{
		printf("Errore nella malloc per nome file su cui il padre deve scrivere\n");
		exit(7);
	}

	sprintf(file, "%s.Naomi", argv[1]);
	/* creazione file (per chi faceva solo parte C) o sola apertura (per chi faceva tutto) */
	if ( (fdw= open (file, O_CREAT | O_WRONLY, PERM)) < 0 )
        {
        	printf("Errore in creazione/apertura %s\n", file);
        	exit(8);
        }
	
	/* creazione figli */
	for (q=0; q < B; q++)
	{
		if((pid[q]=fork())<0)
		{
			printf("Errore nella fork %d-esima\n", q);
			exit(9);
		}	

		if(pid[q]==0)
		{
			printf("Figlio con indice %d e pid %d analizza il blocco %d-esimo del file %s\n",q,getpid(),q, argv[1]);
			for (k=0; k < B; k++)
			{
				/*chiudo i lati inutilizzati*/
				close(p[k][0]);
				if (k != q)
					close(p[k][1]);
			}
			 
			/* il figlio deve aprire il file in lettura */
			if((fd=open(argv[1],O_RDONLY))<0)
			{
				printf("Errore apertura file %s", argv[1]);
				exit(0);  /* decidiamo di tornare 0 che verra' interpretato dal padre come valore non ammissibile! */
			}
			
			/* allocazione dinamica memoria per blocco */
			buff=(char *)malloc(L/B*sizeof(char));
			if (p==NULL)
			{
				printf("Errore nella malloc per il blocco\n");
				exit(0);
			}

			lseek(fd, (long)q*L/B, SEEK_SET); 	/* ci si deve posizionare nella posizione giusta */
			read(fd,buff,L/B); 			/* si legge il blocco di competenza di questo processo */
	  		write(p[q][1],&buff[0],1);		/* mandiamo al padre il primo carattere del blocco */
			/* ritorno al padre */
			exit(L/B);
		}
	}

	/* codice del padre */
	/* chiude lati che non usa */
	for(q=0;q < B; q++)
		close(p[q][1]);

	/* si legge dalle pipe tutti i caratteri inviati dai figli */
	for(q=0; q < B; q++)
	{	
		read(p[q][0],&ch,1);
		/* stampa di debug */	
		printf("DEBUG-Il figlio di indice %d e pid %d ha letto dal file %s come ultimo carattere del blocco %d-esimo il carattere %c\n", q, pid[q], argv[1],q,ch);
		/* scriviamo sul file creato/aperto */
		write(fdw, &ch,1);

	}	

	/* Attesa della terminazione dei figli */
	for(q=0; q < B; q++)
	{
   		pidFiglio = wait(&status);
   		if (pidFiglio < 0)
   		{
      			printf("Errore wait\n");
      			exit(10);
   		}
		for (k=0; k < B; k++)
		{
			if (pid[k] == pidFiglio)
			{
 	  			if ((status & 0xFF) != 0)
        	        	printf("Figlio con pid %d e indice %d terminato in modo anomalo\n", pidFiglio, k);
   				else
   				{
               				ritorno=(int)((status >> 8) & 0xFF);
               				if (ritorno==0)
                                		printf("Il figlio con pid=%d e indice %d ha ritornato %d e quindi vuole dire che ci sono stati dei problemi\n", pidFiglio, k, ritorno);
                        		else  printf("Il figlio con pid=%d e indice %d ha ritornato %d\n", pidFiglio, k, ritorno);
				}	
			}	
		}	
	}	
exit(0);
}/* fine del main */