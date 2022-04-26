
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
typedef int pipe_t[2];
char buff[255];
pipe_t *piped;//pipe per la comunicazione padre-figlio
int numlinee; //indica a che linea siamo
int inviate; //numero di linee inviate al padre
int i; //indice per i processi
 //indice del buffer
void send(int sig)
{
    int dim=strlen(buff);
    ++dim;
    write(piped[i][1],buff,dim*sizeof(char));
    ++inviate;
}
void not_send(int sig)
{
    write(piped[i][1],&numlinee,sizeof(int));

}
int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc !=3)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
 
    int n=argc-1; //numero di processi figli da creare
    int fd; //file descriptor
    int *pid; //variabile che contiene il fid del figlio
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    bool *finito;
    int j;
    int nr,nw; //variabili per il controllo di numero di letture/scritture dalle pipe
    //controllo sui file
    for (i = 0; i < n; i++)
    {
        if((fd=open(argv[i+1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(2);
        }
        close(fd);
    }
    
    //allocazione della pipe padre figlio
    piped=malloc(n*sizeof(pipe_t));
    pid=malloc(n*sizeof(int));
    //allocazione dell'array booleano
    finito=malloc(n*sizeof(bool));
    if(finito==NULL || pid==NULL)
    {
        printf("Errore nella allocazione dell'array booleano oppure dell'array di pid \n");
        exit(3);
    }
    for ( i = 0; i < n; i++)
    {
       finito[i]=0;
    }
    
    //controllo se l'allocazione è andata a buon fine
    if(piped==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(4);
    }
    //creazione delle pipe
    for (i = 0; i < n; i++)
    {
        
        if(pipe(piped[i])<0)
        {
            printf("Errore nella creazione della pipe figlio padre\n");
            exit(5);
        }

    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);
    signal(SIGUSR1,send);
    signal(SIGUSR2,not_send);
    //creazione dei figli
    for ( i = 0; i < n; i++)
    {
        if((pid[i]=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(6);
        }
        //codice figlio
        if(pid[i]==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",i,getpid());
            
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int  j = 0; j < n; j++)
            {
               close(piped[j][0]);
                if(i!=j)
                {
                   close( piped[j][1]);
                }

            }
            //apertura del file da parte del figlio
            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(-1);
            }
            j=0;
            numlinee=0;
            inviate=0;
            //lettura dal padre
            while(read(fd,&buff[j],1))
            {
                if(buff[j]=='\n')
                {
                    ++numlinee;
                    buff[j]=0;
                    ++j;
                    write(piped[i][1],&j,sizeof(int));
                    pause();
                    j=0;
                }
                else
                {
                    ++j;
                }
            }

          exit(inviate); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (i = 0; i < n; i++)
    {
        close(piped[i][1]);

    }
    //recupero delle informazioni 
    int dim1,dim2;
    while(finito[0]!=1 && finito[1]!=1)
    {
        nr=read(piped[0][0],&dim1,sizeof(int));
        if(nr!=sizeof(int))
        {
            finito[0]=1;
        }
       nr=read(piped[1][0],&dim2,sizeof(int));
       if(nr!=sizeof(int))
        {
            finito[1]=1;
        }
        if(dim1==dim2)
        {
            if(finito[0]!=1 && finito[1]!=1)
           { sleep(1);
            kill(pid[0],SIGUSR1);
            kill(pid[1],SIGUSR1);
            nr=read(piped[0][0],buff,dim1*sizeof(char));
            printf("il figlio di indice 0 ha scritto la linea: %s\n",buff);
            if(nr!= (dim1*sizeof(char)))
            {
                finito[0]=1;
            }
            nr=read(piped[1][0],buff,dim2*sizeof(char));
             if(nr!= (dim2*sizeof(char)))
            {
                finito[1]=1;
            }
            printf("il figlio di indice 1 ha scritto la linea: %s\n",buff);
           }

        }
        else
        {
            sleep(1);
            kill(pid[0],SIGUSR2);
            kill(pid[1],SIGUSR2);
            read(piped[0][0],&numlinee,sizeof(int));
            printf("il figlio di indice 0 ha constatato che la linea numero %d è stata modificata\n",numlinee);
             read(piped[1][0],&numlinee,sizeof(int));
            printf("il figlio di indice 1 ha constatato che la linea numero %d è stata modificata\n",numlinee);
            
        }
    }
    //il padre aspetta i figli
   for (i = 0; i < n; i++)
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
             printf("il figlio con pid=%d ha ritornato il valore: %d\n",pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

