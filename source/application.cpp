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

application::application(std::string _filename, bool _choose, std::string _backgroundfile ) :
	filename(_filename),
	backgroundfile(_backgroundfile),
	configured(false),
	ch1(0), //TODO  non ha più molto senso che siano inizializzate a 0, togliere?
	ch2(0),
	stay_alive(true),
	refresh(false),
	ask(false),
	choose(_choose),
	config_empty(true),
	pause_root(false)
{
	//read data from file and fill the vector
	read_data(filename, data, time_live, time_real );
	//if background data file exists, remove the background
	if(!backgroundfile.empty())
		remove_background();
	//check if configs exist, and read them
	get_config();
}

void application::get_config() {
	//creo il filename "nomefile.config" con le configurazioni dei bin o, se esiste, lo leggo
	std::size_t pos=filename.find(".Spe");
	fileconfname=filename;
	fileconfname.erase(pos, filename.length());
	fileconfname="./configure_files/"+fileconfname+".config"; //nella cartella configure_files

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
		configured=false;
	}
	// provo ad aggiungere un else if, nel caso in cui sia vuoto il file di conf
	else if (fileconfig.peek() == std::ifstream::traits_type::eof()) {
		fileconfig.close();
		config_empty=true;
		configured=false;
	}
	else{ //il file esiste, leggo i valori
		config_empty=false;
		bin_config temp; //variabile temporanea in cui leggere
		for(;;){ //leggo tutte le configurazioni
			fileconfig>>temp.left >>temp.right;
			if(fileconfig.eof())
				break;
			bins.push_back(temp);
		}
		if(choose) //l'utente ha chiesto di poter scegliere
			choose_config();
		else{ //uso l'ultima configurazione usata
			ch1=bins.back().left;
			ch2=bins.back().right;
		}
		//a control layer it's never useless
		//this checks if the configured size of the peak it's not wrong
		if(ch1<ch2)
			configured=true;
		else
			std::cout << "Configurazione picco errata, provare a cancellare i "
				"file di configurazione e ripetere l'operazione." << std::endl;
			configured=false;
		fileconfig.close();
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

void application::read_data (const std::string& file, std::vector<int>& d,
								std::string& t_live, std::string& t_real){
	std::ifstream in;
	in.open(file.c_str()); //Apro canale in ingresso e controllo che tutto vada
	if(in.fail()){
		std::cerr<< "Errore apertura canale in ingresso con il file ''"<<file<< "''. Chiudo. " << std::endl;
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
	t_live = times.substr(0,found2);
	t_real = times.substr(found2+1);
	unsigned int n; //numero di canali
	in>>n>> n; //leggo due volte: nei file ho sempre uno 0 (che è?) e poi il numero di bin

	//leggo i dati
	d.resize(n);
	for(int i=0; i<n; ++i)
		in >> d[i];

	in.close();
}

void application::remove_background(){
	std::vector<int> back;
	std::string back_live, back_real;
	read_data (backgroundfile, back, back_live, back_real);

	unsigned int bl=stoi(back_live);
	unsigned int tl=stoi(time_live);
	for(int i=0; i<back.size(); ++i)
		back[i]=(int)(((double)back[i]/(double)bl)*tl);
	for(int i=0;i<data.size(); ++i){
		data[i]-=back[i];
		if(data[i] < 0)
			data[i]=0;
	}
}

void application::set_config(unsigned int canale1, unsigned int canale2){
	bool valid_config;
	if(canale1>canale2){ //non ha senso
		std::cout << "Non è una configurazione valida per il picco, verrà "
			"chiesto di ripetere la configurazione." << std::endl;
		valid_config=false;
	}
	else
		valid_config=true;

	if (valid_config) {
		std::ofstream out(fileconfname.c_str()); //apro canale in uscita con file di config
		//controllo che il canale funzioni
		if(out.fail()){
			std::cerr <<"File out ''" << fileconfname <<"'' error."<< std::endl;
			exit(4);
		}
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
		for(unsigned int i =0; i<bins.size(); ++i)
			//potrei usare un append, ma come gestisco la situazione dello swap precedente?
			out << bins[i].left << std::endl << bins[i].right << std::endl;
		out.close();
		config_empty=false;
		configured=true;
	}
	else
		configured=false;
}

void application::ROOT_stuff(){
	//dichiaro tutte le varibili di root all'inizio per poter entrare nel loop senza problemi
	TApplication myApp("myApp", 0, 0);
	TCanvas canvas3, canvas4, canvas2;
	TF1* g1;
	TF1* pp;
	TF1* total;
	//BOOOOOOOOH QUi è tutta la roba che ha scritto la meroni e la lascio così per ora
	gStyle->SetOptStat(111111111);
	gStyle->SetOptFit(111111);

	//OPZIONE 1: più performante ma se non unzoommo va in crash
	/*	TH1F* gg=new TH1F("gg", " spettro", data.size(), 0.,data.size());

	//riempio il grafico e stampo tutto (devo farlo una sola volta, non ho bisogno del loop)
	for (unsigned int i=0; i<data.size(); i++)
	gg->SetBinContent(i, data[i]);
	*/

	while(stay_alive){

		//OPZIONE 2: più pesante ma meno buggosa
		TH1F* gg=new TH1F("gg", " spettro", data.size(), 0.,data.size());
		//riempio il grafico e stampo tutto (devo farlo una sola volta, non ho bisogno del loop)
		for (unsigned int i=0; i<data.size(); i++)
			gg->SetBinContent(i, data[i]);

		if(!configured){
			std::cout << "Non è stata trovata alcuna configurazione dei canali "
				"per il fit, oppure la configurazione inserita non è valida; "
				"analizzare il grafico ed inserire i canali scegliendo (1)."
				<< std::endl;
			canvas3.Modified();
			canvas3.Update();
			canvas4.Modified();
			canvas4.Update();
			canvas2.cd();
			gg->Draw();
			canvas2.Modified();
			canvas2.Update();

		}
		else{
			g1=new TF1("g1", "gaus", ch1, ch2);
			pp= new TF1("pp", "pol1", ch1, ch2);
			total= new TF1("total","gaus(0)+pol1(3)",ch1,ch2);
			// fit gaus+polinomio
			Double_t par[12],errpar[12];  //TODO 12?? da capire meglio
			for(int i=0; i<12; ++i){
				par[i]=0;
				errpar[i]=0;
			}

			// fit con solo gauss per ottenere parametri iniziali
			gg->Fit(g1,"R"); //TODO sarebbe bello poter stampare su file i risultati del fit, ma come diavolo funziona root?
			g1->GetParameters(&par[0]);

			short width=20; //larghezza stampa

			std::cout << std::endl << "### Fit results - Gauss only ###"
				<< std::endl;
			std::cout << std::left << "#" << std::setw(width) << "Max" <<
				std::setw(width) << "Mean" << std::setw(width) << "StDev"
				<< std::endl;
			std::cout << std::left << " " << std::setw(width) << par[0] <<
				std::setw(width) << par[1] << std::setw(width) << par[2] <<
				std::endl << std::endl;
			total->SetParameters(par);
			total->SetLineColor(6);
			gg->Fit(total,"R+","",ch1,ch2);

			total->GetParameters(&par[0]);
			errpar[0]=total->GetParError(0);
			errpar[1]=total->GetParError(1);
			errpar[2]=total->GetParError(2);

			std::cout<<std::endl << "### Fit results - Gauss + Polinomial(1) ###"
				<< std::endl << std::endl;
			std::cout << std::left << "#" << std::setw(width) << "Chi Squared "
				<< std::setw(width) << "N Deg. Freedom" << std::setw(width)
				<< "Probability" << std::endl;
			std::cout << std::left << " " << std::setw(width) <<
				total->GetChisquare() << std::setw(width) << total->GetNDF()
				<< std::setw(width) << total->GetProb() << std::endl
				<< std::endl;
			std::cout<< std::left <<std::setw(15)<< "## Gauss parameters ##"
				<< std::endl;
			std::cout<<std::left<< "#" << std::setw(width) << "Max" <<
				std::setw(width) << "Mean" << std::setw(width) << "StDev"
				<< std::endl;
			std::cout<<" " << std::left<< std::setw(width) << par[0] <<
				std::setw(width) << par[1] << std::setw(width) << par[2] <<
				std::endl<<std::endl;
			std::cout << "#" << std::left<< std::setw(width) << "Error_Max"
				<<std::setw(width) << "Error_Mean" << std::setw(width) <<
				"Error_StDev" << std::endl;
			std::cout << " " << std::left << std::setw(width) << errpar[0] <<
				std::setw(width) << errpar[1] << std::setw(width) << errpar[2]
				<< std::endl << std::endl;
			std::cout << std::left << "#" << std::setw(width) << "StDev/Mean"
				<< std::setw(width) << "Gaussian Area" << std::setw(width) <<
				"Error_Area"<< std::endl;
			double area_gauss=par[0]*par[2]*2.507;
			double arg=pow((par[2]*errpar[0]),2.)+pow((par[0]*errpar[2]),2.);
			double err_area_gauss=2.507*pow(arg,0.5);
			std::cout << std::left << " " << std::setw(width) << par[2]/par[1]
				<< std::setw(width) << area_gauss << std::setw(width) <<
				err_area_gauss << std::endl << std::endl;
			std::cout << std::left << "#" << std::setw(width) << "Live Time" <<
				std::setw(width) << "Real Time" << std::endl;
			std::cout<< std::left  << " "  << time_live << "\t\t     " <<
				time_real << std::endl << std::endl;

			// per disegnare le curve parziali
			g1->SetParameters(&par[0]);
			g1->SetLineColor(3);
			canvas3.cd();
			g1->Draw();
			canvas3.Modified();
			canvas3.Update();

			pp->SetParameters(&par[3]);
			pp->SetLineColor(5);
			canvas4.cd();
			pp->Draw();
			canvas4.Modified();
			canvas4.Update();
			canvas2.cd();
			gg->Draw();
			canvas2.Modified();
			canvas2.Update();
		}

		bool previously_configured=configured; //nel ciclo while (questo thread sta "aspettando") l'utente potrebbe cambiare le configurazioni con l'altro thread: se configured diventa true entro nell'if, ma non dovrei!, quindi mi salvo lo stato di configured prima che l'utente possa cambiarlo.
		//un po' di roba di comunicazione tra thread
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

		if(previously_configured){ //per poterli ricreare al ciclo successivo
			delete g1;
			delete pp;
			delete total;
		}

		mut_refresh.lock();
		refresh=false; //devo riportarlo indietro, se no si rischia di entrare in un loop infinito
		mut_refresh.unlock();
		delete gg; //OPZIONE 2
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
				std::cout << "Premi:\n\t(1) per configurare i canali ed "
					"eseguire un fit;\n\t(2) per terminare il programma;" ;
				if(pause_root)
					std::cout << "\n\t(r) per far ripartire ROOT (per grafici "
						"interattivi)." << std::endl;
				else
					std::cout << "\n\t(p) per mettere in pausa ROOT "
						"(per power saving). " <<std::endl;
			}
			else{
				std::cout << "Premi:\n\t(1) per configurare i canali ed "
					"eseguire un fit;\n\t(2) per scegliere una configurazione "
					"precedentemente usata;\n\t(3) per terminare il programma";
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
