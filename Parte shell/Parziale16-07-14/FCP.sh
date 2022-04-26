#!/bin/sh
if test $# -ne 2
then echo "errore numero di parametri non corretto"; exit 1
fi
#controllo sul primo parametro
case $1 in 
	/*) if test ! -d $1 -o ! -x $1
	then echo "$1 non è una directory o non è attraversabile"; exit 2
	fi;;
	*) echo "$1 non è in forma assoluta";;	
esac
#controllo sul secondo parametro
`expr $2 + 0`>/dev/null 2>&1
ret=$?
if test $ret -eq 2 -o $ret  -eq 3
then echo "errore $2 non è un numero"; exit 3
else
 if test $2 -lt 0 
 then echo "$2 non è strettamente maggiore di zero"; exit 4
fi
fi

#setto la variabile path
PATH=`pwd`:$PATH
export PATH


FCR.sh $1 $2  

