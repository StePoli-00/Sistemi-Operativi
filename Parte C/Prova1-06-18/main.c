
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
typedef int pipe_t[2];

int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 4)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
    //variabili globali
    int n=argc-2; //numero di processi figli da creare
    int K=atoi(argv[n+1]); 
    int X; //il numero fornito dalll'utente
    int fd; //file descriptor
    int L; //variabile dove verrà contenula la lunghezza delle linee lette dal padre
    char buff[255]; //buffer per lettura del file da parte del figlio
    int *pid; //array di pid dei figli
    pipe_t *fp;//pipe per la comunicazione figlio-padree
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
    //controllo sull'ultimo parametro
    if(K<=0)
    {
        printf("Errore %d non strettamente positivo",K);
        exit(3);
    }
    //chiedo all'utente di fornire il numero
    printf("inserisci un numero strettamente positivo e minore o uguale a %d\n",K);
    //salvo il numero in una variabile
    scanf("%d",&X);
    if(X<=0 || X>K)
    {
        printf("Errore %d non strettamente positivo o maggiore di %d",X,K);
        exit(4);
    }
    //allocazione della pipe figlio-padre e dell'array di pid
    fp=malloc(n*sizeof(pipe_t));
    pid=malloc(n*sizeof(int));
    //controllo se l'allocazione è andata a buon fine
    if( fp==NULL || pid==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(5);
    }
    //creazione delle pipe
    for (size_t i = 0; i < n; i++)
    {
       
        if(pipe(fp[i])<0)
        {
            printf("Errore nella creazione della pipe figlio padre\n");
            exit(6);
        }

    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);

    //creazione dei figli
    for (int i = 0; i < n; i++)
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
            for (int  j = 0; j < n; j++)
            { //sempre qua l'errore non i ma j nelle close
                close(fp[j][0]);
                if(j!=i)
                {
                    close(fp[j][1]);
                }

            }
            //il figlio apre il file
            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(-1);
            }
            int j=0;
            //viene incrementato ogni volta che leggiamo una riga
            int riga=0;
            //servirà per distinguere i file con la riga X-esima da quelli che non ce l'hanno
            bool found=0;
            while(read(fd,&buff[j],1))
            {
                //se siamo alla fine della linea
                if(buff[j]=='\n')
                {
                    //incrementiamo le righe lette e controlliamo se siamo arrivati a quella desiderata
                    ++riga;
                    if(riga==X)
                    {
                        //abbiamo trovato la linea
                        found=1;
                        ++j;
                        // buff[j]=0;
                        // printf("%s\n",buff);
                        //printf("%d\n",j);
                        //scriviamo la lunghezza della linea e successivamente la linea e usciamo
                        write(fp[i][1],&j,sizeof(j));
                        write(fp[i][1],buff,j);
                        exit(j); 
                    }
                    //altrimenti ricominciamo a riempire il buffer
                     j=0;
                }
                else //se non abbiamo finito di leggere una linea incrementiamo
                { 
                    ++j;
                }

            }
            //se non abbiamo trovato la X-esima linea
            if(found==0)
            {
                 //costruiamo una stringa per far capire all'utente che non è presente la X-esima linea nel file analizzato
                sprintf(buff,"non esiste la linea %d nel file %s",X,argv[i+1]);
                //contiamo la dimensione della stringa
                j=strlen(buff);
                //incrementiamo per il terminatore
                ++j;
                // buff[j]=0;
                // printf("%s\n",buff);
                //scriviamo lunghezza e linea al padre
                write(fp[i][1],&j,sizeof(int));
                write(fp[i][1],buff,j);
                exit(0);
            }

          
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (int i = 0; i < n; i++)
    {
        close(fp[i][1]);
    }
    //recupero delle informazioni 
    L=0;
    for (int i = 0; i < n; i++)
    {
        //il padre legge la lunghezza della linea mandata dal figlio
        read(fp[i][0],&L,sizeof(int));
        //printf("%d\n",L);
        //il padre legge la linea grazie al dato della lunghezza
        read(fp[i][0],buff,L);
        //il padre usa l'info della lunghezza per mettere il terminatore
        buff[L-1]=0;
        //il padre scrive le informazioni lette
        printf("il figlio di indice %d con pid=%d  associato al file %s ha comunicato:%s\n",i,pid[i],argv[i+1],buff);
        
        

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
             printf("il  figlio di indice %d con pid=%d ha ritornato il valore %d(0 se la linea non esiste)\n",i,pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

