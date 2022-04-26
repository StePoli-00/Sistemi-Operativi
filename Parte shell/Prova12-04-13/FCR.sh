#!/bin/sh
# ger stringa filetmp

cd $1
for i in *
do 
	if test -f $i -a -r $i #se $i è un file ed è leggibile
	then 
	case $i in
		*.$2) #controllo che termini con .$2
	
			
		if test $4 = A 
		then
		       	echo `pwd`>> $3
		else
			echo "la directory $1 contiene:" #altrimenti riporto a video il nome dei file e la loro prima linea
			echo $i
			echo "la prima linea di `basename $i` è : `head -1 $i`"
		fi;;
	esac
	fi
done

if test $4 = A #controllo se siamo nuovamente nella fase A poichè nella fase B devo controllare una sola dir e non tutta una gerarchia
then
for i in *
do 
	if test -d  $i -a -x $i #se $i è una directory eseguibile faccio la chiamata ricorsiva
	then 
		FCR.sh $1/$i $2 $3 $4
	fi
done
fi

