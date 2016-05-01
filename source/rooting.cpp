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

rooting::rooting() :
	myApp("myApp", 0, 0)
{
	//BOOOOOOOOH QUi è tutta la roba che ha scritto la meroni e la lascio così per ora
	gStyle->SetOptStat(111111111);
	gStyle->SetOptFit(111111);
}

void rooting::run_no_config(std::vector<int>& data){

	gg=new TH1F("gg", " spettro", data.size(), 0.,data.size());
	//riempio il grafico e stampo tutto (devo farlo una sola volta, non ho bisogno del loop)
	for (unsigned int i=0; i<data.size(); i++)
		gg->SetBinContent(i, data[i]);

	std::cout << std::endl << "Nessuna configurazione dei canali per il fit trovata, analizzare il grafico ed inserire i canali scegliendo (1). " <<std::endl;
	canvas_data=new TCanvas();
	canvas_data->cd();
	gg->Draw();
	canvas_data->Modified();
	canvas_data->Update();
}

void rooting::delete_no_config(){
	delete gg;
	if(canvas_data->IsOnHeap())
		delete canvas_data;
}

void rooting::run_same_no_config(std::vector<int>& cleaned, std::vector<int>& uncleaned){

	gg=new TH1F("gg", " spettro senza fondo", cleaned.size(), 0.,cleaned.size());
	gg2=new TH1F("gg2", " spettro con fondo", uncleaned.size(), 0.,uncleaned.size());

	//riempio il grafico e stampo tutto (devo farlo una sola volta, non ho bisogno del loop)
	for (unsigned int i=0; i<cleaned.size(); i++){
		gg->SetBinContent(i, cleaned[i]);
		gg2->SetBinContent(i, uncleaned[i]);

	}
	std::cout << std::endl << "Nessuna configurazione dei canali per il fit trovata, analizzare il grafico ed inserire i canali scegliendo (1). " <<std::endl;
	canvas_data=new TCanvas();
	canvas_data->cd();
	gg->Draw();
	gg2->Draw("same"); //corretto?
	canvas_data->Modified();
	canvas_data->Update();
}

void rooting::delete_same_no_config(){
	delete gg;
	delete gg2;
	if(canvas_data->IsOnHeap()) 
		delete canvas_data;
}

void rooting::run_split_no_config(std::vector<int>& cleaned, std::vector<int>& uncleaned){

	gg=new TH1F("gg", " spettro senza fondo", cleaned.size(), 0.,cleaned.size());
	gg2=new TH1F("gg2", " spettro con fondo", uncleaned.size(), 0.,uncleaned.size());

	//riempio il grafico e stampo tutto (devo farlo una sola volta, non ho bisogno del loop)
	for (unsigned int i=0; i<cleaned.size(); i++){
		gg->SetBinContent(i, cleaned[i]);
		gg2->SetBinContent(i, uncleaned[i]);

	}
	std::cout << std::endl << "Nessuna configurazione dei canali per il fit trovata, analizzare il grafico ed inserire i canali scegliendo (1). " <<std::endl;

	canvas_data=new TCanvas();
	canvas_data->Divide(1,2);
	canvas_data->cd(1);
	gg->Draw();
	canvas_data->cd(2);
	gg2->Draw();
	canvas_data->Modified();
	canvas_data->Update();
}


void rooting::delete_split_no_config(){
	delete gg;
	delete gg2;
	if(canvas_data->IsOnHeap())
		delete canvas_data;
}


void rooting::run_one_config(std::vector<int>& data,bin_config config, times data_times){
	unsigned int ch1=config.left;
	unsigned int ch2=config.right;
	// canvas
	canvas_gauss=new TCanvas("gauss", "gauss");
	canvas_pol=new TCanvas("pol", "pol");
	canvas_data=new TCanvas("spettro", "spettro");



	gg=new TH1F("gg", " spettro", data.size(), 0.,data.size());
	//riempio il grafico e stampo tutto (devo farlo una sola volta, non ho bisogno del loop)
	for (unsigned int i=0; i<data.size(); i++)
		gg->SetBinContent(i, data[i]);

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


	//TODO ma solo io penso che è un bordello allucinante il codice che segue?
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
	std::cout<< std::left  << " "  << data_times.live << "\t\t     " <<
		data_times.real << std::endl << std::endl;

	// per disegnare le curve parziali
	g1->SetParameters(&par[0]);
	g1->SetLineColor(3);


	canvas_gauss->cd();
	g1->Draw();
	canvas_gauss->Modified();
	canvas_gauss->Update();

	pp->SetParameters(&par[3]);
	pp->SetLineColor(5);
	canvas_pol->cd();
	pp->Draw();
	canvas_pol->Modified();
	canvas_pol->Update();
	canvas_data->cd();
	gg->Draw();
	canvas_data->Modified();
	canvas_data->Update();
}

