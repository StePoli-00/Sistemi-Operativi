#!/bin/sh

cd $1

	if test -f $2 -a -r $2
	then 

		touch sorted
		sort -f $2  > sorted
		echo `pwd`/sorted >> $3
	fi

for j in *
do
	if test -d $j -a -x $j
	then FCR.sh $1/$j $2 $3
	fi
done
