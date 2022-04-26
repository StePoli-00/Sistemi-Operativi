#!/bin/sh

if test $#  -ne 3 
then  "echo numero di parametri errato"; exit 1
fi

#controllo sul primo parametro
case $1 in
/*) if test ! -d $1 -o ! -x $1
then echo "$1 non  attraversabile o non esistente"; exit 2
fi;;
*) echo "$1 non in forma assoluta";exit 3;;
esac
#controllo sul secondo parametro
case $2 in
	#anche se non richiesto controllo che non sia stato passato un percorso di un file o di una directory
	*/*) echo "$2 non è una stringa";exit 4;;
esac
#controllo sul terzo parametro
expr $3 + 0 > /dev/null 2>&1 
ris=$?
if test $ris -eq 2 -o $ris -eq 3 #controllo che $3 sia un numero
then echo "$3 non è un numero"; exit 5
fi
if  test $3 -lt 1 #controllo che $3 sia strettamente positivo
then echo "$3 non è strettamente positivo"; exit 6
fi

#setto la variabile path
PATH=`pwd`:$PATH
export PATH

>/tmp/nomedir #creo un file temporaneo che conterrà il nome delle directory trovate
echo "inizio FASE A"
fase=A #servirà per distinguere le fasi e le operazioni da eseguire con il file ricorsivo
FCR.sh $1 $2 /tmp/nomedir $fase #chiamata ricorsiva
dim=`wc -l </tmp/nomedir`
echo "ho trovato $dim  directory che corrispondono alle specifiche:"
more /tmp/nomedir 
echo "fine FASE A"

if test $dim -gt $3
then
echo "inizio FASE B"
echo "digita un numero compreso tra 1 e $3"
read ans
if test $ans -ge 1 -a $ans -le $3 #se le dir trovate sono maggiori di $3
then
	fase=B
	count=1
	dir= #conterrà la dir  in posizione scelta dall'utente
	for i in `cat /tmp/nomedir` #cerco la directory in posizione scelta dall'utente
	do
	if test $count -eq $ans
	then 
		dir=$i
	fi
	count=` expr $count + 1`
	done
	#invoco il file ricorsivo
	FCR.sh $dir $2 /tmp/nomedir $fase 
fi
fi
rm /tmp/nomedir
echo "fine FASE B"

