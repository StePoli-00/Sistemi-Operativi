//quando fai un array di struct nella read e nella write non devi scrivere &d, ma direttamente d!!

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
typedef int pipe_t[2];
struct dati {
    int c1; //conterrà l'indice del processo
    int c2;//primo carattere numerico trovato
};

int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 2)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
 
    int n=argc-1; //numero di processi figli da creare
    int fd; //file descriptor
    int *pid; //array di pid dei figli
    int i; //indice per i processi
    pipe_t *piped;//pipe per la comunicazione in pipeline: tutti i figli (eccetto il primo )leggono da pipe[i-1]
 //e scrivono su pipe[i] e il padre legge da pipe[n-1]
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    char Cn; //carattere utilizzato per la read dei figli
    struct dati *d; //struct utilizzata dai figli e dal padre
    int nr,nw; //variabili per il controllo del numero di letture/scritture dei figli e del padre dalla pipe
    int sum;
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
    
    //allocazione della pipe e dell'array di pid
    piped=malloc(n*sizeof(pipe_t));
    pid=malloc(n*sizeof(int));
    
    //controllo se l'allocazione è andata a buon fine
    if(piped==NULL || pid==NULL)
    {
        printf("Errore nella allocazione della pipe o dell'array di pid\n");
        exit(3);
    }
    //creazione delle pipe
    for (i = 0; i < n; i++)
    {
        if(pipe(piped[i])<0)
        {
            printf("Errore nella creazione della pipe padre figlio\n");
            exit(4);
        }
    
    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);

    //creazione dei figli
    for ( i = 0; i < n; i++)
    {
        if((pid[i]=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(5);
        }
        //codice figlio
        if(pid[i]==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",i,getpid());
            
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int  j = 0; j < n; j++)
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
            //il figlio apre il file
            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(-1);
            }
            //lettura del file
            while(read(fd,&Cn,1))
            {
                if(isdigit(Cn))
                {
                    break;
                }
            }
            if(i==0)
            {
                d=malloc(sizeof(struct dati));
                if(d==NULL)
                {
                    printf("Errore nella allocazione dell'array di struct del figlio\n");
                    exit(-2);
                }
                d[i].c1=i;
                d[i].c2=atoi(&Cn);
                //printf("%d %d\n",d[i].c1,d[i].c2);
                
            }
            else
            {

                d=malloc((i+1)*sizeof(struct dati));
                if(d==NULL)
                {
                    printf("Errore nella allocazione dell'array di struct del figlio\n");
                    exit(-2);
                }
                if((nr=read(piped[i-1][0],d,(i*sizeof(struct dati))))!= (i* sizeof(struct dati)))
                {
                    printf("Errore nella lettura dalla pipe\n");
                    exit(-3);
                }

                d[i].c1=i;
                d[i].c2=atoi(&Cn);
                // for (int  j = 0; j < i; j++)
                // {
                //     printf(" %d:%d %d\n",i,d[j].c1,d[j].c2);
                // }
                
            }
            if((nw=write(piped[i][1],d,(i+1)*sizeof(struct dati)))!=((i+1)* sizeof(struct dati)))
            {
                printf("Errore nella scrittura sulla pipe\n");
                exit(-4);
            }
          exit(d[i].c2); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (i = 0; i < n; i++)
    {
        close(piped[i][1]);
        if(i!=n-1)
        {
            close(piped[i][0]);
        }

    }
    d=malloc(n*sizeof(struct dati));
    if(d==NULL)
    {
        printf("Errore nella allocazione della struct nel padre\n");
        exit(6);
    }
    if((nr=read(piped[n-1][0],d,n*sizeof(struct dati)))!= (n* sizeof(struct dati)))
    {
        printf("Errore nella lettura dalla pipe\n");
        exit(7);
    }
    sum=0;
    for (i = 0; i < n; i++)
    {
     
     printf("il figlio di indice %d con pid=%d ha ritornato la struct:\nc1=%d\nc2=%d\n",i,pid[i],d[i].c1,d[i].c2);
    sum+=d[i].c2;
    }
    printf("la somma di tutti i caratteri numerici trovati dai figli è %d\n",sum);

    //il padre aspetta i figli
   for (i = 0; i < n; i++)
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
             printf("il figlio con pid=%d ha ritornato il valore: %d (< di 255 errori)\n",pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

