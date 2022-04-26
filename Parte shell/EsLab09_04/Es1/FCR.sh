#!/bin/sh
#FCR.sh
cd $1
		for j in *
		do
			if test -f $j -a -r $j
			then dim=`wc -l < $j`;
				#echo dim\=$dim
				if test $2 -eq $dim
				then	dim1=`grep [0-9] $j | wc -l`
					#echo dim1\=$dim1
					if test $dim1 -eq $dim
					then files="$files $j"
					fi
				fi
			fi
		done
		if test "$files"
		then echo Trovata directory `pwd` con $files
			echo Invocare parte c\?
			read risposta
			case $risposta in
				s* | S* | y* | Y*)echo invocata;;
				*)echo Nessuna invocazione;;
			esac
		fi
for i in *
do
	if test -d $i -a -x $i
	then FCR.sh $1/$i $2
	fi
done
