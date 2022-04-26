#!/bin/sh

cd $1
file=$2.txt

if test -f $file -a -r $file -a -w $file	
then echo `pwd`/$file >> $3
fi



for j in *
do
	if test -d $j -a -x $j 
	then FCR.sh $1/$j $2 $3 
	fi
done
