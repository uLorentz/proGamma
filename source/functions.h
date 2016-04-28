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
		std::string filename, backgroundfile, type;
};


/******** BIN_CONFIG *********/
//structure containing fit bounds
struct bin_config{
	unsigned int left;
	unsigned int right;
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
		dataget(std::string _filename);

		//legge i dati
		void read_data (std::vector<int>& data, times& t);

		//legge le config da file
		void get_config(std::vector<bin_config>& bins);
		//scrive il vettore config su file
		void writeconfig(const std::vector<bin_config> &bins);
	private:
		std::string filename, fileconfname;
};


/******** ROOTING *********/
class rooting{
	public:
		rooting();

		/************* NO CONFIG ************/
		void run_no_config(std::vector<int>& data);
		//MUST be called after "run_no_config()"
		void delete_no_config();

		//print on same canvas uncleaned data and cleaned data
		void run_same_no_config(std::vector<int>& cleaned, std::vector<int>& uncleaned);
		//MUST be called after  "run_same_no_config()"
		void delete_same_no_config();

		//print on a split canvas uncleaned data and cleaned data
		void run_split_no_config(std::vector<int>& cleaned, std::vector<int>& uncleaned);
		//MUST be called after  "run_split_no_config()"
		void delete_split_no_config();



		/************ CONFIGURED ************/
		//COMMENTARE
		void run_one_config(std::vector<int>& data,bin_config config, times data_times);
		//MUST be called after "run_one_config()"
		void delete_one_config();

		//Come la precedente ma due sulla stessa canvas
		void run_same_config(std::vector<int>& cleaned, std::vector<int>& uncleaned,bin_config config, times data_times);
		//MUST be called after "run_same_config()"
		void delete_same_config();

		//Come la precedente ma splittate sulla stessa canvas
		void run_split_config(std::vector<int>& cleaned, std::vector<int>& uncleaned,bin_config config, times data_times);
		//MUST be called after "run_same_config()"
		void delete_split_config();


	private:
		TApplication myApp; // TApplication needed for X11 output
		TCanvas *canvas_data, *canvas_gauss, *canvas_pol;
		TH1F* gg, *gg2;
		TF1* g1, *g2;
		TF1* pp, *pp2;
		TF1* total, *total2;

};


/******** APPLICATION *********/
//the "proGamma" itself
class application{
	public:
		//name of '.Spe' file. 'choose=false' if you want to use the last config,
		//backgroundfile: leave blanc if no background is provided
		//_type: type of canvas drawing, possibile options are: "single", "split", "same
		application(std::string filename,  std::string backgroundfile="", std::string _type="single");
		void run();

	private:
		/* METHODS */
		//if a background file is provided it removes the background from the data
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

		rooting root;
		//string with 'live time' and 'real time' of data collection
		times data_times, back_times;



		//data vector and background
		std::vector<int> data, data_cleaned, background;
		std::string type;
		//if 'true' background file has been provided
		bool background_removed;
		//'true' if peak bounds are configured, otherwise 'false'
		bool configured;
		//'true' if the config file is empty, 'false' otherwise
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
