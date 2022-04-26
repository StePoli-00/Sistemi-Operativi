
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
typedef int pipe_t[2];
char Cx;
int occ;
int q; //indice dei processi
void stampa(int sig)
{
    printf("il figlio di indice %d ha trovato %d occorrenze del carattere %c\n",q,occ,Cx);

}
void continua(int sig)
{
    ;
}

int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 5)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
    //variabili globali
    int Q=argc-3; //numero di processi figli da creare
    int fd; //file descriptor
   
    int *pid; //variabile che contiene il fid del figlio
    int L=atoi(argv[2]);
    //pipe_t *padrefiglio;//pipe per la comunicazione padre-figlio
    //pipe_t *figliopadre;//pipe per la comunicazione figlio-padre
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    char Ch;
    int pidp=getpid();
    
    

    //controllo sul file
        if((fd=open(argv[1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[1]);
            exit(2);
        }
        close(fd);
    //controllo sul secondo parametro
    if(L<=0)
    {
        printf("Errore il parametro %d non è strettamente positivo\n",L);
        exit(3);
    }
    pid=malloc(Q*sizeof(int));
    if(pid==NULL)
    {
        printf("Errore nella allocazione dinamica dell'array di pid");
        exit(4);
    }
  
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),Q);
    signal(SIGUSR1,stampa);
    signal(SIGUSR2,continua);
    //creazione dei figli
    for (q = 0; q < Q; q++)
    {
        if((pid[q]=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(5);
        }
        //codice figlio
        if(pid[q]==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",q,getpid());
            
        

            if((fd=open(argv[1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[1]);
            exit(-1);
            }
            Cx=argv[3+q][0];
            //printf("%c\n",Cx);
            occ=0;
            while(read(fd,&Ch,1))
            {
                if(Ch==Cx)
                {
                    ++occ;
                }
                if(Ch=='\n')
                {
                    pause();
                    sleep(1);
                    kill(pidp,SIGUSR2);
                    ritorno=occ;
                    occ=0;
                }
            }
          exit(ritorno); 
        }
                
    }
 
    
    for (q=1;q<=L;++q)
    {
        printf("la %d linea:\n",q);
        for ( int j = 0; j< Q;++j)
        {
            sleep(1);
            kill(pid[j],SIGUSR1);
            pause();
            
        }
    }

    //il padre aspetta i figli
   for ( q= 0; q < Q; q++)
    {
    
        pidf=wait(&status);
        if(pidf<0)
         {
             printf("Errore nella wait\n");
             exit(6);
         }
         if((status & 0xFF)!=0)
         {
             printf("il figlio con pid %d è terminato in modo anomalo\n",pidf);
         }
         else
         {
             ritorno=(int) ((status >>8 ) & 0xFF);
             printf("il  figlio con pid=%d ha ritornato il valore:%d\n",pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

