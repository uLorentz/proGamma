# proGamma
Un semplice programma per l'analisi di dati provenienti da file '.Spe' prodotti dal software ORTEC MAESTRO(c) per l'acquisizione di dati da rivelatori di radiazione (gamma, alfa e beta).
proGamma serve per effettuare fit gaussiani ai dati provenienti da un rivelatore Ortec. ProGamma è in grado di gestire interattivamente i bin del fit, rimuovere il fondo da una presa dati e visualizzare in diverse modalità i grafici dello spettro. 

### Prerequisiti
È necessario avere un compilatore che supporti lo standard C++11 (almeno g++ 4.8.1) ed un'installazione del software ROOT CERN (https://root.cern.ch).

### Compilazione
È sufficiente eseguire da terminale `make`.

### Utilizzo

Per informazioni sull'utilizzo di `proGamma` lanciare `./proGamma --help`.

### Autori

proGamma è stato scritto da:
    Lorenzo Uboldi <lorenzo.uboldi@studenti.unimi.it>

con il contributo di:
    Pietro F. Fontana <pietrofrancesco.fontana@studenti.unimi.it>

ed è distribuito secondo la GNU GENERAL PUBLIC LICENSE 3.0.
