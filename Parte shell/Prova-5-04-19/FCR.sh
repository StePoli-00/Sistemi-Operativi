#!/bin/sh
# ger primo tmpfile tmplunghezza
dim= #conterrrà la lunghezza delle linee del file
cd $1 
for i in *
do 
	#controllo se $i è un file è leggibile e ha lo stesso nome di $2
	if test -f $i -a -r $i -a $i = $2
	then 
		#conto quante sono le linee del file
		dim=`wc -l < $i`
	if test $dim -ge 4 #se il file ha lunghezza in linee maggiore di 4
	then
		echo `pwd`/$i $dim  >> $3 #salvo il nome assoluto e la lunghezza
    		
	fi
	fi
	
done

for i in *
do
	if test -d $i -a -x $i
	then 
		FCR.sh $1/$i $2 $3
	fi
	
done
