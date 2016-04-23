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

//Include of ROOT
#include "TH1F.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TLine.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TStyle.h"
#include "TApplication.h"
#include "TSystem.h"

/******** MANAGE_FLAGS ********/
//Works only with "application" class
class manage_flags{
public:
	//it needs same arguments of main()
	manage_flags(unsigned int argc, char** argv);

private:
	/* METHODS */
	//analize argv and set all the flags to be passed to proGamma
	void setflags();
	//no corresponding flag found, print a short help message
	void error();
	//print help message
	void help();
	//launch application
	void run();

	/* MEMBERS */
	//string vector with all  the arguments
	std::vector<std::string> arg;
	//min and max number of arguments
	const int minargs, maxargs;

	//variables needed by application constructor
	std::string filename;
	bool config;
};


/******** BIN_CONFIG *********/
//structure containing fit bounds
struct bin_config{
	int left;
	int right;
};


/******** TIMES *********/
struct times{
	std::string live;
	std::string real;
};



/******** DATAGET *********/
//I commenti ai metodi verranno poi... ora sto cercando di far funzionare tutto...
class dataget{
public:
	//in ingresso il nome del file, il vettore coi dati e la struct coi tempi in cui salvare tutto	
	dataget(std::string _filename, std::vector<int>& data, times& t);
	//legge le config da file
	void get_config(std::vector<bin_config>& bins);
	//scrive il vettore config su file
	void writeconfig(const std::vector<bin_config> &bins);
private:
	std::string filename, fileconfname;
	
	//legge i dati
	void read_data (std::vector<int>& data, std::string& t_live, std::string& t_real);
};


/******** ROOTING *********/
class rooting{
public:
	rooting(std::vector<int>& data, times tdat, bin_config config, std::vector<int> & fondo, times tfon, short canvas_type);
private:


};




//TODO TODO TODO TODO //
// TUtta la parte del configured e empty config!



/******** APPLICATION *********/
//the "proGamma" itself
class application{
public:
	//name of '.Spe' file. 'choose=false' if you want to use the last config,
	//'choose=true' if you want to choose from all existent configs. //backgroundfile: leave blanc if no background is provided
	application(std::string filename,  std::string backgroundfile="");
	void run();

private:
	/* METHODS */


	//if a background file is provided it removes the background data from the main data
	void remove_background();
	//to choose configuration of the peak
	void choose_config();

	//set config
	void set_config(unsigned int canale1, unsigned int canale2);

	//name self-explaining
	void ROOT_stuff();

	//self explaining
	void wakeup_root();




	/* MEMBERS */
	dataget signal; // signal 

	//string with 'live time' and 'real time' of data collection
	times data_times, back_times;



	//data vector and background
	std::vector<int> data, background;
	//'true' if peak bounds are configured, otherwise 'false'
	bool configured;
	bool config_empty;
	//'true' if user want to chose a particular config, otherwise 'false'
	bool choose;
	//peak fit bounds
	unsigned int ch1, ch2;
	//vector containing all peak config
	std::vector<bin_config> bins;
	//if true root can live
	bool stay_alive;
	//when 'true' peaks config are modified and ROOT canvas should be reloaded
	bool refresh;
	//when 'true' launch the config choose menu, when 'false' wait before asking
	bool ask;

	//pause root for power saving
	bool pause_root;
	//threading stuff
	std::mutex mut_ask, mut_refresh, mut_pause;
	std::condition_variable cond, cond_pause;
};

#endif /*FUNCTIONS_H*/
