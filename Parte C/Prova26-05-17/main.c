
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

typedef int pipe_t[2];
struct dati
{
    long int c1; //valiore massimo delle occorrenze calcolato dal processo
    long int c2; //indice d'ordine del processo che ha calcolato il  massimo
    long int c3;//somma di tutte le occorrenze calcolate dai processi
};
int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 4)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
    //variabili globali
    int n=argc-2; //numero di processi figli da controllare
    char Cx; //conterrà l'ultimo parametro
    int i; //indice per i processi
    int fd; //file descriptor
    int nr,nw; //varibili per controllare le letture e scritture dei figli dalla pipe
    int *pid; //array di pid dei figli
    pipe_t *piped;//pipe per la comunicazione in pipeline: i figli eccetto il primo leggono da pipe di i-1 e scrivono su pipe di i 
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    char ch; // per la lettura dei caratteri dal file da parte dei figli
    struct dati d; //struct usata dai figli e dal padre per la read dall'ultimo figlio
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
    //controllo sull'ultimo parametro che sia un singolo carattere
    int dim=strlen(argv[n+1]);
    if(dim!=1)
    {
        printf("Errore %s non è un singolo carattere\n",argv[n+1]);
        exit(3);
    }
    Cx=argv[n+1][0];
    //allocazione della pipe per la comunicazione in pipeline
    piped=malloc(n*sizeof(pipe_t));
    //controllo se l'allocazione è andata a buon fine
    if(piped==NULL)
    {
        printf("Errore nella allocazione dinaminca della pipe\n");
        exit(4);
    }
    //creazione delle pipe
    for ( i = 0; i < n; i++)
    {
        if(pipe(piped[i])<0)
        {
            printf("Errore nella creazione della pipe padre figlio\n");
            exit(5);
        }
       

    }
    //allocazione dell'array di pid
    pid=malloc(n*sizeof(int));
    if(pid==NULL)
    {
        printf("Errore nella allocazione dinamica dell'array di pid\n");
        exit(6);
    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);

    //creazione dei figli
    for (i = 0; i < n; i++)
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
                if(i==0)
                {
                    close(piped[j][0]);
                }
                else if(j!=i-1)
                {
                    close(piped[j][0]);
                }
                if(j!=i)
                {
                    close(piped[j][1]);
                }

            }
            //apertura del file da parte del figlio
            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(-1);
            }
           
            long int occ=0;
            //lettura del file
            while(read(fd,&ch,1))
            {
                if(ch==Cx)
                {
                 ++occ;   
                }
               
            }

            if(i==0)
            {
               
                d.c1=occ;
                d.c2=i;
                d.c3=occ;
                //printf("%dleggo:\n%ld\n%ld\n%ld\n",i,d.c1,d.c2,d.c3);
                nw=write(piped[i][1],&d,sizeof(struct dati));
                if(nw!= sizeof(struct dati))
                {
                    printf("Errore di scrittura sulla pipe da parte del figlio di indice %d\n",i);
                    exit(n+2);
                }
            }
            else
            {
                nr=read(piped[i-1][0],&d,sizeof(struct dati));
                //printf("%d leggo \n%ld\n%ld\n%ld\n",i,d.c1,d.c2,d.c3);
                if(nr!= sizeof(struct dati))
                {
                    printf("Errore di lettura sulla pipe da parte del figlio di indice %d\n",i);
                    exit(n+1);
                }
                if(occ>d.c1)
                {
                    d.c1=occ;
                    d.c2=i;
                }
                d.c3+=occ;
                //printf("%d scrivo:\n%ld\n%ld\n%ld\n",i,d.c1,d.c2,d.c3);
                nw=write(piped[i][1],&d,sizeof(struct dati));
                 if(nw!= sizeof(struct dati))
                {
                    printf("Errore di scrittura sulla pipe da parte del figlio di indice %d\n",i);
                    exit(n+2);
                }
            }

          exit(i); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for ( i = 0; i < n; i++)
    {
         close(piped[i][1]);
        if(i!=n-1)
        {
           
            close(piped[i][0]);
        }
    }
    //recupero delle informazioni 
    nr=read(piped[n-1][0],&d,sizeof(struct dati));
    if(nr!= sizeof(struct dati))
     {
         printf("Errore di lettura dall' ultima pipe da parte del dal padre\n");
         exit(7);
     }
     for(int i=0;i<n;++i)
     {
         if(i==d.c2)
         {
              printf("il figlio di indice %d con pid=%d ha trovato il numero massimo di %ld occorrenze del carattere %c nel file %s\n",i,pid[i],d.c1,Cx,argv[i+1]);
              printf("I figli hanno trovato in totale %ld occorrenze del carattere %c nei file\n",d.c3,Cx);

         }
     }

    //il padre aspetta i figli
   for (int i = 0; i < n; i++)
    {
    
        pidf=wait(&status);
        if(pidf<0)
         {
             printf("Errore nella wait\n");
             exit(8);
         }
         if((status & 0xFF)!=0)
         {
             printf("il figlio con pid %d è terminato in modo anomalo\n",pidf);
         }
         else
         {
             ritorno=(int) ((status >>8 ) & 0xFF);
             printf("il  figlio con pid=%d ha ritornato il valore:%d (se > di %d problemi)\n",pidf,ritorno,n-1);
         }       
    }

exit(0);
}

    
    

    

