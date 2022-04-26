#!/bin/sh
#controllo sul numero di parametri
if test $# -ne 1
then echo "errore, troppi parametri passati";exit 1
fi
case $1 in
	/*) if test ! -d $1 -o ! -x $1
	then echo "$1 directory inesistente o non attraversabile"; exit 2
	fi;;
	*) echo " $1 non in forma assoluta"; exit 3;;
esac
PATH=`pwd`:$PATH
export PATH
>/tmp/filetmp
conta=0
echo $conta > /tmp/filetmp
echo "inizia la fase A"
	FCR.sh $1  $conta  /tmp/filetmp
	read tot < /tmp/filetmp
echo "il numero di livelli totali è $tot"
echo "fine fase A"
	
	
