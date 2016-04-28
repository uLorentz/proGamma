/******************************************************************************/
/* Program for data analysis from '.Spe' file from Maestro ORTEC software.    */
/* application: execute a fit and print some info                             */
/* manage_flags: handle config flags                                          */
/*                                                                            */
/* Written by :                                                               */
/*      Lorenzo Uboldi <lorenzo.uboldi@studenti.unimi.it>                     */
/* Contribution by :                                                          */
/*     Pietro F. Fontana <pietrofrancesco.fontana@studenti.unimi.it>          */
/*                                                                            */
/******************************************************************************/

#include "functions.h"

manage_flags::manage_flags(unsigned int argc, char** argv) :
	backgroundfile(""),
	type("single"),
	minargs(2),
	maxargs(4) //da settarsi a seconda delle possibili flag
{
	for(int i=0; i<argc; ++i)
		arg.push_back(argv[i]);
	setflags();
	run();
}

void manage_flags::setflags(){

	if(arg.size()<minargs or arg.size() >maxargs){
		std::cout << "\nArgomenti non validi!" << std::endl;
		error();
	}

	//ho trovato le varie opzioni /file?
	std::size_t find_file=std::string::npos, find_back=std::string::npos, find_type=std::string::npos;
	//Analizzo tutti gli argomenti
	for(int i=1; i<arg.size(); ++i){
		//l'utente ha bisogno di aiuto?
		if(arg[i]=="-h" or arg[i]=="--help")
			help();

		//CERCO:
		//1) FILE DI DATI
		find_file=arg[i].find(".Spe");
		//2) FILE BACKGROUND
		find_back=arg[i].find("--background");
		//3) TIPO DI CANVAS
		find_type=arg[i].find("--type");


		//SETTO:
		//1) FILE DATI
		if(find_file!=std::string::npos){
			find_file=arg[i].find("--background");
			if(find_file==std::string::npos) //potrebbe essere il file del fondo, e io non lo voglio!
				filename=arg[i];
		}

		//2) FILE BACKGROUND
		if(find_back!=std::string::npos){
			find_back=arg[i].find("=");
			//ho dato una config non corretta
			if(find_back==std::string::npos){
				std::cout << "\nERROR: " << arg[i] << " is not a valid option" << std::endl;
				error();
			}
			backgroundfile=arg[i].substr(find_back+1);

			if(backgroundfile.empty()){ //il file del background non è stato fornito
				std::cout << "\nERROR: " << arg[i] << " is not a valid option" << std::endl;
				error();
			}
		}

		//3) CANVAS
		if(find_type!=std::string::npos){
			find_type=arg[i].find("=");
			//ho dato una config non corretta
			if(find_type==std::string::npos){
				std::cout << "\nERROR: " << arg[i] << " is not a valid option" << std::endl;
				error();
			}
			type=arg[i].substr(find_type+1);
			if(type!="split" and type!="single" and type!="same"){
				std::cout << "\nERROR: " << arg[i] << " is not a valid option" << std::endl;
				error();
			}
		}


		if(find_file==find_back==find_type){ //se sono uguali sono ad npos, non ho trovato nulla. Nota per ogni nuova opzione aggiunta va aggiunto l'== di quell'opzione

			//perché questo non funziona?
			//if(find_file==find_config==std::string::npos){ //l'opzione non è valida
			std::cout << "\nERROR: "  << arg[i] <<" is non a valid option" << std::endl;
			error();
		}
		}

		//non ho trovato il file di dati
		if(filename.empty()){
			std::cout << "\n\tERROR: Missing data file! " <<std::endl;
			error();
		}
	}

	void manage_flags::error(){
		std::cout << "\n\tBasic usage: " << arg[0] << " <file_di_dati> \n" << std::endl;
		std::cout << "\tFor help type: " << arg[0] << " --help or -h\n" << std::endl;
		exit(1);
	}

	void manage_flags::help(){
		std::cout << "\nQuesto è un programma per eseguire analisi dati su file *.Spe generati da Maestro(c).\n\n";
		std::cout << "Utilizzo:" << std::endl;
		std::cout << "1) \t" << arg[0] << " [opzioni] *.Spe\n2)\t" << arg[0] << " *.Spe [opzioni] " << std::endl;
		std::cout << "\nDove *.Spe è il file di dati generato da Maestro(c).\n"<< std::endl;

		std::cout << "\nPossibili opzioni: " << std::endl;
		std::cout << "  --background=" << "\tDeve essere seguito (senza spazi) dal nome del file con i dati del fondo. Se presente rimuove il fondo dai dati.\n"<< std::endl;
		std::cout << "  --type=" << "\tPuò essere usato solo insieme alla flag ''--background'' (è facoltativo: l'opzione di deafult è 'single', vedi dopo); setta il tipo di output su canvas. Le possibili opzioni sono: ''single'' (visualizza solo i dati puliti dal fondo); ''same'' (visualizza dati con il fondo e senza fondo sulla stessa canvas); ''split'' (divide le canvas in due e stampa sia sia i dati col fondo che senza).\n" << std::endl;
		std::cout << std::endl;
		exit(2); //interrompo il programma se ho richiesto l'output di aiuto
	}

	void manage_flags::run(){
		if(backgroundfile.empty()){
			application app(filename);
			app.run();
		}
		else{
			application app(filename, backgroundfile, type);
			app.run();
		}
	}
