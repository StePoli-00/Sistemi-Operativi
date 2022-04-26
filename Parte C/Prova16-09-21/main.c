
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
 
    int K=argc-1; //numero di processi figli da creare
    int fd; //file descriptor
    int createdf;
    int pid; //variabile che contiene il fid del figlio
    //int *pid; //array di pid dei figli
    int k; //indice per i processi
    pipe_t *piped; //pipe per la comunicazione a ring: ogni figlio legge dalla pipe k e scrive su pipe (k+1)%K 
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    char buffer[250]; //buffer dove andranno le linee lette dai figli
    char*TUTTE[250];
    int j; //indice per scorrere buffer
   

    //creazione del file 
    if((createdf=open("StefanoPolitanò",O_CREAT|O_TRUNC,PERM))<0)
    {
        printf("Errore nella creazione del file\n");
        exit(1);
    }
    //controllo sui file
    for (k = 0; k < K; k++)
    {
        if((fd=open(argv[k+1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[k+1]);
            exit(2);
        }
        close(fd);
    }
    
    //allocazione della pipe padre figlio
    piped=malloc(K*sizeof(pipe_t));
      //allocazione dell'array TUTTE
    *TUTTE=malloc(K*sizeof(TUTTE));
    //controllo se l'allocazione è andata a buon fine
    if(piped==NULL || TUTTE==NULL )
     {
        printf("Errore nella allocazione della pipe o nella creazione dell'array TUTTE \n");
        exit(3);
    }
//     for(k=0;k<K;++k)
//    {
//        TUTTE[k]="ok";
//    }
  
    //creazione delle pipe
    for (k = 0; k < K; k++)
    {
        if(pipe(piped[k])<0)
        {
            printf("Errore nella creazione della pipe padre figlio\n");
            exit(4);
        }
       

    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),K);

    //creazione dei figli
    for ( k = 0; k < K; k++)
    {
        if((pid=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(5);
        }
        //codice figlio
        if(pid==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",k,getpid());
            
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int  j = 0; j < K; j++)
            {
                if(j!=k)
               {
                   close(piped[j][0]);
               }
                if(j!=((k+1)%K))
                {
                    close(piped[j][1]);
                }

            }

            if((fd=open(argv[k+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura del file %s\n",argv[k+1]);
            exit(-1);
            }
            j=0;
        
            while(read(fd,&buffer[j],1))
            {
                if(buffer[j]=='\n')
                {
                    buffer[j]=0;
                    ++j;
                    
                    
                    if(k!=0)
                    {
                    read(piped[k][0],TUTTE,sizeof(TUTTE));
                    }
                    TUTTE[k]=buffer;
                    if(k==K-1)
                    {
                        write(createdf,TUTTE,sizeof(TUTTE));
                    }
                    write(piped[(k+1)%K][1],TUTTE,sizeof(TUTTE));
                    for(k=0;k<K;++k)
                    {
                      printf("%d:%s\n",k,TUTTE[k]);
                    } 
                    ritorno=j;
                    j=0;


                    
                }
                else
                {
                    ++j;
                }

            }
          exit(ritorno); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (k = 0; k < K; k++)
    {
        if(k!=0)
        {
            close(piped[k][0]);
            close(piped[k][1]);
        }

    }
   //il padre innesca la comunicazione a ring
   for(k=0;k<K;++k)
   {
       TUTTE[k]="ok";
   }
   write(piped[0][1],TUTTE,K*sizeof(TUTTE));
    close(piped[0][1]);
    //il padre aspetta i figli
   for (k = 0; k < K; k++)
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

    
    

    

