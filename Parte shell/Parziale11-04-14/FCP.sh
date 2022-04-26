#!/bin/sh
#controllo sul numero di parametri
if test $# -lt 2
then echo "numero parametri passati insufficienti";exit 1 
fi
#controllo sulle gerarchie 
for i in $* 
do
	case $i in   
		/*) if test ! -d $i -o ! -x $i
		then echo "directory non travesabile o non esistente"
		exit 2
		fi;;
		
		*) echo "$i non passata in forma assoluta"; exit 3;;
	esac

done
#setto la variabile path
PATH=`pwd`:$PATH
export PATH
c=1
for i in $*
do      >/tmp/tmp$$-$c
	FCR.sh $i /tmp/tmp$$-$c
    	echo "fase $c"
	echo "ho trovato in $i: `wc -l </tmp/tmp$$-$c` file corrispondenti alle specifiche"	
	for j in `cat /tmp/tmp$$-$c`
	do
		echo $j
		echo "digitare un numero X"
		read ans 
		case $ans in
			*[!0-9]*) echo "$ans non è un numero positivo";;

		
			*) head -$ans < $j;;
		esac
	

	done
	rm /tmp/tmp$$-$c
	c=`expr $c + 1`
done

