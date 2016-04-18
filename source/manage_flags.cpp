#include "functions.h"

manage_flags::manage_flags(unsigned int argc, char** argv) :
	config(false),
	minargs(2),
	maxargs(3) //da settarsi a seconda delle possibili flag

{
	for(int i=0; i<argc; ++i)
		arg.push_back(argv[i]);
	setflags();
	run();

}

void manage_flags::setflags(){

	if(arg.size()<minargs or arg.size() >maxargs)
		error();
	
	//ho trovato le varie opzioni /file?
	std::size_t find_file=0, find_config=0;	
	//Analizzo tutti gli argomenti
	for(int i=1; i<arg.size(); ++i){ 
		//l'utente ha bisogno di aiuto?
		if(arg[i]=="-h" or arg[i]=="--help")
			help();
		

		//CERCO:
		//1)
		find_file=arg[i].find(".Spe");
		//2)
		find_config=arg[i].find("--config");
		

		//SETTO: 
		//1)
		if(find_file!=std::string::npos)
			filename=arg[i];
		
		//2)
		if(find_config!=std::string::npos)
			config=true;	


		if(find_file==find_config){ //se sono uguali sono ad npos, non ho trovato nulla. Nota per ogni nuova opzione aggiunta va aggiunto l'== di quell'opzione

		//perché questo non funziona?
//		if(find_file==find_config==std::string::npos){ //l'opzione non è valida 
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
	std::cout << "Questo è un programma per eseguire analisi dati su file *.Spe generati da Maestro(c).\n\n";
	std::cout << "Usage:" << std::endl;
	std::cout << "1) \t" << arg[0] << " [options] *.Spe\n2)\t" << arg[0] << " *.Spe [options] " << std::endl;
	std::cout << "\nDove *.Spe è il file di dati generato da Maestro(c).\n"<< std::endl;  

	std::cout << "\nAvailable options: " << std::endl;
	std::cout << "  --config" << "\tSe presente il programma chiede quale usare tra le configurazioni di canali precedentemente utilizzate"<< std::endl;
	std::cout << std::endl;	
	exit(2); //interrompo il programma se ho richiesto l'output di aiuto
}


void manage_flags::run(){
	if(!config){
		application app(filename);
		app.run();
	}
	else{
		application app(filename, config);
		app.run();
	}
}

