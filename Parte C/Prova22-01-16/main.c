
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>

typedef int pipe_t[2];
struct dati
{
    char chr; //contiene il carattere AM
    long int occ; //contiene il massimo di occorrenze tra figlio e nipote
    char proc; //contiene F per figlio o N nipote 
    int pid; //pid processo che ha calcolato occ
};
int main (int argc, char **argv)
{
    //controllo sui parametri
    if(argc < 5)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
 
    int m=argc-1; //numero di file
    int n=m/2; //numero di processi figli
    int fd; //file descriptor
    int pid; //variabile che contiene il fid del figlio
    int i; //indice per i processi
    pipe_t *figliopadre;//pipe per la comunicazione padre-figlio
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    int ritornon; //valore di ritorno del nipote
    char AM; //carattere trovato dal figlio
    long int focc; //occorrenze trovate dal figli
    long int nocc; //occorrenze trovate dal nipote
    struct dati d; //struct 
    int pidn; //conterrà il pid del nipote
    int nw; //num di scritture effettuate dalle pipe
    char Ch; //carattere utilizzato per la lettura da file
    //controllo se i parametri sono pari
    if(m%2!=0)
    {
        printf("Errore numero di parametri non pari\n");
        exit(2);
    }

    //controllo sui file
    for (i = 0; i < m; i++)
    {
        if((fd=open(argv[i+1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(3);
        }
        close(fd);
    }
    
    //allocazione della pipe padre figlio
    figliopadre=malloc(n*sizeof(pipe_t));
    //controllo se l'allocazione è andata a buon fine
    if(figliopadre ==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(4);
    }
    //creazione delle pipe
    for (i = 0; i < n; i++)
    {
        if(pipe(figliopadre[i])<0)
        {
            printf("Errore nella creazione della pipe padre figlio\n");
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
                if(j!=i)
                {
                    close(figliopadre[j][1]);
                }

            }
            pipe_t nipotefiglio;
            pipe_t figlionipote;
            //creazione delle pipe per la comuicazione con il nipote
            if(pipe(figlionipote)<0 || pipe(nipotefiglio)<0)
            {
                printf("Errore nella creazione delle pipe nipote-figlio e figlio-nipote\n");
                exit(-1);
            }
            if((pidn=fork())<0)
            {
                printf("Errore nella creazione del nipote\n");
                exit(-1);
            }
            //codice nipote
            if(pidn==0)
            {
                printf("Sono il nipote di indice %d con pid=%d\n",i,getpid()); 
                //chiusura dei lati della pipe non utilizzati dal nipote
                close(figliopadre[i][1]);
                close(figlionipote[1]);
                close(nipotefiglio[0]);
                //il nipote legge il carattere AM
                read(figlionipote[0],&AM,1);
                //apertura del file dal nipote
                if((fd=open(argv[i+1+n],O_RDONLY))<0)
                {
                printf("Errore nell'apertura dei file %s\n",argv[i+1+n]);
                exit(-2);
                }
                nocc=0;

                while((ritornon=read(fd,&Ch,1))!=0)
                {
                    //se il carattere è uguale al carattere mandato dal figlio
                    if(Ch==AM)
                    {
                        ++nocc;
                    }
                }
                //comunicazione delle occorrenze calcolate dal nipote
                write(nipotefiglio[1],&nocc,sizeof(long int ));
                exit(ritornon);

            }
            //chiusure dei lati delle pipe non utilizzati dal figlio
            close(figlionipote[0]);
            close(nipotefiglio[1]);
            //apertura del file da parte del figlio
            if((fd=open(argv[i+1],O_RDONLY))<0)
            {
            printf("Errore nell'apertura dei file %s\n",argv[i+1]);
            exit(-1);
            }
            //lettura ricercando il primo carattere maiuscolo
            while(read(fd,&Ch,1))
            {
                if(isupper(Ch)) //se lo troviamo usciamo 
                {
                    AM=Ch;
                    write(figlionipote[1],&AM,1);
                    break;
                }
            }
            //incremento le occorrenze di quel carattere
            //torno indietro perchè il file descriptor è andato avanti di una posizione
            //lseek(fd,-1l,SEEK_CUR); se volessimo usarla allora dobbiamo mettere focc=0 altrimenti ne conta uno in più
            focc=1;
            
            //ricontinuo a leggere per trovare il carattere AM
            while((ritorno=read(fd,&Ch,1))!=0)
            {
                if(Ch==AM) //incremento Ch se trovo che è uguale ad AM
                {
                    ++focc;

                }
            }
            //leggo dal nipote il suo numero di occorrenze
            read(nipotefiglio[0],&nocc,sizeof(long int ));
            d.chr=AM;
            //confronto se il figlio se ha calcolate di più del nipote 
            if(focc>nocc)
            {
                d.occ=focc;
                d.proc='F';
                d.pid=getpid();

            }
            else
            {
                d.occ=nocc;
                d.proc='N';
                d.pid=pidn;
            }
            //scrivo la struct al padre
            nw=write(figliopadre[i][1],&d,sizeof(struct  dati));
            //controllo l'invio della struct
            if(nw!=sizeof(struct dati))
            {
                printf("Errore nella scrittura della struct da figlio a padre\n");
                exit(-1);
            }
            //il figlio attende il nipote
        
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
             ritornon=(int) ((status >>8 ) & 0xFF);
             printf("il nipote con pid=%d ha ritornato il valore:%d(se 255 errori nel nipote)\n",pidn,ritornon);
         }  
          exit(ritorno); 
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
     
        read(figliopadre[i][0],&d,sizeof(struct dati));
        //stampa le informazioni relative al figlio altrimenti quelle del nipote
        if(d.proc=='F')
        {
            printf("il figlio di indice %d associato al file %s ha ritornato le seguenti informazioni:\nchr=%c\nocc=%ld\nproc=%c\npid=%d\n",i,argv[i+1],d.chr,d.occ,d.proc,d.pid);
        }
        else
        {
             printf("il figlio di nipote %d associato al file %s ha ritornato le seguenti informazioni:\nchr=%c\nocc=%ld\nproc=%c\npid=%d\n",i,argv[i+1+n],d.chr,d.occ,d.proc,d.pid);
        }

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
             ritorno=(int) ((status >>8 ) & 0xFF);
             printf("il figlio con pid=%d ha ritornato il valore:%d(se 255 errore nel figlio)\n",pidf,ritorno);
         }       
    }

exit(0);
}