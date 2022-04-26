
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
typedef int pipe_t[2];
char buff[255];
int i; //indice per i processi
int linee; //conta quante linee vengono stampate
void stampa (int sig)
{
    printf("sono il figlio di indice %d e stampo la linea %s\n",i,buff);
    ++linee;
}

void nonfarenulla(int sig)
{
    ;
}
struct dati {
    int c1; //indice d'ordine del processo
    int c2; //numero di caratteri letti ad ogni linea
};

int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 3)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }

    int n=argc-2; //numero di processi figli da creare
    int fd; //file descriptor
    int *pid; //array di pid dei figli
    pipe_t *piped;//pipe per la comunicazione a pipeline: tutti i figli eccetto il primo leggono su pipe i-1 e scrivono su pipe i
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    int H=atoi(argv[n+1]);
    int nr,nw; //servono per controllare il numero di scritture/letture da pipe
    int j; //indice per il buffer
    struct dati d; //servirà per i processi figli e i proc padre 
  
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
    //controllo sull'ultimo file
    if(H<=0 || H>=255)
    {
        printf("Errore %d non è strettamente positivo o maggiore di 255\n",H);
        exit(3);
    }
    //allocazione della pipe padre figlio
    piped=malloc(n*sizeof(pipe_t));

    //controllo se l'allocazione è andata a buon fine
    if(piped==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(4);
    }
    //allocazione array di pid
    pid=malloc(n*sizeof(int));
    if(pid==NULL)
    {
        printf("Errore nella allocazione diamica dei pid\n");
        exit(5);
    }
    //creazione delle pipe
    for (i = 0; i < n; i++)
    {
       
        if(pipe(piped[i])<0)
        {
            printf("Errore nella creazione della pipe\n");
            exit(6);
        }

    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);
    //associo un handler ai segnali
    signal(SIGUSR1,stampa);
    signal(SIGUSR2,nonfarenulla);
    //creazione dei figli
    for ( i = 0; i < n; i++)
    {
        if((pid[i]=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(7);
        }
        //codice figlio
        if(pid[i]==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",i,getpid());
            
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int j = 0; j < n; j++)
            {
                if(j!=i-1 || i==0)
                {
                    close(piped[j][0]);
                }
                if(j!=i)
                {
                    close(piped[j][1]);
                }

            }
            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(-1);
            }
            j=0;
            while(read(fd,&buff[j],1))
            {
                if(buff[j]=='\n')
                {
                    buff[j]=0;
                    if(i==0)
                    {
                        d.c1=i;
                        d.c2=j;
                         if ((nw=write(piped[i][1],&d,sizeof(struct dati)))!=sizeof(struct dati))
                        {
                            printf("Errore nella scrittura su pipe\n");
                            exit(n+1);
                        }

                    }
                    else
                    {
                        if((nr=read(piped[i-1][0],&d,sizeof(struct dati)))!= sizeof(struct dati))
                        {
                            printf("errore nella lettura dalla pipe\n");
                            exit(n+1);
                        }
                        if(j>d.c2)
                        {
                            d.c1=i;
                            d.c2=j;
                        }
                        if ((nw=write(piped[i][1],&d,sizeof(struct dati))) !=sizeof(struct dati))
                        {
                            printf("Errore nella scrittura su pipe\n");
                            exit(n+2);
                        }
                        
                    }
                    pause();
                    j=0;
                    //sleep(1);
                }
                else
                {
                    ++j;
                }
            }


          exit(linee); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (int i = 0; i < n; i++)
    {
        close(piped[i][1]);
        if(i!=n-1)
        {
            close(piped[i][0]);
        }

    }
    //recupero delle informazioni 
    for (int j = 0; j < H; j++)
    {
        if((nr=read(piped[n-1][0],&d,sizeof(struct dati)))!=sizeof(struct dati))
        {
            printf("erorre di lettura dall'ultima pipe del padre\n");
            exit(8);
        }
        for (size_t i = 0; i < n; i++)
        {
            sleep(1); 
            if(d.c1==i)
            {
                kill(pid[i],SIGUSR1);
            }
            else
            {
                kill(pid[i],SIGUSR2);
            }
        }
        
     

    }

    //il padre aspetta i figli
   for (int i = 0; i < n; i++)
    {
    
        pidf=wait(&status);
        if(pidf<0)
         {
             printf("Errore nella wait\n");
             exit(9);
         }
         sleep(0.5);
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

    
    

    

