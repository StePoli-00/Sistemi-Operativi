#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
struct dati {
    long int c1; //conta occorrenze di caratteri ascii pari 
    long int c2; //conta occorrenze di caratteri ascii dispari 
};
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
    pipe_t *piped;//pipe per la comunicazione figlio-padre
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    struct dati d;
    char Ch; //variabile dove verranno messi i caratteri nella lettura del file
    int pos; //indicherà la posizione di ogni carattere
    //controllo che i parametri siano pari
    if(n%2!=0)
    {
        printf("Errore:numero di parametri non pari\n");
        exit(2);
    }

    //controllo sui file
    for (i = 0; i < n; i++)
    {
        if((fd=open(argv[i+1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(3);
        }
        close(fd);
    }
    
    //allocazione della pipe padre figlio
    piped=malloc(n*sizeof(pipe_t));

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
               close(piped[j][0]);
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
            pos=0;
            d.c1=0;
            d.c2=0;
            
            //codice dei figli pari
            if(i%2==0)
            {
                while(read(fd,&Ch,1))
                {
                    if(pos%2==0)
                    {
                        if(Ch%2==0)
                        {
                            ++d.c1;
                        }
                        else
                        {
                            ++d.c2;
                        }
                    }
                    ++pos;
                }
                

            }
            else
            {

                while(read(fd,&Ch,1))
                {
                    if(pos%2!=0)
                    {
                        if(Ch%2!=0)
                        {
                            ++d.c1;
                        }
                        else
                        {
                            ++d.c2;
                        }
                    }
                    ++pos;
                }
            }
            write(piped[i][1],&d,sizeof(struct dati));
            if(d.c1>d.c2)
            {
                exit(0);
            }
            else
            {
                 exit(1);
            }
           
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (i = 0; i < n; i++)
    {
        close(piped[i][1]);

    }
    //recupero delle informazioni dei figli pari
    for (i = 0; i < n; i+=2 )
    {
        
        read(piped[i][0],&d,sizeof(struct dati));
        printf("il figlio di indice %d ha ritornato le seguenti informazioni:\ncaratteri ascii pari:%ld\ncaratteri ascii dispari:%ld\n",i,d.c1,d.c2);

    }
    //recupero delle informazioni dei figli dispari
    for (i = 1; i < n; i+=2 )
    {
        
        read(piped[i][0],&d,sizeof(struct dati));
        printf("il figlio di indice %d ha ritornato le seguenti informazioni:\ncaratteri ascii pari:%ld\ncaratteri ascii dispari:%ld\n",i,d.c1,d.c2);

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

    
    

    

