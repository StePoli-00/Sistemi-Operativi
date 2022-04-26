#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

#define PERM 0644
typedef int pipe_t[2]; //tipo di elemento per le pipe contenente sia lato di lettura che di chiusura

int main(int argc, char **argv){

    int M; //variabile per il numero di figli
    int i, j; //variabili usate per i cicli
    int pidFiglio, status, ritorno; //variabili che vengono usate per il ciclo finale di controllo per l'attesa dei figli
    pipe_t *pipefp; //pipe da ALLOCARE tra padre e figlio 
    pipe_t pipefn; //pipe da ALLOCARE tra figlio e padre
    int pid; //variabile per il pid dei figli 
    int pidnipote; 
    char linea[255]; 
    int dim; 

    //controllo sui parametri
    if(argc < 3){
        printf("Errore nel passaggio dei parametri\n"); 
        exit(1); 
    }

    M = argc - 1; 
    //allocazione delle pipe
    pipefp = (pipe_t *) malloc(M * sizeof(pipe_t));

    //controllo se è avvenuta correttamente l'allocazione
    if(pipefp == NULL){
        printf("Errore nell'allocazione delle pipe\n"); 
        exit(2); 
    }

    //ciclo per la creazione delle pipe
    for(i = 0; i < M; ++i){
        if(pipe(pipefp[i]) < 0){
            printf("Errore nella creazione della pipe figlio - padre\n"); 
            exit(3); 
        }
    }

    //stampa di debugging
    printf("Sono il processo padre di pid = %d e sto per generare %d figli\n", getpid(), M);

    //ciclo per la creazione dei figli
    for(i = 0; i < M; ++i){
        //fork di creazione del figlio
        if((pid = fork()) < 0){
            printf("Errore nella creazione del figlio di indice %d\n", i);
            exit(4); 
        } 
        //codice del figlio
        if(pid == 0){
            printf("Sono il figlio di pid = %d e di indice %d e sto per generare il nipote\n", getpid(), i);

            //chisura dei lati della pipe non utilizzati dal figlio
            for (j = 0; j < M; j++)
            { 
                close(pipefp[j][0]); 
                if(j != i)
                {
                    close(pipefp[j][1]);
                }

            }

            if((pipe(pipefn)) < 0){
                printf("Errore nella creazione della pipe figlio - nipote di indice %d\n", i); 
                exit(-1); 
            }

            if((pidnipote = fork()) < 0){
                printf("Errore nella creazione del nipote di indice %d\n", i); 
                exit(-1); 
            } 
            //codice del nipote 
            if (pidnipote == 0){
                close(pipefp[i][1]); 
                close(pipefn[0]); 
                close(1); 
                dup(pipefn[1]); 
                close(pipefn[1]); 
                close(pipefn[0]); 

                execlp("tail", "tail", "-1", argv[i + 1], (char *)0); 
                printf("Errore della exec, NON DEVO TORNARE QUI\n"); 
                exit(-2); 
            }
            //codice del figlio di nuovo
            close(pipefn[1]); 
            j = 0; 
            while(read(pipefn[0], &linea[j], 1)){
                //printf("%c\n", linea[j]); 
                if(linea[j] == '\n'){
                    break; 
                }
                else {
                    j++; 
                }
            }
            j--;
            //scrivo al padre la lunghezza della linea ESCLUSO il terminatore
             //printf("%d\n", j); 
        write(pipefp[i][1], &j, sizeof(int));
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

    //codice del padre, chiusura dei lati delle pipe che non usa
    for(i = 0; i < M; ++i){
        close(pipefp[i][1]);
    }
    
    //il padre recupera le informazioni dai figli
    dim = 0; 
    for(i = M - 1; i >= 0; i--){
        read(pipefp[i][0], &dim, sizeof(int)); 
        printf("Il figlio di indice %d ha ricevuto la linea dal nipote lunga %d caratteri dal file %s\n", i, dim, argv[i + 1]); 
    }


    //il padre aspetta i figli
    for (i = 0; i < M; i++){
    
        pidFiglio = wait(&status);
        if(pidFiglio < 0){
            printf("Errore in wait\n");
            exit(5);
        }

        if((status & 0xFF) != 0) {
            printf("Il figlio con pid %d è terminato in modo anomalo\n",pidFiglio);
        }
         else {
            ritorno = (int) ((status >>8 ) & 0xFF);
            printf("Il figlio con pid = %d ha ritornato il valore: %d (pid nipote) \n", pidFiglio,ritorno);
        }       
    }

    exit(0);
}