void rooting::delete_one_config(){
	delete gg;
	delete g1;
	delete pp;
	delete total;
	if(canvas_data->IsOnHeap())
		delete canvas_data;
	if(canvas_pol->IsOnHeap())
		delete canvas_pol;
	if(canvas_gauss->IsOnHeap())
		delete canvas_gauss;
}

void rooting::run_same_config(std::vector<int>& cleaned, std::vector<int>& uncleaned,bin_config config, times data_times){
	unsigned int ch1=config.left;
	unsigned int ch2=config.right;
	//canvas
	canvas_gauss=new TCanvas("gauss", "gauss");
	canvas_pol=new TCanvas("pol", "pol");
	canvas_data=new TCanvas("spettro", "spettro");



	gg=new TH1F("gg", " spettro senza fondo", cleaned.size(), 0.,cleaned.size());
	gg2=new TH1F("gg2", " spettro con fondo", uncleaned.size(), 0.,uncleaned.size());
	//riempio il grafico e stampo tutto (devo farlo una sola volta, non ho bisogno del loop)
	for (unsigned int i=0; i<cleaned.size(); i++){
		gg->SetBinContent(i, cleaned[i]);
		gg2->SetBinContent(i, uncleaned[i]);
	}

	g1=new TF1("g1_no_fondo", "gaus", ch1, ch2);
	pp= new TF1("pp_no_fondo", "pol1", ch1, ch2);
	total= new TF1("total_no_fondo","gaus(0)+pol1(3)",ch1,ch2);
	g2=new TF1("g1_con_fondo", "gaus", ch1, ch2);
	pp2= new TF1("pp_con_fondo", "pol1", ch1, ch2);
	total2= new TF1("total_con_fondo","gaus(0)+pol1(3)",ch1,ch2);

	// fit gaus+polinomio
	Double_t par[12],errpar[12];  //TODO 12?? da capire meglio
	Double_t par2[12],errpar2[12];  //TODO 12?? da capire meglio
	for(int i=0; i<12; ++i){
		par[i]=0;
		errpar[i]=0;
		par2[i]=0;
		errpar2[i]=0;
	}

	// fit con solo gauss per ottenere parametri iniziali
	gg->Fit(g1,"R"); //TODO sarebbe bello poter stampare su file i risultati del fit, ma come diavolo funziona root?
	g1->GetParameters(&par[0]);

	gg2->Fit(g2,"R"); //TODO sarebbe bello poter stampare su file i risultati del fit, ma come diavolo funziona root?
	g2->GetParameters(&par2[0]);

	short width=20; //larghezza stampa

	//TODO per ora stampa solo i dati relativi a "cleaned" (a rigore quello senza fondo)
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
	total2->SetParameters(par2);
	total2->SetLineColor(6);
	gg2->Fit(total2,"R+","",ch1,ch2);


	total->GetParameters(&par[0]);
	errpar[0]=total->GetParError(0);
	errpar[1]=total->GetParError(1);
	errpar[2]=total->GetParError(2);

	total2->GetParameters(&par2[0]);
	errpar2[0]=total2->GetParError(0);
	errpar2[1]=total2->GetParError(1);
	errpar2[2]=total2->GetParError(2);



	//TODO ma solo io penso che è un bordello allucinante il codice che segue?
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
	std::cout<< std::left  << " "  << data_times.live << "\t\t     " <<
		data_times.real << std::endl << std::endl;

	// per disegnare le curve parziali
	g1->SetParameters(&par[0]);
	g1->SetLineColor(3);
	g2->SetParameters(&par2[0]);
	g2->SetLineColor(3);
	canvas_gauss->cd();
	g1->Draw();
	g2->Draw("same");
	canvas_gauss->Modified();
	canvas_gauss->Update();

	pp->SetParameters(&par[3]);
	pp->SetLineColor(5);
	pp2->SetParameters(&par2[3]);
	pp2->SetLineColor(5);
	canvas_pol->cd();
	pp->Draw();
	pp2->Draw("same");
	canvas_pol->Modified();
	canvas_pol->Update();
	canvas_data->cd();
	gg->Draw();
	gg2->Draw("same");
	canvas_data->Modified();
	canvas_data->Update();
}

void rooting::delete_same_config(){
	delete gg;
	delete g1;
	delete pp;
	delete total;
	delete gg2;
	delete g2;
	delete pp2;
	delete total2;
	if(canvas_data->IsOnHeap())
		delete canvas_data;
	if(canvas_pol->IsOnHeap())
		delete canvas_pol;
	if(canvas_gauss->IsOnHeap())
		delete canvas_gauss;
}



