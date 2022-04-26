#include <unistd.h>

typedef int pipe_t[2];
//comunicazione pipeline
//i figli eccetto il primo legge da pipe[i-1] e scrivono su pipe[i]

int main(int argc,char **argv)
{
    int i=0; //indice dei figli
    int n=argc-1;
    pipe_t *piped;
     //.....//
     //dentro al figlio
     for (int j = 0; j < n; j++)
     {
         if(i==0 || j!=i-1)
         {
             close(piped[j][0]);
         }
         if( i!=j)
         {
             close(piped[j][1]);
         }
     }

     //nel padre
     for ( i = 0; i < n; i++)
     {
        close (piped[i][1]);
	    if (i != N-1)
        {
             close (piped[i][0]);
        }
     }
     // il padre legge dall'ultima pipe
     //read(piped[n-1][0],....,sizeof(..));
     
     



}