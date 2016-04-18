La versione 2.0 migliora l'interazione con l'utente. 
Ora è possibile configurare i canali del fit senza interrompere il programma.

Il programma legge il file .Spe che viene dato in input, cerca il file .config 
 se non lo trova lo crea "vuoto". Chiede di configurare i canali e poi li scrive. 
 Se invece il file .config è già pieno allora fitta con i dati contenuti.

Nel codice, per poter implementare l'interazione con l'utente, ho dovuto 
 dividere quest'ultima da quello che fa root; per tale motivo ho dovuto scrivere 
 il programma con un minimo di multithreading, quando vedete che commento con 
 threading passate oltre se non vi è chiaro cosa succede...
