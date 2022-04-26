
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
    if(argc < 4)
    {
       printf("Errore numero di parametri insufficienti\n");
       exit(1);
    }
    //variabili globali
    int B=atoi(argv[2]); //numero di processi figli
    int fd,fd2; //file descriptor
    int q; //indice dei processi figli
    int pid; //variabile che contiene il fid del figlio
    //int pid[]; //array di pid dei figli
    pipe_t *pipefigliopadre;//pipe per la comunicazione figlio-padre
    char buff[255]; //buffer per memorizzare i blocchi letti dai figli
    int L=atoi(argv[3]); //dimensioni in caratteri del file 
    int ritorno,pidf,status; //variabili per recuperare il valore di ritorno del figlio
    

    //controllo sul file
        if((fd=open(argv[1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file %s\n",argv[1]);
            exit(2);
        }
        close(fd);
    //controllo che B sia strettamente positivo
    if(B<=0)
    {
        printf("Errore: %d non strettamente positivo\n",B);
        exit(3);
    }
    //controllo che L sia strettamente positivo
     if(L<=0)
    {
        printf("Errore: %d non strettamente positivo\n",L);
        exit(4);
    }
    //allocazione della pipe padre figlio
    pipefigliopadre=malloc(B*sizeof(pipe_t));

    //controllo se l'allocazione è andata a buon fine
    if(pipefigliopadre==NULL)
    {
        printf("Errore nella creazione della pipe\n");
        exit(3);
    }
    //creazione delle pipe
    for ( q = 0; q < B; q++)
    {
        if(pipe(pipefigliopadre[q])<0)
        {
            printf("Errore nella creazione della pipe padre figlio\n");
            exit(4);
        }
        

    }
    //creazione del file
    //char str[255];
    dim=strlen(argv[1]);
    char *str=malloc((dim+8)*sizeof(char));
    if( str==NULL)
    {
        printf("Errore nella allocazione del nome del file da creare\n");
        exit(5);
    }
    sprintf(str,"%s.Chiara",argv[1]);
    if((fd2=(open(str,O_CREAT|O_WRONLY|O_TRUNC,PERM)))<0)
    {
        printf("Errore nella creazione del file %s\n",str);
        exit(5);
    }
    printf("Sono il proc padre con pid=%d e sto per creare %d processi figli\n",getpid(),B);

    //creazione dei figli
    for (int q = 0; q < B; q++)
    {
        if((pid=fork())<0)
        {
           printf("Errore nella fork\n");
            exit(6);
        }
        //codice figlio
        if(pid==0)
        {
            printf("Sono il figlio di indice %d con pid=%d\n",q,getpid());
            
            //chisura dei lati della pipe non utilizzati dal figlio
            for (int  j = 0; j < B; j++)
            {
                close(pipefigliopadre[j][0]); //se è scrittore
               
                if(j!=q)
                {
                    close(pipefigliopadre[j][1]);
                    
                }
                
            }
            //il figlio apre il file
        if((fd=open(argv[1],O_RDONLY))<0)
        {
            printf("Errore nell'apertura dei file da parte del figlio di indice %d\n",q);
            exit(-1);
        }
        int Il=(q*L)/B; //il figlio calcola l'inizio della lettura 
        int Fl=((q+1)*L)/(B-1); //il figlio calcola la fine di lettura
        int dim=Fl-Il; //il figlio calcola quanto deve leggere
       //il figlio legge un blocco di byte
        read(fd,&buff,dim*sizeof(char));
        //il figlio scrive l'ultimo carattere del blocco letto al padre
        write(pipefigliopadre[q][1],&buff[dim-1],1);
        //ritorna la dimensione del blocco letto
        exit(dim); 
        }
                
    }
    //chiusura lati delle pipe inutilizzate dal padre
    for (int  q= 0; q < B; q++)
    {
        close(pipefigliopadre[q][1]);
    }
    char c;
    //recupero delle informazioni 
    for (int  q= 0; q < B; q++)
    {
        //il padre legge l'ultimo carattere del blocco letto dai figli
        read(pipefigliopadre[q][0],&c,1);
        //il padre lo scrive sul file creato
        //printf("il padre ha letto:%c\n",c);
        write(fd2,&c,1);

    }
    
    //il padre aspetta i figli
   for (int q = 0; q < B; q++)
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
             printf("il  figlio di indice %d con pid=%d ha ritornato il valore: %d(255 se ha avuto problemi)\n",q,pidf,ritorno);
         }       
    }

exit(0);
}
    
    

    

