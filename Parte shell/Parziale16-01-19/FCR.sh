#!/bin/sh

cd $1
f1=$2.1 #conterrà il valore di f.1
f2=$2.2 #conterrà il valore di f.2
found1=false #servirà per accertarsi di aver trovato il primo file
found2=false #servirà per accertars di aver trovato il secondo file
for i in *
do
	if test -f $i 
	then
		if test $i = $f1 
		then
		
		found1=true
		
		fi
		if test $i = $f2
		then 
		found2=true
		fi
	if test $found1 = true -a $found2 = true #solo quando li avrò trovato entrambi controllerò la loro dimensione
		then 

			dim1=`wc -l <$f1`
			dim2=`wc -l <$f2`

	if test  $dim1 -eq $dim2 #se sono uguali scrivo il percorso nel file temporaneo
	then
		
		echo `pwd`>>$3 
	       	
	else
		found1=false #altrimenti continuo la ricerca rimettend					o le variabili a false
		found2=false
	fi
	fi
	fi		
done

for i in *
do 
	if test -d $i -a -x $i
	then FCR.sh $1/$i $2 $3
	fi
done
