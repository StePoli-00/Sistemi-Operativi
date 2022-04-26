#!/bin/sh
# ger DIR lista
found=false
cd $1

for i in *
do 
	if test -x $2 
	then 
	cd $2
	for j in *
	do
		if test -f $j 
		then 
		dim=`grep [0-9] $j | wc -l`>/dev/null 2>&1  #controllo se il file ha almeno un carattere numerico e conto quante sono tali linee
		if test $dim -ge 1  #se sono maggiori o uguali a 1 
		then 
			
			echo `pwd`/$j >>$3 #inserisco il nome del file dentro al file che memorizza i file trovati
			found=true
			
		fi
		fi

	done
	cd ..
	fi
done

if test  $found = true
then echo "ho trovato la directory con le specifiche richieste `pwd`"
fi

for i in *
do
	if test -d $i -a -x $i
	then FCR.sh $1/$i $2 $3 
	fi
done


