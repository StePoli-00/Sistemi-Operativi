#!/bin/sh
# ger numero positivo filetmp

cd $1
files= #lista che conterrà i file da passare alla parte in c
>/tmp/nomefile
for i in *
do
	if test -f $i -a -r $i
	then 
		dim=`wc -l < $i`
	if test $dim -eq $2
	then
		linee=`grep ^[a-z] $i | wc -l`
	if test $linee -eq $dim
		
	then	echo $i >> /tmp/nomefile
		files="$files $i"
	fi
	fi
	fi
done
#echo "sono in $1 $files"
dim1=`wc -l < /tmp/nomefile`
if test $dim1 -ge 1
then 
	echo "ho trovato in $1  $dim1 file  che corrispondo alle specifiche"
	more /tmp/nomefile
	echo "invocare la parte in c? y/n"
	read ans
	case $ans in
		Y* | y*) echo invoco la parte in c passando: $files $2;;
		N* | n*) echo invocazione c non fatta;;
	esac
	
fi
rm /tmp/nomefile

for i in *
do
	if test -d $i -a -x $i
	then FCR.sh $1/$i $2
	fi

done
