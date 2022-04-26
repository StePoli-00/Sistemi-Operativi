#!/bin/sh
cd $2

for j in * #j assume i valori di tutti i file presenti nella dir dove ci siamo spostato
do 
	if test -f $j -a -r $j #se questo è un file ed è leggibile
	then 
	numt=`grep 't$' $j | wc -l` #vediamo se questo file ha delle linee che finiscono con t e contiamo quante sono
       if test $numt -ge $1 #se il numero di linee è maggiore del parametro passato in FCP
       then echo `pwd`/$j >> $3	       #stampo il percorso assoluto del file e lo metto nel file temporaneo
	fi
	fi
done


for i in *   #classico for della ricorsione in cui chieidamo se c'è una dir traversabile 
do 
	if test -d $i -a -x $i
	then FCR.sh $1 $2/$i $3
	fi
done
