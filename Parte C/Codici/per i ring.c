#include <stdlib.h>
#include <unistd.h>
typedef int pipe_t[2];


int main(int argc,char **argv)
{
    char ok='x';
    int i;//indice per i processi
    int n=argc-1; //numero di processi da creare
    pipe_t *piped; //comunicazione a ring: i figli leggono dalla pipe i e scrivono su pipe di (i+1)%n
    piped=malloc(n*sizeof(pipe_t));
    //questo nel figlio
    for (int j = 0; j< n; j++)
    {
         if(j!=i)
      {
       close(piped[j][0]);
      }
      if(j!=((i+1)%n))
      {
          
          close(piped[j][1]);
      }
    }
    //nel padre fa le chiusure delle pipe eccetto la prima pipe lato lettura per innescare la comunicazione a ring 
    //e per non ricevere sigpipe quando l'ultimo figlio scriverà su pipe[0][0]
     for (i=1;i<n;++i)
    {
       
        close(piped[i][0]);
        close(piped[i][1]);

    }
    //il padre innesca la comunicazione a ring
    write(piped[0][1],&ok,1);
    //chiude adesso il lato di scrittura che non serve più
    close(piped[0][1]);



}

/* per i segnali fai la sleep poi la kill se un processo deve inviare qualcosa
//l'altro fa una pause e niente sleep