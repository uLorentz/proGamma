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

dataget::dataget(std::string _filename) :
	filename(_filename),
	fileconfname("")
{
}


void dataget::read_data ( std::vector<int>& data, times& t){

	std::ifstream in;
	in.open(filename.c_str()); //Apro canale in ingresso e controllo che tutto vada

	if(in.fail()){
		std::cerr<< "Errore apertura canale in ingresso con il file ''"<<filename<< "''. Chiudo. " << std::endl;
		exit(2);
	}
	std::string temp; //stringa in cui buttare le righe prima del numero di bin
	std::string times; //stringa temporanea per i tempi
	std::size_t found=0, found2=0;
	for(;;){
		getline(in, temp);
		found2=temp.find("$MEAS_TIM:"); //cerco i tempi live e real
		if(found2!=std::string::npos)
			getline(in, times);

		found=temp.find("$DATA:"); //cerco la fine dell'header del file, dopo iniziano i dati
		if(found!=std::string::npos)
			break;
	}
	found2=times.find(" ");
	
	t.live = times.substr(0,found2);
	t.real = times.substr(found2+1);
	unsigned int n; //numero di canali
	in>>n>> n; //leggo due volte: nei file ho sempre uno 0 (che è?) e poi il numero di bin

	//leggo i dati
	data.resize(n);
	for(int i=0; i<n; ++i)
		in >> data[i];

	in.close();
}

void dataget::get_config(std::vector<bin_config>& bins) {
	//creo il filename "nomefile.config" con le configurazioni dei bin o, se esiste, lo leggo
	std::size_t pos=filename.find(".Spe");
	fileconfname=filename;
	fileconfname.erase(pos, filename.length());
	fileconfname="./configure_files/"+fileconfname+".config"; //nella cartella configure_files
	//'true' if config file empty, otherwise 'false' //TEMP

	//apro un canale in ingresso "filename.config"
	std::fstream fileconfig;
	fileconfig.open(fileconfname, std::fstream::in);
	//se non esiste allora lo chuido, ne apro uno in uscita (per creare il file) e lo riempio con due zeri
	if(fileconfig.fail()){
		fileconfig.close();
		fileconfig.open(fileconfname, std::fstream::out);
		if(fileconfig.fail()){
			std::cerr <<"File out ''" << fileconfname <<"'' error."<< std::endl;
			exit(3);
		}
		//riempio con due zeri il file ma la flag "configured" rimane su false, non ho i canali del picco
		fileconfig.close();
	}
	// provo ad aggiungere un else if, nel caso in cui sia vuoto il file di conf //TEMP
	else if (fileconfig.peek() == std::ifstream::traits_type::eof()) {
		fileconfig.close();
	}
	else{ //il file esiste, leggo i valori
		bin_config temp; //variabile temporanea in cui leggere
		for(;;){ //leggo tutte le configurazioni
			fileconfig>>temp.left >>temp.right;
			if(fileconfig.eof())
				break;
			bins.push_back(temp);
		}
		fileconfig.close();
	}
}


void dataget::writeconfig(const std::vector<bin_config>& bins){
	std::ofstream out(fileconfname.c_str()); //apro canale in uscita con file di config
	//controllo che il canale funzioni
	if(out.fail()){
		std::cerr <<"File out ''" << fileconfname <<"'' error."<< std::endl;
		exit(4);
	}
	for(int i=0;i <bins.size(); ++i)
		out << bins[i].left <<std::endl << bins[i].right<< std::endl;
	out.close();
}
