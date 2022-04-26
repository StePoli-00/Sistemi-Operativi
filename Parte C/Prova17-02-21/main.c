
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
typedef int pipe_t[2];

int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 3)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
    //variabili globali
    int Q=argc-1; //numero di processi figli da creare
    int q; //indice per i figli
    int fd; //file descriptor
    char ok='x'; //token per la comunicazione con i figli
    int pid; //variabile che contiene il fid del figlio
    int num; //conta le occorrenze dei numeri
    char linea[255]; //buffer per contenere la linea da leggere 
    int j; //indice per scorrere il buffer linea
    pipe_t *piped;//pipe per la comunicazione a ring: i figli leggono da q e scrivono sulla pipe q+1%Q
    int nr,nw; //servono per controllare il numero di letture/scritture delle pipe
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    

    //controllo sui file
    for (q=0;q<Q;++q)
    {
        if((fd=open(argv[q+1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[q+1]);
            exit(2);
        }
        close(fd);
    }
    
    //allocazione della pipe padre figlio
    piped=malloc(Q*sizeof(pipe_t));
    //controllo se l'allocazione è andata a buon fine
    if(piped==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(3);
    }
    //creazione delle pipe
    for (q=0;q<Q;++q)
    {
        if(pipe(piped[q])<0)
        {
            printf("Errore nella creazione della pipe padre figlio\n");
            exit(4);
        }
        

    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),Q);

    //creazione dei figli
    for (q=0;q<Q;++q)
    {
        if((pid=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(5);
        }
        //codice figlio
        if(pid==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",q,getpid());
            sleep(0.8);
            
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int  j = 0; j < Q; j++)
            {
               if(j!=q)
               {
                close(piped[j][0]);
               }
               if(j!=((q+1)%Q))
               {
                   
                   close(piped[j][1]);
               }

            }
            // il figlio apre il file associato
            if((fd=open(argv[q+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[q+1]);
            exit(-1);
            }
            j=0;
            num=0;
            //lettura del file
            while(read(fd,&linea[j],1))
            {
                
                //se il carattere è numerico incremento il contatore
                if(isdigit(linea[j]))
                {
                    ++num;
                }

                //controllo se sono arrivato alla fine della linea
                if(linea[j]=='\n')
                {
                    linea[j]=0;
                    nr=read(piped[q][0],&ok,1);
                    if(nr!=1)
                    {
                        printf("Errore nella lettura del carattere di stampa\n");
                        exit(-1);
                    }
                    printf("il figlio di indice %d e pid=%d ha letto %d caratteri numerici nella linea:%s\n",q,getpid(),num,linea);
                    num=0;
                    nw=write(piped[(q+1)%Q][1],&ok,1);
                    if(nw!=1)
                    {
                        printf("Errore nella scrittura del carattere di stampa\n");
                        exit(-1);
                    }
                    j=0;
                    
                }
                else
                {
                    ++j;
                }
            }

          exit(num); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    //partiamo da q=1 per evitare la sigpipe da parte del sistema,in caso l'ultimo figlio
    //scrivesse e il padre chiudesse la pipe di lettura e per innescare la comunicazione a ring
    
    for (q=1;q<Q;++q)
    {
       
        close(piped[q][0]);
        close(piped[q][1]);

    }
    //il padre innesca la comunicazione a ring
    write(piped[0][1],&ok,1);
    close(piped[0][1]);
    

    //il padre aspetta i figli
   for (q=0;q<Q;++q)
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
             sleep(0.8);
             printf("il  figlio di indice %d con pid=%d ha trovato nell'ultima linea letta %d caratteri numerici (<=255 se ci sono stati degli errori)\n",q,pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

