#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

typedef int pipe_t[2];
bool is_done(bool *v,int n)
{

    for ( int i = 0; i < n; i++)
    {
       if(v[i]==0)
       {
           return 0;
       }  
    }
    return 1;
    
}

int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 3)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
    //variabili globali
    int n=argc-1; //numero di processi da creare
    int fd; //file descriptor
    int i; //indice per i proc figli
    int pid; //variabile che contiene il fid del figlio
    pipe_t *padrefiglio;//pipe per la comunicazione padre-figlio
    pipe_t *figliopadre;//pipe per la comunicazione figlio-padre
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    char buff[250]; //buffer per la lettura delle linee
    int j; //indice per scorrere il buffer
    char token;
    int max; //conterrà il valore massimo
    int scritture; //indica il numero di scritture eseguite dai figli
    bool *finito;
    int nr;
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
    padrefiglio=malloc(n*sizeof(pipe_t));
    figliopadre=malloc(n*sizeof(pipe_t));
    finito=malloc(n*sizeof(bool));
    for (int  i = 0; i < n; i++)
    {
       finito[i]=0;
    }
    
    //controllo se l'allocazione è andata a buon fine
    if(padrefiglio==NULL || figliopadre==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(3);
    }
    //creazione delle pipe
    for (i = 0; i < n; i++)
    {
        if(pipe(padrefiglio[i])<0)
        {
            printf("Errore nella creazione della pipe padre figlio\n");
            exit(4);
        }
        if(pipe(figliopadre[i])<0)
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
               if(i!=j)
               {
                   close(figliopadre[j][1]);
               }

            }
            //il figlio apre il file
            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(-1);
            }
            //lettura del file
            scritture=0;
            j=0;
            while(read(fd,&buff[j],1))
            {

                    if(buff[j]=='\n')
                    {
                        ++j;
                        buff[j-1]=0;
                        //printf("%s\n",buff);
                        write(figliopadre[i][1],&buff[0],1);
                        read(padrefiglio[i][0],&token,1);
                        if(token=='S')
                        {
                            ++scritture;
                            printf("Sono il processo di indice %d e pid=%d ho trovato il carattere massimo %c fra tutti gli altri, ecco la linea: %s\n",i,getpid(),buff[0],buff);
                        }
                        j=0;
                    }
                    else
                    {
                        ++j;
                    }

            }
          exit(scritture); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (int i = 0; i < n; i++)
    {
        close(padrefiglio[i][0]);
        close(figliopadre[i][1]);

    }
    //recupero delle informazioni 
    
    int pos;
    char ch;
    while(is_done(finito,n)!=1)
    {  
        max=-1;
        pos=-1;
        for (int i= 0; i< n; i++) 
        {

            nr=read(figliopadre[i][0],&ch,1);
           // printf("%c\n",ch);
            if(nr!=1)
            {
                finito[i]=1;
            }
            
             if(finito[i]!=1)
           {
                if(ch>max)
                {
                    max=ch;
                    pos=i;
                }
           }
        }
        //il padre manda ai figli le informazioni sulla stampa
        for (int  i = 0; i < n; i++)
        {
            if(finito[i]!=1)
                {
                if(i==pos)
                {
                    token='S';
                }
                else
                {
                    token='N';
                }         
                write(padrefiglio[i][1],&token,1);
            }

        }
    }
    
    
    //il padre aspetta i figli
   for (int i = 0; i < n; i++)
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
             printf("il  figlio con pid=%d ha ritornato il valore: %d\n",pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

