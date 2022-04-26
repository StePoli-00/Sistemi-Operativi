#!/bin/sh
cd $1
conta=`expr $2 + 1`
for i in *
do
	if test -d  $i -a -x $i
	then
	       	FCR.sh `pwd`/$i $conta $3 $4
		ret=$?
		echo $ret
		read prec < /tmp/filetmp
		if test $ret -gt $prec #se il valore in ricosione è maggiore
		then 
			echo $ret > /tmp/filetmp #aggiorno il numero di livello raggiunto
		fi
	fi


done
exit $conta
