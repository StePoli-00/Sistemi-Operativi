#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>

#define PERM 0644 //permessi per il file creato
typedef int pipe_t[2]; //tipo di elemento per le pipe contenente sia lato di lettura che di chiusura

int main(int argc, char **argv){

    int M; //variabile per i parametri
    int m, j; //(m per i figli) j per il contatore dei cicli while
    int fd; //file descriptor che viene usato per l'apertura di file
    int pidFiglio, status, ritorno; //variabili che vengono usate per l'attesa dei figli
    pipe_t *pipefp; //pipe da ALLOCARE tra figlio e padre
    pipe_t speciale; //pipe speciale usata tra il figlio speciale e il padre 
    int pid; //variabile per il pid dei figli 
    char nomefile[50]; //array per salvare il nome del file da creare 
    int fcreato; //variabile per il file creato 
    char LINE[255]; //buffer per salvare la linea sia per padre che per figlio
    int lunghezza; //variabile per la lunghezza 
    int linee_ricevute; //variabile che conta quante linee riceve il padre
    int ret; //variabile che salva la lunghezza dell'ultima linea
    int len; //variabile per la lunghezza della linea che riceve dal figlio 
    int nr; //variabile  per controllare il valore di ritorno delle read del padre 

    //controllo sui parametri (maggiore o uguale a 2)
    if(argc < 3){
        printf("Errore nel passaggio dei parametri\n"); 
        exit(1); 
    }

    //inizializzo M
    M = argc - 1; 
    //creo una stringa con il nome del file da creare
    sprintf(nomefile, "/tmp/francescozampirollo"); 
    //creo il file con /tmp/ il mio nome e cognome 
    if((fcreato = creat(nomefile, PERM)) < 0){
        printf("Errore nella creazione del file francescozampirollo\n"); 
        exit(2); 
    }

    //creo la pipe speciale che il padre usa con il figlio speciale
    if((pipe(speciale)) < 0){
        printf("Errore nella creazione della pipe speciale\n"); 
        exit(3); 
    }
    //codice per la creazione del figlio speciale 
    if((pid = fork()) < 0){
        printf("Errore nella creazione del figlio speciale\n"); 
        exit(4); 
    }
    //inizia il codice del figlio speciale
    if(pid == 0){ 
        //chiudo lo stdin per ridirigere in input il file F1 in modo da avere SOLO il numero di linee dal risultato della wc
        close(0); 
        //apro il file F1 che verrà ridiretto in input
        if((fd = open(argv[1], O_RDONLY)) < 0){
            printf("Errore nell'apertura del file F1\n"); 
            exit(-1); 
        }

        //chiudo lo stdout in modo che posso passare l'informazione al padre tramite la pipe
        close(1); 
        //eseguo la dup di speciale[1] in modo che il padre possa recuperare l'informazione
        dup(speciale[1]); 
        //chiudo la pipe rimasta aperta
        close(speciale[1]); 
        close(speciale[0]); 

        //eseguo il wc - l
        execlp("wc", "wc", "-l", argv[1], (char *)0); 
        //NON DEVO TORNARE QUI
        perror("Errore nella exec, qui non devo tornare\n"); 
        exit(-1); 
    }

    //codice che ritorna al padre dopo figlio speciale
    //chiudo i lati che non usa
    close(speciale[1]); 
    j = 0; 
    //leggo dalla pipe il risultato dell'exec
    while(read(speciale[0], &LINE[j], 1)){
        //se sono alla fine della linea metto il terminatore
        if(LINE[j] == '\n'){
            LINE[j] = '\0'; 
        } else {
            //altrimenti incremento j
            j++; 
        }
    }

    //ricavo la lunghezza dei futuri file (uguale per tutti)
    lunghezza = atoi(LINE); 
    printf("La lunghezza in linee del primo file è %d\n", lunghezza);  
    //chiudo la pipe che stava usando il padre
    close(speciale[0]); 

    //il padre aspetta il figlio speciale
    pidFiglio = wait(&status);
    if(pidFiglio < 0){
        printf("Errore in wait\n");
        exit(5);
    }
    if((status & 0xFF) != 0) {
        printf("Il figlio con pid = %d è terminato in modo anomalo\n",pidFiglio);
    }
    else {
        //stampa non necessaria
        
    } 

    //allocazione della pipe che usa per comunicare con il figlio
    pipefp = (pipe_t *) malloc(M * sizeof(pipe_t));

    //controllo se è avvenuta correttamente l'allocazione
    if(pipefp == NULL){
        printf("Errore nell'allocazione della pipe\n"); 
        exit(6); 
    }

    //ciclo per la creazione delle pipe
    for(m = 0; m < M; ++m){
        if(pipe(pipefp[m]) < 0){
            printf("Errore nella creazione della pipe tra figlio e padre\n"); 
            exit(7); 
        }
    }

    //stampa di debugging
    printf("Sono il processo padre di pid = %d e sto per generare %d figli\n", getpid(), M);

    //ciclo per la creazione dei figli
    for(m = 0; m < M; ++m){
        //fork di creazione del figlio
        if((pid = fork()) < 0){
            printf("Errore nella creazione del figlio di indice %d\n", m);
            exit(8); 
        } 
        //codice del figlio
        if(pid == 0){
            printf("Sono il figlio di pid = %d e di indice %d\n", getpid(), m);
            //chisura dei lati della pipe non utilizzate dal figlio
            for (j = 0; j < M; j++)
            {
                close(pipefp[j][0]); 
                if(j != m)
                { 
                    close(pipefp[j][1]); 
                }

            }
            //apertura del file da parte del figlio
            if((fd = open(argv[m + 1], O_RDONLY)) < 0) {
                printf("Errore nell'apertura del file %s\n",argv[m + 1]);
                exit(-1);
            }
            //ciclo di lettura del figlio
            j = 0;
            while(read(fd, &LINE[j], 1)){
                if(LINE[j] == '\n'){
                    //incremento per posizionare il terminatore
                    ++j; 
                    //scrivo la lunghezza al padre compreso il terminatore che prenderà il posto del \n
                    write(pipefp[m][1], &j, sizeof(int));
                    //metto il terminatore
                    LINE[j] = '\0';  
                    //scrivo la linea intera
                    write(pipefp[m][1], LINE, j); 
                    ret = j; 
                    j = 0; 
                } else {
                    j++; 
                }
            }
            //il figlio ritorna la lunghezza dell'ultima linea
            exit(ret);
        }
    }

    //codice del padre, chiusura dei lati delle pipe che non usa (quella in scrittura)
    for(m = 0; m < M; ++m){
        close(pipefp[m][1]); 
    }
    
    //il padre recupera le informazioni dai figli
    linee_ricevute = 0; 
    //ciclo che deve continuare per tutte le linee dei file moltiplicate per tutti i figli
    while(linee_ricevute != (lunghezza * M)){
        for(m = 0; m < M; ++m){
            //leggo la lunghezza della linea che sta per ricevere
            read(pipefp[m][0], &len, sizeof(int)); 
            //leggo la linea ricevuta
            nr = read(pipefp[m][0], LINE, len); 
            //controllo sia giusto il numero di caratteri letti
            if(nr != len){
                printf("Errore nella lettura delle linee\n"); 
                exit(9);
            }
            //scrivo la linea sul file creato
            write(fcreato, LINE, len);
            //incremento il numero di linee che deve essere uguale alla lunghezza del ritorno della wc moltiplicata per tutti i figli
            linee_ricevute++;  
        }
    }

    //il padre aspetta i figli
    for (m = 0; m < M; m++){
    
        pidFiglio = wait(&status);
        if(pidFiglio < 0){
            printf("Errore in wait\n");
            exit(10);
        }
        if((status & 0xFF) != 0) {
            printf("Il figlio con pid = %d è terminato in modo anomalo\n",pidFiglio);
        }
         else {
            ritorno = (int) ((status >>8 ) & 0xFF);
            printf("Il figlio con pid = %d ha ritornato il valore: %d\n", pidFiglio,ritorno);
        }       
    }
    exit(0);
}