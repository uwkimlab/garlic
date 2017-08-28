//Class Controller
//
// Controller.h
//
#pragma once
#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_
#include "development.h"
#include "plant.h"
#include "weather.h"
#include "initinfo.h"
#include "solar.h"
#ifndef FLOAT_EQ
#define EPSILON 0.001   // floating point comparison tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))
#endif
#define MINUTESPERDAY (24*60);



class CController
{
private:
	enum InputDataFormat {GENERIC, ICASA, SPAR}; // input data format
	bool cropEmerged, cropHarvested;
//	double soilTemperature, timeStep;
	double year_begin, year_end;
	double	firstDayOfSim,	lastDayOfSim;
	char weatherFile[120], initFile[120], outputFile[120], cropFile[120], leafFile[120], logFile[120], parmFile[120];
	const char * runFile;
	int iCur, // current record number
		errorFlag;
    struct tm date;



	TInitInfo			initInfo;
//	COutput*            output;
	CPlant*             plant;
//	Timer*				time;
	TWeather*			weather;
	CDevelopment*       develop;
	InputDataFormat     weatherFormat;

public:
	CController();
    ~CController();
	void setErrStatus(int ier) {errorFlag = ier;}
	int getErrStatus() {return errorFlag;}
	char* getWeatherFile() {return weatherFile;}
	char* getInitFile() {return initFile;}
	char* getOutputFile() {return outputFile;}
	char* getLogFile() {return logFile;}
	CPlant * getPlant() {return plant;}

	void addOutputMessage(char*);
    void readWeatherFile();
	void createOutputFiles();
	void outputToCropFile(int DAP);
	void outputToLeafFile(int DAP);

//	COutput* getOutput() {return output;}
//	Timer* getTime() {return time;}
	InputDataFormat  getWeatherFormat()  {return weatherFormat;}
	double getFirstDayOfSim() {return firstDayOfSim;}
	double getLastDayOfSim() {return lastDayOfSim;}
	double get_year_begin() {return year_begin;}
	double get_year_end() {return year_end;}

	TInitInfo getInitInfo() {return initInfo;}
	TWeather * getCurrentWeather() {return weather;}
	void initialize();
	int run(const char * fn);

  void readline(istream& is, char *s, streamsize n);


};
#endif
