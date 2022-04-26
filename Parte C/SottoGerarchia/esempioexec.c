#include <stdio.h>
#include <unistd.h>
int main(int argc, char **argv)
{
 char *v[4];

 v[0]="ls";
 v[1]="-li";
 v[2]="Lab";
 v[3]=(char *)0;
 //execv("/bin/ls",v);
execl("/usr/bin/cat","cat","f1",(char *)0);
 



}
