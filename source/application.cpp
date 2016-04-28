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

application::application(std::string filename,  std::string backgroundfile, std::string _type ) :

	signal(filename),

	type(_type),
	ch1(0),
	ch2(0),
	stay_alive(true),
	refresh(false),
	ask(false),
	config_empty(true), //can't know if config are not empty till you prove it
	pause_root(false),
	background_removed(false)
{
	signal.read_data(data, data_times);
	signal.get_config(bins);
	if(bins.empty()){
		config_empty=true;
		configured=false;
	}
	else{ //uso l'ultima configurazione usata
		config_empty=false;
		configured=true;
		ch1=bins.back().left;
		ch2=bins.back().right;
	}

	if(backgroundfile!=""){ //allora ho dati del fondo da caricare
		dataget back(backgroundfile); //la classe back può morire anche qui, non mi serve più poi (non ha file di configurazione)
		back.read_data(background, back_times);
		remove_background();
	}
}

void application::choose_config(){ //abbastanza autoesplicativo
	std::cout << "\nLe configurazioni di canali per i fit trovate sono le "
		"seguenti: " << std::endl;
	for(unsigned int i=0;i<bins.size(); ++i){
		std::cout << i+1<< "): \n";
		std::cout << "\t"<<bins[i].left<< "\t" << bins[i].right << std::endl;
	}
	std::cout << "Quale vuoi usare? (invio per l'ultimo disponibile): ";
	short answ=0;
	std::string str;
	getline( std::cin, str);
	if(str.empty())
		str="0";
	try{
		answ=std::stoi(str);
	}
	catch(const std::invalid_argument& ia){
		std::cout << "\n\n\nATTENZIONE: hai inserito una scelta non valida. "
			"Prova di nuovo! \n" << std::endl;
		choose_config();
		return;
	}
	if(answ==0) //setto all'ultimo (l'utente ha schiacciato invio)
		answ=bins.size();
	//swappo quello da usare con l'ultimo e scrivo sul file config
	set_config(bins[answ-1].left, bins[answ-1].right);
	//potrei (per performance, questa funzione si mette a ricercare da capo,
	//un sacco di overhead) scrivere qui quello che devo fare (swappare le
	//conf e scrivere su file), ma per comodità uso la funz. già fatta
}

void application::set_config(unsigned int canale1, unsigned int canale2){
	if(canale1>=canale2){ //non ha senso
		std::cout << "E' stata inserita una configurazione non valida, verra' "
			"nuovamente proposta la scelta."<< std::endl;
		configured=false;
	}
	else {
		bool add=true; //potrei usare un goto.... ma evitiamo vah!
		for(unsigned int i=0;i<bins.size();++i){ //ricerco se ho già usato queste configurazioni in precedenza
			if(bins[i].left==canale1 && bins[i].right==canale2){
				bin_config temp=bins[i];
				bins[i]=bins.back();
				bins[bins.size()-1]=temp;
				add=false; //mi salto il prossimo if dopo il for
				break;
			}
		}
		if(add){ //configurazione mai usata in precedenza
			bin_config temp;
			temp.left=canale1;
			temp.right=canale2;
			bins.push_back(temp);
		}

		ch1=canale1;
		ch2=canale2;

		signal.writeconfig(bins);
		config_empty=false;
		configured=true;
	}
}

void application::remove_background(){
	double back_time=std::stoi(back_times.live);
	double data_time=std::stoi(data_times.live);
	//normalizzo ai tempi di data
	for(int i=0; i<background.size(); ++i)
		background[i]=(int)(((double)background[i]/(double)back_time)*data_time);
	data_cleaned.resize(data.size());
	for(int i=0;i<data.size(); ++i){
		data_cleaned[i]=data[i]-background[i];
		if(data_cleaned[i] < 0)
			data_cleaned[i]=0;
	}
	background_removed=true;
}

void application::ROOT_stuff(){

	while(stay_alive){
		//l'insieme di if che segue è un po' incasinato, si può fare di meglio?
		if(!configured){ //fit non configurato
			if(background_removed){ //c'è il fondo da rimuovere
				if(type=="single")
					root.run_no_config(data_cleaned);
				if(type=="split")
					root.run_split_no_config(data_cleaned, data);
				if(type=="same")
					root.run_same_no_config(data_cleaned, data);
			}
			else
				root.run_no_config(data);
		}
		else{ //fit configurato
			bin_config bin={ch1, ch2};
			if(background_removed){
				if(type=="single")
					root.run_one_config(data_cleaned, bin, data_times);
				if(type=="split")
					root.run_split_config(data_cleaned, data, bin, data_times);
				if(type=="same")
					root.run_same_config(data_cleaned, data, bin, data_times);
			}
			else
				root.run_one_config(data, bin ,data_times);
		}


		bool previously_configured=configured; //nel ciclo while (questo thread sta "aspettando") l'utente potrebbe cambiare le configurazioni con l'altro thread: se configured diventa true entro nell'if, ma non dovrei!, quindi mi salvo lo stato di configured prima che l'utente possa cambiarlo.
		//un po' di roba di comunicazione tra thread

		//lo ammmetto ho scritto quello che segue sotto effetto di droghe pesanti (redbull+caffé), non ho idea di cosa io abbia scritto ma funziona...
		while(stay_alive and !refresh){
			//se sono arrivato qua dovrei avere tutte le canvas e la roba di root che è partita, è arrivato il momento di chiedere all'utente cosa vuole fare della sua vita
			std::unique_lock<std::mutex> lk(mut_ask);
			ask=true; //il main thread può far comparire il menù
			cond.notify_all();
			lk.unlock();

			//finché sto nel while il systema di root processa e le canvas sono interattive
			while (!refresh and stay_alive and !pause_root)
				gSystem->ProcessEvents();

			//root è pausa, questo thread deve quindi smettere di lavorare: devo aspettare finché non mi si chiede di svegliarmi di novo
			std::unique_lock<std::mutex> lk2(mut_pause);
			cond_pause.wait(lk2, [this]{return !pause_root;}); //se pause root è true aspetto
			lk2.unlock();
		}

		if(!previously_configured){ //per poterli ricreare al ciclo successivo
			if(background_removed){ //c'è il fondo da rimuovere
				if(type=="single")
					root.delete_no_config();
				if(type=="split")
					root.delete_split_no_config();
				if(type=="same")
					root.delete_same_no_config();
			}
			else
				root.delete_no_config();
		
		}
		else{
			if(background_removed){
				if(type=="single")
					root.delete_one_config();
				if(type=="split")
					root.delete_split_config();
				if(type=="same")
					root.delete_same_config();
			}
			else
				root.delete_one_config();

		}

		mut_refresh.lock();
		refresh=false; //devo riportarlo indietro, se no si rischia di entrare in un loop infinito
		mut_refresh.unlock();
	}
}

