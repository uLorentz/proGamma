/********************************************************************************/
/* Classi per l'analisi dei dati di un rivelatore salvati in file .Spe. 	*/
/* Application: esegue il fit di gaussiane e le analizza 			*/
/* Manage_flags: gestisce le flags del main					*/
/*										*/
/*			Written by Lorenzo Uboldi	 			*/
/*			Contribution by Pietro F. Fontana			*/
/*										*/
/********************************************************************************/



#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <iomanip>
//threading
#include <thread>
#include <mutex>
#include <condition_variable>

//Include di root
#include "TH1F.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TLine.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TStyle.h"
//#include "TObjArray.h" 
//#include "TRandom.h"
#include "TApplication.h"
#include "TSystem.h"


//FUNZIONA SOLO CON LA CLASSE APPLICATION
class manage_flags{
public:
	//da passare le stesse variabili del main!
	manage_flags(unsigned int argc, char** argv);
private:
	/* METHODS */

	//analizza argv
	void setflags();
	//nessuna corrispondenza tra flags trovata
	void error();
	//stampa info sul programma
	void help();	
	//fa partire application
	void run(); 

	/* MEMBERS */
	//vettore di stringhe con tutti gli argomenti	
	std::vector<std::string> arg;

	const int minargs, maxargs; //numero minimo e massimo di args che posso avere


	//variabili da passare al costruttore di application
	std::string filename;
	bool config;

};

//struttura con gli estremi del fit, rende meno dispersivo il codice
struct bin_config{
	int left;
	int right;
};

//classe che esegue tutto
class application{
public:
	//nome file .Spe. Choose=false se si vuole usare l'ultima config disponibile, =true se si vuole scegliere tra tutte le config
	application(std::string _filename, bool _choose=false); 
	void run();
private:
/**METHODS**/
	//legge sul file "filename" i dati e riempie il vettore data
	void read_data ();
	//da chiamare nel costruttore, crea il file di config se non esiste, se esiste legge le config 
	void get_config();
	//fa scegliere la config 
	void choose_config();
	
	//setta la config
	void set_config(unsigned int canale1, unsigned int canale2);
	//da definire un metodo che fa le cose di root e uno che chiede diinserire nuovi bin, entrambi thread, il secondo può killare il primo, modificare il file di config e ripartire

	//tutta la pappardella di root
	void ROOT_stuff(); 

/**members**/
	std::string filename,fileconfname; //filename=file.Spe (dati), fileconfname=file.config (configurazioni dei picchi)
	std::string time_real, time_live; //stringhe con i tempi vivi e reali della presa dati
	std::vector<float> data; //dati
	bool configured; // sono settati i canali del picco?
	bool choose; //scelgo tra tutte le conf o uso l'ultima?
	unsigned int ch1, ch2; //i canali da cui fare il fit
	std::vector<bin_config> bins; //tutte le conf
	bool stay_alive; //finché è true ROOT vive
	bool refresh; //quando diventa true i dati del fit sono cambiati e le canvas vanno aggiornate
	bool ask; //true parte il menù di scelta delle opzioni, false aspetta a chiedere

	//threading stuff	
	std::mutex mut_ask, mut_refresh;
	std::condition_variable cond;
};

#endif /*FUNCTIONS_H*/
