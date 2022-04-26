
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
typedef int pipe_t[2];

int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 3)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
 
    int n=argc-1; //numero di processi figli da creare
    int fd; //file descriptor
    int pid; //variabile che contiene il fid del figlio
    int i; //indice per i processi
    pipe_t *figliopadre;//pipe per la comunicazione figlio-padre
    pipe_t *nipotepadre;//pipe per la comunicazione nipote-padre
    int pidf,status; //variabili per recuperare il valore di ritorno del figlio
    int pidn;
    char Ch; //carattere per la lettura dei caratteri dai file
    char Ch2; //carattere utilizzato per la scrittura sui file
    long int change; //indicherà quante  sono le trasformazioni fatte
    long int ret; //variabile che contiene il ritorno del nipote e del figlio in termini di long int 

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
    
    //allocazione della pipe figlio-padre e padre-nipote
    figliopadre=malloc(n*sizeof(pipe_t));
    nipotepadre=malloc(n*sizeof(pipe_t));
    //controllo se l'allocazione è andata a buon fine
    if(figliopadre==NULL || nipotepadre==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(3);
    }
    //creazione delle pipe
    for (i = 0; i < n; i++)
    {
        if(pipe(figliopadre[i])<0)
        {
            printf("Errore nella creazione della pipe padre figlio\n");
            exit(4);
        }
        if(pipe(nipotepadre[i])<0)
        {
            printf("Errore nella creazione della pipe figlio padre\n");
            exit(5);
        }

    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);

    //creazione dei figli
    for ( i = 0; i < n; i++)
    {
        if((pid=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(6);
        }
        //codice figlio
        if(pid==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",i,getpid());
            
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int  j = 0; j < n; j++)
            {
                close(figliopadre[j][0]);
                close(nipotepadre[j][0]);
                if(j!=i)
                {
                    close(figliopadre[j][1]);
                    close(nipotepadre[j][1]);
                }

            }
            if((pid=fork())<0)
            {
                printf("Errore nella creazione del nipote\n");
                exit(-1);
            }
            if(pid==0)
            {
                printf("Sono il nipote del figlio di indice %d con pid=%d\n",i,getpid());
                close(figliopadre[i][1]);
                //apertura del file da parte del nipote
                if((fd=open(argv[i+1],O_RDWR))<0)
                {
                printf("Errore nell'apertura dei file %s\n",argv[i+1]);
                exit(-2);
                }
                //lettura del file da parte del nipote
                change=0;
                while(read(fd,&Ch,1)) 
                {
                    if(islower(Ch)) //se il carattere è minuscolo
                    {  
                        Ch2=toupper(Ch); //trasformo il carattere in maiuscolo 
                        lseek(fd,-1l,SEEK_CUR); //torno indietro di una posizione per modificare il carattere 
                        write(fd,&Ch2,1); //scrivo sul file il carattere trasformato
                        ++change; //incremento le trasformazioni
                    }
                }
                //il nipote scrive al padre 
                write(nipotepadre[i][1],&change,sizeof(long int ));
                //mando il valore di ritorno al figlio
                ret=change/256;
                exit(ret);


            }
            close(nipotepadre[i][1]);
            //il figlio apre il file
            if((fd=open(argv[i+1],O_RDWR))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(-2);
            }
             //lettura del file da parte del figlio
                change=0;

                while(read(fd,&Ch,1))
                {
                    if(isdigit(Ch)) //se il carattere è numerico 
                    { 
                        Ch2=' '; //inserisco lo spazio
                        lseek(fd,-1l,SEEK_CUR); //torno indietro di una posizione 
                        write(fd,&Ch2,1); //scrivo sul file 
                        ++change; //incremento le trasformazioni
                    }
                }
                 //il figlio scrive al padre 
                write(figliopadre[i][1],&change,sizeof(long int ));
        //il figlio deve attendere il nipote
        pidn=wait(&status);
        if(pidn<0)
         {
             printf("Errore nella wait\n");
             exit(-3);
         }
         if((status & 0xFF)!=0)
         {
             printf("il nipote con pid %d è terminato in modo anomalo\n",pidn);
         }
            else
         {
             ret=(long int) ((status >>8 ) & 0xFF);
             printf("il nipote con pid=%d ha ritornato il valore: %ld\n",pidn,ret);
         }  
         //il figlio manda il valore di ritorno
         ret=change/256;
         exit(ret);

         
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (i = 0; i < n; i++)
    {
        close(nipotepadre[i][1]);
        close(figliopadre[i][1]);
    }
    //recupero delle informazioni 
    for (i = 0; i < n; i++)
    {
     //il padre legge dalla pipe in comunicazione con il figlio e con il nipote 
     read(figliopadre[i][0],&change,sizeof(long int));
     printf("il figlio di indice %d associato al file %s ha trasformato %ld occorrenze di caratteri numerici\n",i,argv[i+1],change);
    read(nipotepadre[i][0],&change,sizeof(long int));
    printf("il nipote di indice %d associato al file %s ha trasformato %ld occorrenze di caratteri minuscoli\n",i,argv[i+1],change);

    }

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
             ret=(long int ) ((status >>8 ) & 0xFF);
             printf("il figlio con pid=%d ha ritornato il valore: %ld\n",pidf,ret);
         }       
    }

exit(0);
}

    
    

    

