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
//#include "TObjArray.h"
//#include "TRandom.h"
#include "TApplication.h"
#include "TSystem.h"

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

//structure containing fit bounds
struct bin_config{
	int left;
	int right;
};

//the "proGamma" itself
class application{
public:
	//name of '.Spe' file. 'choose=false' if you want to use the last config,
	//'choose=true' if you want to choose from all existent configs.
	application(std::string _filename, bool _choose=false);
	void run();

private:
	/* METHODS */
	//read 'filename' file and put data in a 'data' vector
	void read_data ();
	//create config file if non existent, read config file if existent
	void get_config();
	//to choose configuration of the peak
	void choose_config();

	//set config
	void set_config(unsigned int canale1, unsigned int canale2);

	//name self-explaining
	void ROOT_stuff();

	/* MEMBERS */
	//filename=file.Spe (data), fileconfname=file.config (peak configs)
	std::string filename,fileconfname;
	//string with 'live time' and 'real time' of data collection
	std::string time_real, time_live;
	//data vector
	std::vector<float> data;
	//'true' if config file empty, otherwise 'false' //TEMP
	bool config_empty;
	//'true' if peak bounds are configured, otherwise 'false'
	bool configured;
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

	//threading stuff
	std::mutex mut_ask, mut_refresh;
	std::condition_variable cond;
};

#endif /*FUNCTIONS_H*/
