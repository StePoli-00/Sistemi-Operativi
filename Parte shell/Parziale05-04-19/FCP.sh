#!/bin/sh
#controllo sul nome dei parametri
if test $# -lt 3
	then echo "errore pochi parametri passati";exit 1 
fi

c=1 #variabile contatore
last= #conterrà l'ultimo parametro
for i in $*
do
	if test $c -eq $#
	then last=$i
	fi
	c=`expr $c + 1`
done	
#controlliamo che l'ulitmo parametro sia una stringa
expr $last + 0 >/dev/null 2>&1 

guess=$?

if test $guess -eq 0 -o $guess -eq 1 #se guess resituisce  0 o 1 sappiamo che last non è una stringa
then echo " $last non è una stringa"; exit 2
fi
#controlliamo che l'ultimo parametro sia in forma relativa semplice
case $last in
	*/*) echo " $last parametro passato non in forma relativa semplice";exit 3;;
	*);; 
esac 
#esporto la variabile d'ambiente PATH
PATH=`pwd`:$PATH
export PATH
#creo un file temporaneo
>/tmp/file$$
c=1
#controllo delle gerarchie passate
for j in $* 
do
	if test $c -ne $#
	then 
	case $j in
		/*) if test ! -d $j -o ! -x $j
		then echo " $j non è attraversabile o non esistente"; exit 4 
		fi;;
		*)echo " $j non è in forma assoluta"; exit 5 ;;
	esac
	c=`expr $c + 1`
	fi
done 
c=1
#chiamata ricorsiva escludendo  l'ultimo parametro
for i in $*
do
	if test $c -ne $# # se c non è arrviato all'ultimo parametro
	then	FCR.sh $i $last /tmp/file$$ #invochiamo il file ricorsivo
	c=`expr $c  + 1`
	fi	
done



dim=`wc -l</tmp/file$$` #conto quanti file sono stati trovati e lo riporto con un echo
echo "file trovati: $dim "

for i in `cat /tmp/file$$` #apro il file temporaneo
do 
	nomefile=`basename $i` #se volessimo visualizzare il nome relativo semplice del file
	echo "il file $i  ha `wc -c < $i` caratteri" #visualizzo il nome assoluto e il numero di carateri del file
	echo "vuoi ordinare il file $nomefile? s/n" #chiedo all'utente se vuole riordinare il file
	read answ                                  #leggo la sua risposta
	case $answ in
		s* | S* | Y* | y* ) sort -f $i;; #riordino il file in caso di risposta affermativa
		n* | N*) echo "$nomefile non è stato ordinato";; 
	esac

done
rm /tmp/file$$
