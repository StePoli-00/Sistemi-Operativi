#!/bin/sh
if test $# -ne 2
then echo "errore numero parametri non corretto"; exit 1
fi
#controllo sul primo parametro
case $1 in
	/*) if test  ! -d $1 -o ! -x $1
	then 
		echo "$1 directory non attraversabile o non essitente"; exit 2
	fi;;
	*) echo "$1 non in forma assoluta"; exit 3;;
esac
#controllo sul secondo parametro

#anche se non richiesto controllo che il secondo parametro non abbia uno slash

case $2 in
	*/*) echo "errore $2 non è una stringa";exit 4;;
	*);;
esac

#esporto la variabile PATH porca madonna
PATH=`pwd`:$PATH
export PATH
>/tmp/filetmp

FCR.sh $1 $2 /tmp/filetmp


echo "ho trovato `wc -l < /tmp/filetmp` directory che soddisfano le specifiche"
for j in `cat /tmp/filetmp`
do
	echo $j
	echo "vuoi invocare la parte in c?"
	read ans
	case $ans in
		y* | Y* |S* |s*) echo  " invoco lo script passando $2.1 $2.2 \n";;
		n* |N*) echo "niente invocazione"
	esac

	

done
rm /tmp/filetmp


