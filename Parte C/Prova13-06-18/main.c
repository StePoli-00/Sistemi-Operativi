
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#define PERM 0644
typedef int pipe_t[2];

int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 0)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
    //variabili globali
    int n; //numero di parametri
    int fd; //file descriptor
    int pid; //variabile che contiene il fid del figlio
    //int *pid; //array di pid dei figli
    pipe_t *pf;//pipe per la comunicazione padre-figlio
    pipe_t *fp;//pipe per la comunicazione figlio-padre
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    

    //controllo sui file
    for (int i = 0; i < n; i++)
    {
        if((fd=open(argv[i+1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(2);
        }
        close(fd);
    }
    
    //allocazione della pipe padre figlio
    pf=malloc(n*sizeof(pipe_t));
    fp=malloc(n*sizeof(pipe_t));
    //controllo se l'allocazione è andata a buon fine
    if(pf==NULL || fp==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        //exit();
    }
    //creazione delle pipe
    for (int i = 0; i < n; i++)
    {
        if(pipe(pf[i])<0)
        {
            printf("Errore nella creazione della pipe padre figlio\n");
            //exit();
        }
        if(pipe(fp[i])<0)
        {
            printf("Errore nella creazione della pipe figlio padre\n");
            //exit();
        }

    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);

    //creazione dei figli
    for (int i = 0; i < n; i++)
    {
        if((pid=fork())<0)
        {
           printf("Errore nella fork\n");
            //exit();
        }
        //codice figlio
        if(pid==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",i,getpid());
            
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int  j = 0; j < n; j++)
            {
                close(pf[j][0]); //se è scrittore
                close(pf[j][1]); // se è lettore
                if(j!=i)
                {
                    close(pf[j][1]);
                    close(pf[j][0]);
                }

            }

            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(-1);
            }

          exit(0); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (int i = 0; i < n; i++)
    {
        close(pf[i][0]);
        close(fp[i][1]);

    }
    //recupero delle informazioni 
    for (int i = 0; i < n; i++)
    {
     
     //read(pf[i][0])

    }

    //il padre aspetta i figli
   for (int i = 0; i < n; i++)
    {
    
        pidf=wait(&status);
        if(pidf<0)
         {
             printf("Errore nella wait\n");
             //exit();
         }
         if((status & 0xFF)!=0)
         {
             printf("il figlio con pid %d è terminato in modo anomalo\n",pidf);
         }
         else
         {
             ritorno=(int) ((status >>8 ) & 0xFF);
             printf("il  figlio di indice %d con pid=%d ha ritornato il valore: %d\n",i,pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

