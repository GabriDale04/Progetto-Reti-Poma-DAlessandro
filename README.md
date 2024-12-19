# Progetto finale Reti di calcolatori e calcolo distribuito D’Alessandro & Poma

L’esercizio consiste nella realizzazione di un piccolo videogioco.
I giocatori dovranno raccogliere della frutta in un campo a forma di scacchiera.
Ciascun frutto varrà una diversa quantità di punti.
Lo scopo dei giocatori è raccogliere la frutta e accumulare punti entro un limite di tempo.

## SERVER

- All’avvio vengono passati per argomento la porta, il numero di giocatori per lobby e la durata delle partite.
- Il server accetta i primi n-giocatori che si connettono e poi avvia la partita, eventuali altri tentativi di connessione vengono respinti.
- Il server gestisce il gioco mantenendo una matrice con le posizioni di frutti e giocatori che viene trasmessa in tempo reale a tutti i client.
- Il server tiene traccia del tempo di gioco e dei punteggi dei singoli giocatori e li trasmette ai client mantenendoli aggiornati.
- Allo scadere del tempo il server manda un segnale di fine partita a tutti i client connessi con i punteggi finali e chiude la connessione.
- Terminata una partita il server si resetta e attende la connessione di nuovi client per iniziare una nuova partita.

## CLIENT

- All’avvio vengono passati per argomento l’IP del server, la porta del server e il nome del giocatore.
- Il client si connette al server e invia il nome utente, poi attende di essere inserito in una partita. A partita iniziata visualizza il tabellone di gioco, il tempo rimanente  e i punteggi in grafica ASCII Art.
- Il client invia al server i comandi di movimento ricevuti da tastiera.
- I punteggi e il tabellone vengono aggiornati costantemente sulla base delle informazioni fornite dal server.
- Quando il server manda il messaggio di fine partita il client smette di inviare i comandi di moviemento, visualizza la classifica ricevuta dal server, si disconnette e termina.

## DETTAGLI IMPLEMENTATIVI

- Server e Client saranno sviluppati usando il linguaggio C.
- La comunicazione fra server e client avverrà attraverso comandi testuali formattati come segue: “nomeComando valore”
