
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
    //variabili globali
    int n=argc-1; //numero di processi da creare
    int fd,creato; //file descriptor
    int pid; //variabile che contiene il pid del figlio
    pipe_t *piped;//pipe per la comunicazione  tra coppie di figli
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    char str[255]; //stringa che servirà per creare il nome del file da parte del figlio dispari
    char cp; //carattere che veràà utilizzato dai proc figli per la lettura da file
    int dim; //conterà i caratteri letti
    
    if(n%2!=0)
    {
        printf("i file non sono in numero pari\n");
        exit(2);
    }
    //controllo sui file
    for (int i = 0; i < n; i++)
    {
        if((fd=open(argv[i+1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(3);
        }
        close(fd);
    }
    
    //allocazione della pipe padre figlio
    piped=malloc((n/2)*sizeof(pipe_t));
    //controllo se l'allocazione è andata a buon fine
    if( piped==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(4);
    }
    //creazione delle pipe
    for (size_t i = 0; i < (n/2); i++)
    {
        
        if(pipe(piped[i])<0)
        {
            printf("Errore nella creazione della pipe per la comunicazione tra coppie di figli\n");
            exit(5);
        }

    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);

    //creazione dei figli
    for (int i = 0; i < n; i++)
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
            
            
                //codice del processo pari
                if((i%2)==0) 
                {
                    //chisura dei lati della pipe non utilizzati dal figlio
                    for (int  j = 0; j < (n/2); j++)
                    {
                        close(piped[j][0]);
                        if(j!=(i/2))
                        {
                            close(piped[j][1]);
                        }
                    }
                    //apertura del file da parte del figlio pari
                    if((fd=open(argv[i+1],O_RDONLY))<0)
                    {
                        printf("Errore nell'apertura del file %s da parte del figlio\n",argv[i+1]);
                        exit(-1);
                    }
                   
                    //il figlio legge il file
                    dim=0;
                    while(read(fd,&cp,1))
                    {
                        write(piped[i/2][1],&cp,1);
                        ++dim;
                    }
                    
                }
                else //codice del processo dispari
                {
                   //creo il nome del file 
                    sprintf(str,"%s.MAGGIORE",argv[i+1]);
                    //crea il file
                    if((creato=open(str,O_CREAT|O_WRONLY|O_TRUNC,PERM))<0)
                    {
                        printf("Errore nella creazione del file %s\n",str);
                        exit(-1);
                    }
                    for (int  j = 0; j < (n/2); j++)
                    {
                        close(piped[j][1]);
                        if(j!=((i/2)))
                        {
                            close(piped[j][0]);
                        }


                    }
                    //apertura del file da parte del figlio dispari
                    if((fd=open(argv[i+1],O_RDONLY))<0)
                    {
                        printf("Errore nell'apertura del file %s da parte del figlio\n",argv[i+1]);
                        exit(-1);
                    }
                    char cd;
                    dim=0;
                    while(read(fd,&cd,1))
                    {
                        read(piped[i/2][0],&cp,1);
                        //printf("%c\n",cp);
                        if(cd>cp)
                        {
                            
                            write(creato,&cd,1);
                        }
                        else
                        {
                            
                            write(creato,&cp,1);
                        }
                        ++dim;
                    }
                   
                }
                 exit(dim);
                
            }
    }
                
    
    //chiusura lati delle pipe inutilizzate dal padre
    for (int i = 0; i < (n/2); i++)
    {
        close(piped[i][0]);
        close(piped[i][1]);
       

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
             printf("il  figlio di indice %d con pid=%d ha ritornato il valore: %d\n",i,pidf,ritorno);
         }       
    }

exit(0);
}

    
    

    

