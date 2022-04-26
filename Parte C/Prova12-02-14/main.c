
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#define PERM 0644
typedef int pipe_t[2];
char nome[30]; //conterrà il nome del file da creare
int i; //indice per i processi
void finescrittura(int sig)
{
    printf("un figlio ha terminato di scrivere sul file \n");
}
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
    pipe_t nipotefiglio; //pipe per la comunicazione nipote-figlio
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    int pidn; //conterrà il pid del nipote
    
    int Fcreato; //file descriptor per il file che verrà creato
    int dim; //conterrà la lunghezza in caratteri del file;
    int half; //conterrà valore  della metà della dimensione in caratteri dei file
    int j;  //indice che conterà il numero di scritture/letture fatte sul file inverso
    char ok='x'; //token che il nipote manda al nipote
    int nr; //conterra numero di letture fatte
    char Ch; //conterrà i caratteri letti dal file

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
    
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),n);
 

    signal(SIGUSR1,finescrittura);
    //creazione dei figli
    for ( i = 0; i < n; i++)
    {
        if((pid=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(3);
        }
        //codice figlio
        if(pid==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",i,getpid());
            sprintf(nome,"inverso%d",i);
            //il figlio crea il file inverso
            if((Fcreato=open(nome,O_CREAT|O_TRUNC|O_WRONLY,PERM))<0)
            {
            printf("Errore nella creazione del file %s\n",nome);
            exit(-1);
            }
            //crea la pipe di comunicazione con il nipote
            if(pipe(nipotefiglio)<0)
            {
                printf("Errore nella creazione della pipe nipote figlio\n");
                exit(-1);
            }
            //apriamo il file per contiamo la dimensione del file che ci servirà per far leggere correttamente il figlio e il nipote
            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura del file %s\n",argv[i+1]);
            exit(-1);
            }
            dim=0;
            //contiamo i caratteri
            while(read(fd,&Ch,1))
            {
                ++dim;
            }
           
            //chiudiamo il file 
            close(fd);
            half=dim/2;
             // stampa di debuggin printf("%d:%d\n",i,half);
            //creazione del nipote
            if((pidn=fork())<0)
            {
                printf("Errore nella creazione del nipote di indice %d\n",i);
                exit(-1);

            }
            //codice del nipote
            if(pidn==0)
            {

                 printf("Sono il nipote del figlio di indice %d con pid=%d\n",i,getpid());
                //chiusura delle pipe non utilizzate dal nipote
                close(nipotefiglio[0]);
                //apertura del file da parte del nipote
                if((fd=open(argv[i+1],O_RDONLY))<0)
                 {
                 printf("Errore nell'apertura del file da parte del nipote %s\n",argv[i+1]);
                 exit(-2);
                 }
                //ci spostiamo alla fine del file 
                lseek(fd,-1l,SEEK_END);
                j=0;
                //serve per scrivere correttamente con i file di dimensione in caratteri dispari 
                // if(half%2==0)
                // {
                //    ++half;
                   
                // }
                //leggiamo finchè non siamo arrivati alla metà
                while(read(fd,&Ch,1))
                {
                    if(j==half)
                    {
                        break;
                    }
                    else 
                    {   //se non siamo arrivati alla metà continuiamo a scrivere sul file 
                        ++j;
                        write(Fcreato,&Ch,1);
                        //torniamo indietro di due 
                        lseek(fd,-2l,SEEK_CUR);
                    }
                }
                //mandiamo l'ok al figlio così può iniziare a leggere
                write(nipotefiglio[1],&ok,1);
                //ritorniamo il numero di caratteri scritti
                exit(j);
                

            }
            //il figlio riapre il file stavolta per leggerlo
            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura del file %s\n",argv[i+1]);
            exit(-1);
            }
            //chiudiamo il lato della pipe che non utilizziamo
            close(nipotefiglio[1]);
            //spostiamo il figlio alla posizione corretta
            lseek(fd,(half-1),SEEK_CUR);
            //controlliamo che la lettura dell'ok sia andata a buon fine
            nr=read(nipotefiglio[0],&ok,1);
            if(nr!=sizeof(char))
            {
                printf("Errore nella lettura dell'ok da parte del nipote\n");
                exit(-1);
            }
            j=0;
             while(read(fd,&Ch,1))
                {
                    //leggiamo fino a che in nostro indice j non sia uguale alla metà,ma in realtà stiamo tornando indietro fino all'inizio
                    if(j==half)
                    {
                        break;
                    }
                    else
                    {
                        //incremento i caratteri letti/scritti e scriviamo i caratteri sul file
                        ++j;
                        write(Fcreato,&Ch,1);
                        lseek(fd,-2l,SEEK_CUR);
                    }
                }
                //chiudiamo il file per sicurezza
                close(Fcreato);
                //mandiamo al padre l'informazione di aver terminato
                sleep(1);
                kill(getppid(),SIGUSR1);
                sleep(1);
            //attendiamo il nipote
            pidn=wait(&status);
              if(pidn<0)
               {
                   printf("Errore nella wait\n");
                   exit(-1);
               }
               if((status & 0xFF)!=0)
               {
                   printf("il nipote con pid %d è terminato in modo anomalo\n",pidn);
               }
               else
               {
                   ritorno=(int) ((status >>8 ) & 0xFF);
                   printf("il nipote del figlio di indice %d con pid=%d ha ritornato il valore: %d\n",i,pidn,ritorno);
               } 
               //ritorniamo i caratteri letti/scritti dal figlio
         exit(j); 
        }
                
    }
    
    // for(i=0;i<n;++i)
    // {
        
    //     pause();
       
    // }
    //recupero delle informazioni 
    for (i = 0; i < n; i++)
    {
       
     //crea il nome del file da aprire
     sprintf(nome,"inverso%d",i);
     //il padre attente di ricevere conferma dai figli di aver terminato di scrivere dai file
     pause();
     //controlla l'apertura 
     if((Fcreato=open(nome,O_RDONLY))<0)
     {
         printf("Errore nella apertura del file %s da parte del padre\n",nome);
         exit(3);
     }
     //stampa il contenuto del file inverso
      
     printf("Ecco il file %s:",nome);
     while(read(Fcreato,&Ch,1))
     {
         printf("%c",Ch);
     }
     printf("\n");
     close(Fcreato);
     
    }

    //il padre aspetta i figli
   for (i = 0; i < n; i++)
    {
        pidf=wait(&status);
        if(pidf<0)
         {
             printf("Errore nella wait\n");
             exit(4);
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

    
    

    

