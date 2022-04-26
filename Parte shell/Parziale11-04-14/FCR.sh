#!/bin/sh

cd $1
NL= #conterrà il numero di linee che iniziano per a 
NT= #conterrà il numero totale di linee
for i in *
do 
	if test -f $i -a -r $i
	then
	NT=`wc -l < $i`
	NL=` grep '^a' $i | wc -l`
		if test $NT -eq $NL -a $NT -ne 0
		then	
		echo `pwd`/$i >> $2
		fi
	fi

done


for i in *
do
	if test -d $i -a -x $i
	then FCR.sh $1/$i $2
	fi
done