void rooting::run_split_config(std::vector<int>& cleaned, std::vector<int>& uncleaned,bin_config config, times data_times){
	unsigned int ch1=config.left;
	unsigned int ch2=config.right;


	gg=new TH1F("gg", " spettro senza fondo", cleaned.size(), 0.,cleaned.size());
	gg2=new TH1F("gg2", " spettro con fondo", uncleaned.size(), 0.,uncleaned.size());
	//riempio il grafico e stampo tutto (devo farlo una sola volta, non ho bisogno del loop)
	for (unsigned int i=0; i<cleaned.size(); i++){
		gg->SetBinContent(i, cleaned[i]);
		gg2->SetBinContent(i, uncleaned[i]);
	}

	g1=new TF1("g1_no_fondo", "gaus", ch1, ch2);
	pp= new TF1("pp_no_fondo", "pol1", ch1, ch2);
	total= new TF1("total_no_fondo","gaus(0)+pol1(3)",ch1,ch2);
	g2=new TF1("g1_con_fondo", "gaus", ch1, ch2);
	pp2= new TF1("pp_con_fondo", "pol1", ch1, ch2);
	total2= new TF1("total_con_fondo","gaus(0)+pol1(3)",ch1,ch2);

	// fit gaus+polinomio
	Double_t par[12],errpar[12];  //TODO 12?? da capire meglio
	Double_t par2[12],errpar2[12];  //TODO 12?? da capire meglio
	for(int i=0; i<12; ++i){
		par[i]=0;
		errpar[i]=0;
		par2[i]=0;
		errpar2[i]=0;
	}

	// fit con solo gauss per ottenere parametri iniziali
	gg->Fit(g1,"R"); //TODO sarebbe bello poter stampare su file i risultati del fit, ma come diavolo funziona root?
	g1->GetParameters(&par[0]);

	gg2->Fit(g2,"R"); //TODO sarebbe bello poter stampare su file i risultati del fit, ma come diavolo funziona root?
	g2->GetParameters(&par2[0]);

	short width=20; //larghezza stampa

	//TODO per ora stampa solo i dati relativi a "cleaned" (a rigore quello senza fondo)
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
	total2->SetParameters(par2);
	total2->SetLineColor(6);
	gg2->Fit(total2,"R+","",ch1,ch2);


	total->GetParameters(&par[0]);
	errpar[0]=total->GetParError(0);
	errpar[1]=total->GetParError(1);
	errpar[2]=total->GetParError(2);

	total2->GetParameters(&par2[0]);
	errpar2[0]=total2->GetParError(0);
	errpar2[1]=total2->GetParError(1);
	errpar2[2]=total2->GetParError(2);



	//TODO ma solo io penso che è un bordello allucinante il codice che segue?
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
	std::cout<< std::left  << " "  << data_times.live << "\t\t     " <<
		data_times.real << std::endl << std::endl;

	// per disegnare le curve parziali
	g1->SetParameters(&par[0]);
	g1->SetLineColor(3);
	g2->SetParameters(&par2[0]);
	g2->SetLineColor(3);

	//canvas
	canvas_gauss=new TCanvas("gauss", "gauss");
	canvas_pol=new TCanvas("pol", "pol");
	canvas_data=new TCanvas("spettro", "spettro");
	//stampo gauss
	canvas_gauss->Divide(1,2);
	canvas_gauss->cd(1);
	g1->Draw();
	canvas_gauss->cd(2);
	g2->Draw();
	canvas_gauss->Modified();
	canvas_gauss->Update();

	pp->SetParameters(&par[3]);
	pp->SetLineColor(5);
	pp2->SetParameters(&par2[3]);
	pp2->SetLineColor(5);
	//stampo il polinoio
	canvas_pol->Divide(1,2);
	canvas_pol->cd(1);
	pp->Draw();
	canvas_pol->cd(2);
	pp2->Draw();
	canvas_pol->Modified();
	canvas_pol->Update();
	//stampo i dati
	canvas_data->Divide(1,2);
	canvas_data->cd(1);
	gg->Draw();
	canvas_data->Modified();
	canvas_data->Update();
	canvas_data->cd(2);
	gg2->Draw();
	canvas_data->Modified();
	canvas_data->Update();
}

void rooting::delete_split_config(){
	delete gg;
	delete g1;
	delete pp;
	delete total;
	delete gg2;
	delete g2;
	delete pp2;
	delete total2;
	if(canvas_data->IsOnHeap())
		delete canvas_data;
	if(canvas_pol->IsOnHeap())
		delete canvas_pol;
	if(canvas_gauss->IsOnHeap())
		delete canvas_gauss;
}

