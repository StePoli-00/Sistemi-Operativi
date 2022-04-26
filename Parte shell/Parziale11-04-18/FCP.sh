#!/bin/sh
echo "inizia FCP.sh"
# controllo sui parametri
if test $# -lt 3
then echo " errore pochi parametri passati"; exit 1
fi
#controllo sul primo parametro
expr $1 + 0 > /dev/null 2>&1
ris=$? #contiene il risultato della somma sopra riportata
if test $ris -eq 2 -o $ris -eq 3
then echo "$1 non è un numero"; exit 2
fi
if test $1 -lt 1
then echo "$1 non è positivo"; exit 3 
fi
primo=$1 #contiene il valore 

#esporto la variabile path
PATH=`pwd`:$PATH
export PATH
>/tmp/Files
shift #ora posso fare opportuni controlli riferiti solo alle n    gerarchie
for i in $*
do
	case $i in
	/*) if test -d $i
		then FCR.sh $primo $i /tmp/Files
		else echo "$i non è una directory"
		fi;;
	*) echo "$i non in forma assoluta";;
esac
done
for i in `cat /tmp/Files`
do
	echo "stai visualizzando il file $i:"
	more $i
	echo "fine visualizzazione del file $i"
	
done
#rm /tmp/Files		
echo "Finisce FCP.sh"
