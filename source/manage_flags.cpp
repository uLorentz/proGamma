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
	all_graph(false),
	gross(false),
	minargs(2),
	maxargs(6) //da settarsi a seconda delle possibili flag (TODO 6 è corretto?)
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
	std::size_t find_all=std::string::npos, find_gross=std::string::npos;
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
		//4) ALL GRAPH
		find_all=arg[i].find("--all-graph");
		//5) GROSS
		find_gross=arg[i].find("--gross");

		//SETTO:
		//1) FILE DATI
		if(find_file!=std::string::npos){
			find_back=arg[i].find("--background");
			if(find_back==std::string::npos){ //potrebbe essere il file del fondo, e io non lo voglio!
				filename=arg[i];
				find_back=std::string::npos; //riporto alla posizione iniziale
			}
		}

		//2) FILE BACKGROUND
		if(find_back!=std::string::npos){
			find_back=arg[i].find("=");
			//ho dato una config non corretta
			if(find_back==std::string::npos){
				std::cout << "\nERRORE: "  << arg[i] <<" non è un'opzione valida, lanciare il programma con l'opzione '--help' per maggiori informazioni." << std::endl;
				error();
			}
			backgroundfile=arg[i].substr(find_back+1);

			if(backgroundfile.empty()){ //il file del background non è stato fornito
				std::cout << "\nERRORE: "  << arg[i] <<" non è un'opzione valida, lanciare il programma con l'opzione '--help' per maggiori informazioni." << std::endl;
				error();
			}
		}

		//3) CANVAS
		else if(find_type!=std::string::npos){
			find_type=arg[i].find("=");
			//ho dato una config non corretta
			if(find_type==std::string::npos){
				std::cout << "\nERRORE: "  << arg[i] <<" non è un'opzione valida, lanciare il programma con l'opzione '--help' per maggiori informazioni." << std::endl;
				error();
			}
			type=arg[i].substr(find_type+1);
			if(type!="split" and type!="single" and type!="same"){
				std::cout << "\nERRORE: "  << arg[i] <<" non è un'opzione valida, lanciare il programma con l'opzione '--help' per maggiori informazioni." << std::endl;
				error();
			}
		}
		//4) ALL GRAPH
		else if(find_all!=std::string::npos)
			all_graph=true;
		//5) GROSS
		else if(find_gross!=std::string::npos)
			gross=true;	

	
		if(find_file==std::string::npos and 
		   find_back==std::string::npos and
		   find_type==std::string::npos and
		   find_all==std::string::npos and
		   find_gross==std::string::npos) //TODO Per ogni nuova flag va aggiunto qui il controllo. Probabilmente non è il modo più efficiente  efunzionale..
		{	
			std::cout << "\nERRORE: "  << arg[i] <<" non è un'opzione valida, lanciare il programma con l'opzione '--help' per maggiori informazioni." << std::endl;
			error();
		}
	}

	//non ho trovato il file di dati
	if(filename.empty()){
		std::cout << "\n\tERRORE: File di dati mancante! " <<std::endl;
		error();
	}
}

void manage_flags::error(){
	std::cout << "\n\tUtilizzo: " << arg[0] << " <file_di_dati> \n" << std::endl;
	std::cout << "\tPer un aiuto sulle opzioni disponibili: " << arg[0] << " --help o -h\n" << std::endl;
	exit(1);
}

void manage_flags::help(){
	std::cout << "\nQuesto è un programma per eseguire analisi dati su file *.Spe generati da Maestro(c).\n\n";
	std::cout << "Utilizzo:" << std::endl;
	std::cout << "1) \t" << arg[0] << " [opzioni] *.Spe\n2)\t" << arg[0] << " *.Spe [opzioni] " << std::endl;
	std::cout << "\nDove *.Spe è il file di dati generato da Maestro(c).\n"<< std::endl;

	std::cout << "\nPossibili opzioni: " << std::endl;
	std::cout << "  --background=" << "\tDeve essere seguito (senza spazi) dal nome del file con i dati del fondo. Se presente rimuove il fondo dai dati.\n"<< std::endl;
	std::cout << "  --type=" << "\tPuò essere usato solo insieme alla flag ''--background'' (è facoltativo: l'opzione di deafult è 'single', vedi dopo); setta il tipo di output su canvas. Le possibili opzioni sono: \n\t\t''single'' (visualizza solo i dati puliti dal fondo, opzione di default);\n\t\t''same'' (visualizza dati con il fondo e senza fondo sulla stessa canvas); \n\t\t''split'' (divide le canvas in due e stampa sia sia i dati col fondo che senza).\n" << std::endl;
	std::cout << "  --all-graph" << "\tStampa tutti i grafici: dati, polinomio e gaussiana. Senza questa opzione viene stampato solo il grafico dello spettro.\n"<< std::endl;
	std::cout << "  --gross\tStampa i dati grezzi insieme al fondo, la configurazione di default è 'same'. I dati sono blu, il fondo grigio. La configurazione './proGamma file1.Spe --background=file2.Spe --gross' può essere utilizzata per confrontare due spettri generici.\n" <<std::endl;
	std::cout << std::endl;
	exit(2); //interrompo il programma se ho richiesto l'output di aiuto
}

void manage_flags::run(){
	if(backgroundfile.empty() and gross){
		std::cout << "\nAttento: hai richiesto di stampare il fondo insieme ai dati grezzi ma non hai fornito nessun dato per il fondo.\n"
			 <<"L'opzione ''--gross'' può essere usata solo insieme all'opzione ''--background=nomefile''."<< std::endl;
		exit(2);
	}
	if(backgroundfile.empty()){
		application app(filename, all_graph);
		app.run();
	}
	else{
		application app(filename,all_graph, backgroundfile, gross, type);
		app.run();
	}
}

