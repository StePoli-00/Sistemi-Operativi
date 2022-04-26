
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
    long int c1; //valore massimo delle linee 
    int c2; //indice del processo che ha calcolato il valore massimo delle linee
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
    pipe_t *piped;//pipe per la comunicazione in pipeline: tutti i figli eccetto il primo leggono da pipe di i-1 e scrivono su pipe di i
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    char Ch; // conterrà i caratteri letti da file
    long int linee; //conterrà il num di linee lette dal singolo processo
    struct dati d; //struct usata dai processi e dal padre con l'ultima lettura
    int nr,nw; //conterranno i valori di ritorno delle letture/scritture dalle pipe
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
    //allocazione dell'array di pid
    pid=malloc(n*sizeof(int));
    //controllo se l'allocazione è andata a buon fine
    if(piped==NULL || pid==NULL)
    {
        printf("Errore nella allocazione della pipe o dell'array di pid\n");
        exit(3);
    }
    //creazione della pipe 
    for (i = 0; i < n; i++)
    {
        if(pipe(piped[i])<0)
        {
            printf("Errore nella creazione della pipe\n");
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
            //apertura del file da parte del figlio
            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura del file %s\n",argv[i+1]);
            exit(n);
            }
            linee=0;
            //il figlio legge il file
            while(read(fd,&Ch,1))
            {
                if(Ch=='\n') //ogni volta che incontra il /n incrementa il numero di linee 
                {
                    ++linee;
                }

            }
            if(i==0) //se siamo il figlio di indice 0 inizializziamo la struct
            {
                d.c1=linee;
                d.c2=i;
            }
            else
            {
                //altrimenti leggiamo la struct dal figlio precedente
                nr=read(piped[i-1][0],&d,sizeof(struct dati));
                //controlliamo di aver letto  correttamente la struct 
                if(nr!=sizeof(struct dati))
                {
                    printf("Errore nella lettura della struct dalla pipe\n");
                    exit(n+1);
                }
                //se le linee del processo corrente sono maggiori di quelle calcolate dai processi precedenti
                if(linee>d.c1)
                {
                    //scriviamo i valori del processo corrente
                    d.c1=linee;
                    d.c2=i;
                }
        
            }
            //scrittura della struct sulla pipe 
           nw=write(piped[i][1],&d,sizeof(struct dati));
           //controlliamo la scrittura della struct dalla pipe 
           if(nw!=sizeof(struct dati))
           {
               printf("Errore nella scrittura della struct dalla pipe\n");
                exit(n+2);
           }
           //ritorniamo l'indice del processo figlio
          exit(i); 
          
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (i = 0; i < n; i++)
    {
        if(i!=n-1)
        {
            close(piped[i][0]);
        }
        close(piped[i][1]);

    }
    //recupero delle informazioni 
    //il padre legge dal figlio n-1 la struct
     nr=read(piped[n-1][0],&d,sizeof(struct dati));
        //controlla che la lettura sia andata a buon fine 
        if(nr!= sizeof(struct dati))
           {
               printf("Errore nella lettura della struct dalla pipe da parte del padre\n");
                exit(6);
           }
           //comunica all'utente le informazioni nella struct con anche il pid e il nome del processo che ha trovato il numero di linee massime 
        printf("il figlio di indice %d con pid=%d ha constatato che fra tutti file,quello che ha più linee è il file %s con %ld linee \n",d.c2,pid[d.c2],argv[d.c2+1],d.c1);

    //il padre aspetta i figli
   for (i = 0; i < n; i++)
    {
    
        pidf=wait(&status);
        if(pidf<0)
         {
             printf("Errore nella wait\n");
             exit(7);
         }
         if((status & 0xFF)!=0)
         {
             printf("il figlio con pid %d è terminato in modo anomalo\n",pidf);
         }
         else
         {
             ritorno=(int) ((status >>8 ) & 0xFF);
             printf("il figlio con pid=%d ha ritornato il valore: %d (> di %d problemi )\n",pidf,ritorno,n-1);
         }       
    }

exit(0);
}

    
    

    

