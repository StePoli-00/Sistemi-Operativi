#!/bin/sh
#controllo numero di parametri
if test $# -ne 2 
then echo "numero di parametri errati";exit 0
fi
#contollo sul primo parametro che venga passato in forma assoluta
case $1 in
	/*);;
	*) echo "$! non in forma assoluta"; exit 1 
esac
#controllo sul secondo parametro che sia strettamente positivo
if test  $2 -lt 1
then echo "$2 non strettamente positivo"; exit 2
fi

#setto la variabile path
PATH=`pwd`:$PATH
export PATH
>/tmp/temp1$$
#chiamata ricorsiva
FCR.sh $1 $2 /tmp/temp1$$


for i in `cat /tmp/temp1$$`
do
echo "digita un numero maggiore di 1 e compreso tra $2"
read var
while test  $var -lt 1 -o $var -gt $2 -o -z $var
do 
	
	echo "errore $var non compreso tra 1 e $2, rinserire"
	read var
	
done
echo "passo i parametri alla parte c: $i $var"
done
rm /tmp/temp1$$
