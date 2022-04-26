#/bin/sh
#controllo se siano stati passati almeno 3 parametri
if test $# -lt 3
then echo "numero parametri insufficienti";exit 1
fi
#controllo che il primo parametro sia in forma relativa semplice
case $1 in
	*/*) echo "$1 non in forma relativa semplice"; exit 2;;
	*);;
esac
primo=$1
shift
#controllo che tutte le gerarchie siano in forma assoluta e inoltre controllo che siano directory e che siano attraversabili
for i in $*
do
	case $i in
		/*) if test ! -d $i -o ! -x $i
		then echo " $i directory non attraversabile o non esistente"; exit 3
			fi;;	
		*) echo " $i non è in forma assoluta"; exit 4;;
	esac
done
#setto la variabile path
PATH=`pwd`:$PATH
export PATH
#creo due file temporanei
> /tmp/nomefile

#chiamata ricorsiva
for i in $*
do
	FCR.sh $i $primo /tmp/nomefile 
done

echo "ho trovato `wc -l < /tmp/nomefile` che corrispondo alle caratteristiche" 

indice=1 #serve per contare le posizioni all'interno del file temporaneo
file= #conterrà il nome del file

for i in `cat /tmp/nomefile`
do

	if test `expr $indice % 2` -ne 0 #se la posizione è dispari stampo il nome del file
	then echo " il file $i" 
	file=$i #salvo il nome del file
	else #altrimenti stampo il numero di linee 
		echo "ha $i linee"

		echo "inserisci un numero compreso tra 1 e $i"
		read ans
		if test $ans -ge 1 -a $ans  -le $i  #se il numero inserito è compreso
		then 
			echo "le linee prime $ans linee di $file sono:"
			head -$ans $file 
			echo "fine visualizzazione di $file"

		fi
		
	fi

indice=`expr $indice  + 1`
done
rm /tmp/nomefile
