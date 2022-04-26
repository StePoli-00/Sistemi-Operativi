#!/bin/sh
if test $# -lt 3
then echo "numero parametr isufficienti"; exit 1
fi

if test $1 -lt 1
then echo "$1 non strettamente positivo"; exit 2
fi
primo=$1 #salvo il contenuto di $1 in una variabile

shift #shiftando ora posso fare tutti i controlli riferiti al nome delle gerarchie, senza avrei dovuto distinguere $1 che è un numero dai nomi assoluti, così è più facile
#settiamo la variabile path
PATH=`pwd`:$PATH
export PATH 
a=$$
>/tmp/temp$a #creo un solo file che non crei altri file nella ricorsione
echo "------ Start FCR.sh------" #stampe utili per capire se tutto è andato a buon fine
for i in $*
do
	case $i in
	
		/*)FCR.sh  $primo $i /tmp/temp$$  #controllo se la gerarchia è data in nome assoluto e faccio la ricorsione
		;; 
	*) echo "$i non è un nome assoluto";;
	esac
done
dim=`wc -l < /tmp/temp$a`
echo "ho trovato $dim file" #riporto quanti file ho trovato con le caratteristiche richieste         
for j in `cat /tmp/temp$a`
                do
		echo "sei nel file $j, inserire linea da visualizzare" #chiedo all' utente quale linea del file vuole visualizzare
		read num
		while test $num -lt 1 -o $num -ge  $primo  #il controllo era dato dal testo il while è una nostra aggiunta perchè ha senso richiedere all'utente di rimmettere il numero, altrimenti passa subito al file successivo
		do echo "$num non compreso tra 1 e $primo, riprovare"
			read num
done
echo `head -$num $j | tail -1` #riportiamo la linea richiesta dall'utente
done

#rm /tmp/temp$$	
echo "------end FCR.sh------" #stampe utili per capire se tutto è andato a buon fine

