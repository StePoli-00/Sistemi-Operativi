#!/bin/sh
echo "FCP.sh START"
primo= #contiene primo parametro
#controllo numero di parametri
if test $# -lt 3 
then echo "errore numero di parametri insufficienti";exit 1
fi
primo=$1

shift 
#controllo sulle gerarchie
for i in $*
do
	case $i in
		/*) if test ! -d $i -o ! -x $i
		then echo "$i non attraversabile o non esistente"; exit 2
		fi;;
		*) echo "$i non in forma assoluta"; exit 3;;
	esac
done

#setto variabile PATH
PATH=`pwd`:$PATH
export PATH
>/tmp/ftmp$$
for i in $*
do
	FCR.sh $i $primo /tmp/ftmp$$
done
num=`wc -l < /tmp/ftmp$$`
echo " ho trovato $num file:"
for i in `cat /tmp/ftmp$$`
do
	echo "-------------Inizio visualizzazione di $i---------------\n"
	
	echo "		prima linea:"  `head -1 $i`
	echo "		ultima linea: `tail -1 $i` \n"
	echo "-------------Fine visualizzazionee di $i----------------\n"

done
rm /tmp/ftmp$$
echo "FCP.sh END"