//per far svegliare root dalla pausa
void application::wakeup_root(){
	std::unique_lock<std::mutex> lk(mut_pause);
	pause_root=false; //root può risvegliarsi
	cond_pause.notify_all();
	lk.unlock();
}

void application::run(){
	std::thread root(&application::ROOT_stuff, this);
	root.detach(); //lo rendo completamente indipendente dal main? O lo joino alla fine di run()?
	short what; //answer to the question
	bool processing=true; //controlla il loop del menù
	//se true root va in pausa per power saving

	while(processing){
		std::string str; //risposta dell'utente al menù

		std::unique_lock<std::mutex> lk(mut_ask);
		cond.wait(lk, [this]{return ask;});
		lk.unlock();

		bool fine=false; //l'utente ha inserito correttamente la scelta
		while(!fine){
			// DIVIDO il caso in cui ci sono configurazioni precedenti e il caso in cui non ci sono
			if(config_empty){
				std::cout << "Premi:\n\t(1) per configurare i canali ed eseguire il fit "
					<<  "\n\t(2) per terminare il programma" ;
				if(pause_root)
					std::cout << "\n\t(r) per far ripartire ROOT (per grafici "
						"interattivi)." << std::endl;
				else
					std::cout << "\n\t(p) per mettere in pausa ROOT "
						"(per power saving). " <<std::endl;
			}
			else{
				std::cout << "Premi:\n\t(1) per configurare i canali ed eseguire il fit"
					<< "\n\t(2) per scegliere una configurazione precedentemente"
					<<  " usata;\n\t(3) per terminare il programma";
				if(pause_root)
					std::cout << "\n\t(r) per far ripartire ROOT (per grafici "
						"interattivi)." << std::endl;
				else
					std::cout << "\n\t(p) per mettere in pausa ROOT "
						"(per power saving). " <<std::endl;
			}

			std::cout << "Inserisci: ";
			fine=true;
			getline( std::cin, str);

			//se l'utente non ha inserito p o r
			if(str!="r" and str!="p"){
				if(str.empty())
					fine=false;
				try{
					what=std::stoi(str);
				}
				catch(const std::invalid_argument& ia){
					fine=false;
				}

				if(!config_empty){
					if(!fine or (what<1 or what>3)){
						//se invalid_argument lo è già, ma se è fuori dal range di risposte possibili no
						fine=false;
						std::cout << "\n\n\nATTENZIONE: scelta non valida! "
							"Inserisci una tra le opzioni disponibili! \n"
							<< std::endl;
					}
				}
				else{
					if(!fine or (what<1 or what>2)){
						//se invalid_argument lo è già, ma se è fuori dal range di risposte possibili no
						fine=false;
						std::cout << "\n\n\nATTENZIONE: scelta non valida! "
							"Inserisci una tra le opzioni disponibili! \n" << std::endl;
					}
					if(what==2)
						what++;
				}
			}
			else{ //r o p
				what=0; //così non entro in uno degli "if" sbagliati
				if(str=="r")
					wakeup_root(); //sveglia!
				else
					pause_root=true;
			}
		}
		if(what==1){
			std::cout << "Inserisci i nuovi canali.\n";
			std::cout << "Ch1: ";
			std::cin >> ch1;
			std::cout << "Ch2: ";
			std::cin >> ch2;
			std::cin.ignore(); //rimuovo gli "a capo"
			set_config(ch1, ch2);
			mut_refresh.lock();
			refresh=true; //dico a root di aggiornare i fit e le canvas
			mut_refresh.unlock();
			if(pause_root) //se root è in pausa lo sveglio
				wakeup_root();
		}
		if(what==2){
			choose_config();
			mut_refresh.lock();
			refresh=true; //dico a root di aggiornare i fit e le canvas
			mut_refresh.unlock();
			if(pause_root) //se root è in pausa lo sveglio
				wakeup_root();
		}
		if(what==3){
			std::cout<< std::endl << "ATTENZIONE: riceverai un sacco di insulti"
				" da ROOT ma va tutto ''bene''. Ciao!" << std::endl<< std::endl;
			sleep(2);
			stay_alive=false;
			processing=false;
			if(pause_root) //se root è in pausa lo sveglio
				wakeup_root();
		}
		if(!pause_root) //se root è in una di queste due configurazioni non posso settare ask su false, se no mi inchiodo in una deadlock nel thread di root
			ask=false;
		std::cout << std::endl; //lascio un po' di spazio
	}
}
