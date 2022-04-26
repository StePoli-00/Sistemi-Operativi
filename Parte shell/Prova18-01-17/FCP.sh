#!/bin/sh

if test $# -ne 2
then echo "errore, numero di parametri errato";exit 1
fi

#controllo sul primo parametro
case $1 in
	/*) if test ! -d $1 -o ! -x $1
	then echo "$1  direcotry non attraversabile o insesistente";exit 2
	fi;;
	*) echo "$1 non in forma assoluta"; exit 3;;
esac

#controllo sul secondo parametro
case $2 in
	*/*) echo "$2 non in forma relativa semplice"; exit 4;;
	*) if test ! -d $2 
	then echo "$2 non è una directory";exit 5
	fi;;
esac

#esporto la varibile path
PATH=`pwd`:$PATH

> /tmp/nomefile
#files= #lista che conterrà i file trovati 

FCR.sh $1 $2 /tmp/nomefile
echo "ho trovato `wc -l < /tmp/nomefile` file con le specifiche richieste"

echo "passo alla parte c i seguenti file:"
 more /tmp/nomefile

rm /tmp/nomefile
