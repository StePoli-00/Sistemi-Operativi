
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
    if(argc < 3)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
 
    int M=argc-1; //numero di processi figli da creare
    int i; //indice dei proceddi
    int fd; //file descriptor
    int pid; //variabile pid dei figli
    pipe_t *figliopadre;//pipe per la comunicazione figlio-padre
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    int pidn; //pid del nipote
    char Ch;//buffer dove verrà contenuta la linea mandata dal nipote
    int dim; //lunghezza della linea

    //controllo sui file
    for (i = 0; i < M; i++)
    {
        if((fd=open(argv[i+1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(2);
        }
        close(fd);
    }
    
    //allocazione della pipe padre figlio
    figliopadre=malloc(M*sizeof(pipe_t));
    //controllo se l'allocazione è andata a buon fine
    if (figliopadre==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(3);
    }
    //creazione delle pipe
    for (int i = 0; i < M; i++)
    {
       
        if(pipe(figliopadre[i])<0)
        {
            printf("Errore nella creazione della pipe figlio padre\n");
            exit(4);
        }

    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),M);

    //creazione dei figli
    for (i = 0; i < M; i++)
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
            for (int  j = 0; j < M; j++)
            {
                close(figliopadre[j][0]);
                if(i!=j)
                {
                   close(figliopadre[j][1]);
                }

            }
            //creo una sola pipe di comunicazione tra figlio e nipote
             pipe_t nipotefiglio;
             if(pipe(nipotefiglio)<0)
             {
                 printf("Errore nella creazione della pipe nipote figlio\n");
                 exit(-1);
             }
             //creazione del nipote
             if((pidn=fork())<0)
             {
                 printf("errore nella fork da parte del figlio\n");
                 exit(-2);
             }
             //codice del nipote
             if(pidn==0)
             {
                printf("sono il nipote del figlio di indice %d con pid=%d\n",i,getpid());
                //il nipote chiude le pipe non utilizzate e lo stdout per copiare nipotefiglio[1] e fare la tail con la exec
                close(figliopadre[i][1]);
                close(nipotefiglio[0]);
                close(1);
                dup(nipotefiglio[1]);
                close(nipotefiglio[1]);
                close(nipotefiglio[0]);
                execlp("tail","tail","-1",argv[i+1],(char *)0);
                perror("Errore della exec  da parte del nipote\n");
                exit(-3);
             }
             close(nipotefiglio[1]);
            //leggo la linea dalla pipe 
            dim=0;
             while(read(nipotefiglio[0],&Ch,1))
             {
                 //printf("%c\n",Ch);
                 if(Ch=='\n')
                 {
                     break;
                 }
                 else
                 {
                     dim++;
                 }
             }
             
            write(figliopadre[i][1],&dim,sizeof(int));
        //il figlio aspetta il nipote
        ritorno=-1;
        pid=wait(&status);
        if(pid<0)
         {
             printf("Errore nella wait\n");
             exit(-4);
         }
         if((status & 0xFF)!=0)
         {
             
             printf("il figlio con pid %d è terminato in modo anomalo\n",pid);
         }
         else
         {   
             ritorno=(int) ((status >>8 ) & 0xFF);
             printf("il nipote con pid=%d ha ritornato %d (0 se l'exec è andata a buon fine)\n",pid,ritorno);
         } 
        exit(ritorno); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (int i = 0; i < M; i++)
    {
      
        close(figliopadre[i][1]);

    }
    //recupero delle informazioni nell'ordine inverso 
    for (i=M-1; i>=0;--i)
    {
     read(figliopadre[i][0],&dim,sizeof(int));
     printf("il figlio di indice %d  ha ricevuto la linea dal nipote associato al file %s lunga %d caratteri\n",i,argv[i+1],dim);
     
    }

    //il padre aspetta i figli
   for (i = 0; i < M; i++)
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
             sleep(0.2);
             ritorno=(int) ((status >>8 ) & 0xFF);
             printf("il  figlio con pid=%d ha ritornato il valore:%d (valore di ritorno del nipote)\n",pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

