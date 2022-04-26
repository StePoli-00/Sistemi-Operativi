#!/bin/sh






for i in $params
do 
	echo fase per $i
	FCR.sh $i $x /tmp/conta$$
done

echo il numero di dir =`wc-l < /tmp/conta$$`
for in `cat /tmp/conta$$`
do
	echo "trovato la dir $i, contiene i seguenti file:"
	cd $i
	echo "file: `pwd`/$file"
	echo "la linea $x-esima "

done
