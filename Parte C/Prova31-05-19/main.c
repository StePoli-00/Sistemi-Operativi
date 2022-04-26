//comunicazione padre figlio 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#define PERM 0644
typedef int pipe_t[2];
struct dati
{
    int c1; //conteine il pid del figlio
    char c2[250]; //array che contiene la linea del file
    int c3; //contiene la lunghezza della linea più terminatore
};
int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 4)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
    //variabili globali
    int n=argc-1; //numero di parametri
    int j;//indice che serve per scorrere c2 nella lettura da parte del figlio con la pipe
    char opzione[3];
    int fd; //file descriptor
    int pid; //variabile che contiene il fid del figlio
    pipe_t *figliopadre;//pipe per la comunicazione padre-figlio
   
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    struct dati d;
    

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
    
    //allocazione della pipe padre figlio
    figliopadre=malloc(n*sizeof(pipe_t));
   
    //controllo se l'allocazione è andata a buon fine
    if(figliopadre==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(3);
    }
    //creazione delle pipe
    for (int i = 0; i < n; i++)
    {
        if(pipe(figliopadre[i])<0)
        {
            printf("Errore nella creazione della pipe figlio padre\n");
            exit(4);
        }
        

    }
    printf("Sono il processo padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);

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
            
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int j = 0; j < n; j++)
            {
                close(figliopadre[j][0]);
               
                if(j!=i)
                { 
                    close(figliopadre[j][1]); 
                }

            }
            pipe_t nipotefiglio;
            if(pipe(nipotefiglio)<0)
            {
                printf("erorre nella creazione della pipe nipote figlio\n");
                exit(-1);
            }
            if((pid=fork())<0)
            {
                printf("Errore nella creazione del nipote del figlio di indice %d\n",i);
                exit(-1);
            }
            //codice del nipote
            if(pid==0)
            {   
                printf("Sono il processo nipote  con pid=%d del figlio di indice %d\n",getpid(),i);
               
                close(figliopadre[i][1]);
                close(1);
                dup(nipotefiglio[1]);
                close(nipotefiglio[1]);
                close(nipotefiglio[0]);
                
                //sprintf(opzione,"-%c",'f');
                execlp("sort","sort","-f",argv[i+1],(char *)0);
                printf("Errore nella exec da parte del nipote\n");
                exit(-2);
                
            }
            //chiudo il lato di scrittura della pipe da parte del figlio
            close(nipotefiglio[1]);
            int dim=0;
            j=0;
            //il figlio legge tutte le linee mandate dal figlio
            while(read(nipotefiglio[0],&d.c2[j],1))
            {

                
                
                    if(d.c2[j]=='\n')
                    {
                    
                        dim=j;
                        j=0;

                    }
                    else
                    {
                        ++j;
                    }

            }
            d.c2[dim+1]=0;
            //printf("%s\n",d.c2);
            d.c1=getpid();
            d.c3=dim;
            write(figliopadre[i][1],&d,sizeof(struct dati));
        //il figlio aspetta il nipote
        pidf=wait(&status);
        if(pidf<0)
         {
             printf("Errore nella wait\n");
             exit(-1);
         }
         if((status & 0xFF)!=0)
         {
             printf("il nipote con pid %d è terminato in modo anomalo\n",pidf);
         }
        //  else
        //  {
        //      ritorno=(int) ((status >>8 ) & 0xFF);
        //      printf("il nipote con pid=%d ha ritornato il valore:%d\n",pidf,ritorno);
        //  }  
            ++dim;
          exit(dim); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (int i = 0; i < n; i++)
    {
        close(figliopadre[i][1]);

    }
    //recupero delle informazioni 
    for (int i = 0; i < n; i++)
    {
        //il padre legge le struct mandate dai figli
        read(figliopadre[i][0],&d,sizeof(struct dati));
        //mette il terminatore alla linea
        d.c2[d.c3+1]=0;
        //stampa la struct sul terminale
        printf("il figlio di indice %d ha ritornato la seguente struct:\nc1:%d\nc2:%sc3:%d\n",i,d.c1,d.c2,d.c3);
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
             printf("il  figlio di indice %d con pid=%d ha ritornato il valore:%d(lunghezza della linea con terminatore)\n",i,pidf,ritorno);
         }       
    }

exit(0);
}
    
    

    

