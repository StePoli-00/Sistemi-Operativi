#!/bin/sh
cd $1

NR=0
DIR=false 
trovato=true 

for i in *
do 
	if test -f $i
	then 
		NR=`wc -l < $i`
		if test $NR -le $2
		then trovato=false
		fi
	else
		if test -d $i
		then DIR=true
		fi
	fi
done

for i in *
do
	if test -d $i -a -x $i
		then
			FCR.sh `pwd`/$i $2 $3 
		fi
done
