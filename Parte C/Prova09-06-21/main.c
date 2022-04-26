#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#define PERM 0644
typedef int pipe_t[2];

int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 3)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
    int Z=argc-1; //numero di processi figli da creare
    int fd; //file descriptor
    int pid; //variabile che contiene il fid del figlio
    int z; //indice per i processi
    pipe_t *pf;//pipe per la comunicazione padre-figlio
    pipe_t padrefiglios; //pipe per la comunicazione con il figlio speciale
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    int createdF; //file descriptor del file creato
    int pids;//pid del figlio speciale
    char line[210]; //conterrà le linee lette dai processi figli
    char numero[20]; //conterà il numero di linee ritornato dal figlio speciale
    char nome[80]="/tmp/StefanoPolitanò";
    int dim; //conterrà il valore intero del numero di linee ritornate dalla exec
    int dimlinea; //conterrà la lunghezza in caratteri delle linee letti dalla pipe 
    int j=0; //indice per scorrere line e che conterà la dimensione in caratteri
    int ret; //variabile per il ritorno del figlio

    //creazione del file 
    if((createdF=open(nome,O_CREAT|O_WRONLY|O_TRUNC,PERM))<0)
    {
        printf("Errore nella creazione del file\n");
        exit(1);
    }

    //controllo sui file
    for (z = 0; z < Z; z++)
    {
        if((fd=open(argv[z+1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[z+1]);
            exit(3);
        }
        close(fd);
    }
    //creazione della pipe 
    if((pipe(padrefiglios))<0)
    {
        printf("Errore nella creazione della pipe per il figlio speciale\n");
        exit(4);
    }
    //creazione del figlio speciale
    if((pids=fork())<0)
    {
        printf("Errore nella creazione del figlio speciale\n");
        exit(5);
    }
    //codice del figlio speciale
    if(pids==0)
    {
        
        printf("sono il figlio speciale con pid=%d e conterò la lunghezza del primo file\n",getpid());
         
        if((fd=open(argv[1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura del primo file da parte del figlio speciale\n");
            exit(-1);
        }
         
         
        close(padrefiglios[0]);
        //chiudo lo stdinput
        close(0);
        dup(fd);
        close(padrefiglios[0]);
        //chiudo il lato di lettura
        //chiudo lo stdout
        close(1);
        //duplico la pipe lato scrittura
        dup(padrefiglios[1]);
        close(padrefiglios[1]);
        execlp("wc","wc","-l",(char *)0);
        perror("errore nella exec del figlio speciale\n");
        exit(-2);
        
    }
    j=0;
    //chiudiamo il lato di lettura
    close(padrefiglios[1]);
    //leggo il numero scritto sulla pipe figlio speciale
    while(read(padrefiglios[0],&numero[j],1))
    {
        ++j;
    }
    //chiudiamo la pipe 
    close(padrefiglios[0]);
    //converto il numero di linee in int 
    dim=atoi(numero);
    //printf("%d\n",dim);
   //il padre attende il figlio speciale 
    pids=wait(&status);
    if(pids<0)
     {
         printf("Errore nella wait del figlio speciale\n");
         exit(6);
     }
     if((status & 0xFF)!=0)
     {
         printf("il figlio con pid %d è terminato in modo anomalo\n",pids);
         exit(7);
     }
    //allocazione della pipe padre figlio
    pf=malloc(Z*sizeof(pipe_t));

    //controllo se l'allocazione è andata a buon fine
    if(pf==NULL )
    {
        printf("Errore nella allocazione della pipe\n");
        exit(8);
    }
    //creazione delle pipe
    for (z = 0; z < Z; z++)
    {
        if(pipe(pf[z])<0)
        {
            printf("Errore nella creazione della pipe padre figlio\n");
            exit(9);
        }
       
    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),Z);

    //creazione dei figli
    for ( z = 0; z < Z; z++)
    {
        if((pid=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(10);
        }
        //codice figlio
        if(pid==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",z,getpid());
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int  j = 0; j < Z; j++)
            {
              close(pf[j][0]);
                if(j!=z)
                {
                    close(pf[j][1]);
                }

            }
            //apertura del file da parte del figlio
            if((fd=open(argv[z+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura del file %s\n",argv[z+1]);
            exit(-1);
            }
            j=0;
            //lettura del file da parte del figlio
            while(read(fd,&line[j],1))
            {

                //se abbiamo raggiunto l'acapo
                if(line[j]=='\n')
                {
                    //incrementiamo la dimensione della linea 
                    ++j;
                    //scriviamo per primo la dimensione in caratteri  della linea
                    write(pf[z][1],&j,sizeof(int));
                    //scriviamo la linea
                    write(pf[z][1],&line,j);
                    //qua 
                    ret=j;
                    j=0;
                }
                else
                {
                    ++j;
                }

            }
            //qua
            //j=strlen(line);
            
            //ritorniamo la lunghezza dell'ultima linea letta
          exit(ret); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for ( z = 0; z < Z; z++)
    {
        close(pf[z][1]);
    
    }
    
    //recupero delle informazioni 
    for (int j = 0; j < dim; j++)
    {
     
        for(z=0;z<Z;++z)
        {
            //il padre legge la dimensione della linea
            read(pf[z][0],&dimlinea,sizeof(int));
            //legge la linea 
            read(pf[z][0],line,dimlinea);
            //scrive sul file la linea mandata
            write(createdF,line,dimlinea);
        }

    }

    //il padre aspetta i figli
   for (z = 0; z < Z; z++)
    {
    
        pidf=wait(&status);
        if(pidf<0)
         {
             printf("Errore nella wait\n");
             exit(11);
         }
         if((status & 0xFF)!=0)
         {
             printf("il figlio con pid %d è terminato in modo anomalo\n",pidf);
         }
         else
         {
             ritorno=(int) ((status >>8 ) & 0xFF);
             printf("il figlio con pid=%d ha ritornato il valore: %d(se 255 errori)\n",pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

