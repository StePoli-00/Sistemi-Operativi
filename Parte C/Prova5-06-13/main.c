#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
typedef int pipe_t[2];

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
    int pid; //variabile che contiene il fid del figlio
    int i; //indice per i processi
    pipe_t *figliopadre;//pipe per la comunicazione padre-figlio
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    char linea[255];//buffer che conterrà la linea
    int j=0; //indice di linea

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
    figliopadre=malloc(n*sizeof(pipe_t));
    //controllo se l'allocazione è andata a buon fine
    if(figliopadre==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(3);
    }
    //creazione della pipe
    for (i = 0; i < n; i++)
    {
        
        if(pipe(figliopadre[i])<0)
        {
            printf("Errore nella creazione della pipe figlio padre\n");
            exit(4);
        }

    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);

    //creazione dei figli
    for ( i = 0; i < n; i++)
    {
        if((pid=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(5);
        }
        //codice figlio
        if(pid==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",i,getpid());
            
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int  j = 0; j < n; j++)
            {
                close(figliopadre[j][0]);
                if(j!=i)
                {
                    close(figliopadre[j][1]);
                }

            }
            //chiudo lo stdout duplico il lato scrittura della pipe e chiudo dopo la duplicazione
            
            close(1);
            dup(figliopadre[i][1]);
            close(figliopadre[i][1]);
            //eseguo la head sulla prima linea
            execlp("head","head","-1",argv[i+1],(char *)0);
            //riporto un messaggio di errore in caso di insuccesso della exec
            perror("errore nella exec del figlio\n");
            exit(-1);
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (i = 0; i < n; i++)
    {
      close(figliopadre[i][1]);

    }
    //recupero delle informazioni 
    for (i = 0; i < n; i++)
    {
     //setto j a zero per ogni giro
     j=0;
     while(read(figliopadre[i][0],&linea[j],1)) //comincio la lettura dalla pipe iesima
     {
         if(linea[j]=='\n') //quando incontro l'acapo metto il terminatore ed esco dal ciclo
        {
            linea[j]=0;
            break;
        }
        else
        {
            ++j; //altrimenti incremento
        }
     }
     //riporto le informazioni riportate dal figlio
    printf("il figlio di indice %d associato al file %s ha ritornato la linea: %s\n",i,argv[i+1],linea);
    }

    //il padre aspetta i figli
   for (i = 0; i < n; i++)
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
             printf("il figlio con pid=%d ha ritornato il valore: %d\n",pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

