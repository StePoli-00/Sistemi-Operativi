#!/bin/sh
#FCR.sh
linee= #conterrà il numero di linee di un file
quinta= #conterrà la quinta linea di un fille
file= #nome del file creato
cd $2
 
for i in *
do
	
	if test -f $i -a -r $i	
	then 
		linee=`wc -l < $i`
	#echo "le linee sono" $linee
	 if test $linee -ge $1
	  then 
		if test $linee -ge 5
		then	
			quinta=`head -5 $i | tail -1`
			#echo "$quinta"
			file=$i.quinta
			#echo "$file"
			echo $quinta > $file
			echo `pwd`/$file >> $3
		
		else
		file=$i.NOquinta
		touch $file
		echo `pwd`/$file >> $3
		fi
	 fi
	fi
done

for j in *
do
if test -d $j -a -x $j
then FCR.sh $1 `pwd`/$j $3
fi
done

	
