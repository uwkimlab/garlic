#pragma once
#include "stdafx.h"
#include "controller.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#ifndef FLOAT_EQ
#define EPSILON 0.01   // floating point comparison tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))
#endif
#define MINUTESPERDAY (24*60);

// const a = 17.27; b = 237.7; //constant in deg C
inline double E_sat(double T){return 0.6105*exp(17.27*T/(237.7+T));}
using namespace std;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CController::CController()
{
//	time			 = NULL;
	weather 		 = NULL;
	plant			 = NULL;
//	output			 = NULL;
	iCur = 0;
	weatherFormat = GENERIC;
    firstDayOfSim = 0;
	lastDayOfSim = 365;
	year_begin = year_end = 2010;
	errorFlag = 0;
}

CController::~CController()
{
//	if ( time != NULL )
//		delete time;
	if ( weather != NULL )
		delete [] weather ;
	if ( plant != NULL )
		delete plant;
//	if ( output != NULL )
//		delete output;
}
//**********************************************************************
void CController::initialize()
{
	cout <<setiosflags(ios::left) << endl
		<< " ***********************************************************" << endl
		<< " *               Garlic Crop Simulation Model              *" << endl
		<< " *                     VERSION  1.0.01                     *" << endl
		<< " *                  Author: Soo-Hyung Kim                  *" << endl
		<< " *        University of Washington, Seattle, WA            *" << endl
		<< " ***********************************************************" << endl
		<< endl << endl;

//	char runFile[20] = "Run.dat";
	try
	{
        ifstream fstr(runFile, ios::in);
		if (!fstr)
		{
			throw "Specified run file(s) not found";			
		}
		errorFlag = 0;
		fstr.getline(weatherFile, sizeof(weatherFile), '\n');
		fstr.getline(initFile, sizeof(initFile), '\n');
		fstr.getline(outputFile, sizeof(outputFile), '\n');
		fstr.close();
		// Names of rogue output files
	}
	catch (const char* message)
	{
		cerr << "Error: " << message << "\n";
		exit(1);
	}
	{
        createOutputFiles();
		ofstream cropOut(cropFile, ios::out); 
		cropOut << setiosflags(ios::right) 
			<< setiosflags(ios::fixed)
			<< setw(3) << "DAP"
 			<< setw(9) << "year" 
 			<< setw(6) << "jday" 
			<< setw(8) << "time"
			<< setw(8) << "Leaves"
			<< setw(8) << "LA/pl"
			<< setw(8) << "LAI"
			<< setw(10) << "PFD"
			<< setw(10) << "SolRad"
			<< setw(8) << "Tair"
			<< setw(8) << "Tcan"
			<< setw(10) << "Pn"
			<< setw(10) << "ET"
			<< setw(8) << "totalDM"
			<< setw(8) << "shootDM"
			<< setw(8) << "bulbDM"
			<< setw(8) << "leafDM"
			<< setw(8) << "stemDM"
			<< setw(8) << "rootDM"
			<< setw(9) << "sol_C"
			<< setw(9) << "reserv_C"
		    << endl;

	}


	try
	{
		ifstream cfs(initFile, ios::in);
		if (!cfs) 
		{
			throw "Initialization File not found.";
		}
		cfs.getline(initInfo.description, sizeof(initInfo.description),'\n');
		cfs >> initInfo.cultivar >> initInfo.GDD_rating >> initInfo.genericLeafNo >> initInfo.maxLeafLength >> initInfo.maxElongRate >> initInfo.Topt >> initInfo.maxLTAR;
		cfs >> initInfo.latitude >> initInfo.longitude >> initInfo.altitude;
		cfs >> initInfo.year1 >> initInfo.beginDay >> initInfo.sowingDay >> initInfo.plantDensity 
			>> initInfo.year2 >> initInfo.scapeRemovalDay >> initInfo.endDay;
		cfs >> initInfo.CO2 >> initInfo.timeStep;
		if (cfs.eof()) cfs.close();
		cout << "Reading initialization file : " << initFile << endl <<endl;
		cout << setiosflags(ios::left)
			<< setw(10)	<< "Cultivar: " << initInfo.cultivar << endl
			<< setw(6) << "begin year: " << initInfo.year1 << endl
			<< setw(6) << "Sowing day: " << initInfo.sowingDay << endl 
			<< setw(6) << "Scape removal day: " << initInfo.scapeRemovalDay << endl 
			<< setw(6) << "end year: " << initInfo.year2 << endl 
			<< setw(6) << "end day: " << initInfo.endDay << endl 
			<< setw(6) << "TimeStep (min): " << initInfo.timeStep << endl
			<< setw(6) << "average [CO2]: " << initInfo.CO2 << endl << endl;

	}
	catch(const char* message)
	{
		cerr << message << "\n";
		exit(1);
	}

	year_begin = initInfo.year1;
	year_end = initInfo.year2;
	firstDayOfSim = initInfo.beginDay;
	lastDayOfSim = initInfo.endDay;
	int sim_days = 365;

	if (year_begin < year_end)
	{
		sim_days = (365-firstDayOfSim +1) + (365*abs(year_end-year_begin)) + lastDayOfSim; // todo: need to account for leap years using date time functions
	}
	else if (year_begin = year_end)
	{
		sim_days = lastDayOfSim - firstDayOfSim;
	}
	else
	{
		sim_days = lastDayOfSim;
	}

	
	cropEmerged = false;
	cropHarvested = false;

//    time = new Timer(firstDayOfSim, initInfo.year1, initInfo.timeStep/60.0); // Timer class gets stepsize in hours
	int dim = (int)((sim_days)*(24*60/initInfo.timeStep)) + 1; // counting total records of weather data
    weather = new TWeather[dim];

	plant	= new CPlant(initInfo);
//	output	= new COutput(this, plant);
}
//Read weather data file in format of SPAR data
void CController::readWeatherFile()
{
	CSolar * sun = new CSolar();

	cout << "Reading weather file : " << weatherFile << endl << endl;
	cout << setiosflags(ios::left)
        << "Please wait. This may take a few minutes.."
		<< endl << endl;



	ifstream wfs(weatherFile, ios::in);
	if (!wfs)
	{
		throw "Weather File not found.";
	}
	int i=0, count = 0;
//	ofstream ostr(logFile, ios::out |ios::app);
	const int minutesPerDay = 24*60;
	string line;
	string strDate, strTime;
//	CDate * date;
	struct tm date;
	date.tm_year = initInfo.year1;
	__time64_t curDateTime; // this type is valid until year 3000, see help

	while(!wfs.eof())
	{
//		wfs.getline(buf, sizeof(TWeather),'\n');
		if (count < 1) 
		{
			getline(wfs, line);
		}
		else
		{
			i = count-1;
			getline(wfs, line);
			istringstream inputLine(line);
			//year	jday	time	Tair	RH	wind	SolRad	rain	Tsoil

			inputLine >> weather[i].year >>  strDate >> strTime >> weather[i].airT >>weather[i].RH >> weather[i].wind
				>> weather[i].solRad >> weather[i].rain >> weather[i].soilT;
//			weather[i].year = initInfo.year;
//			weather[i].soilT = weather[i].airT;
			weather[i].PFD = weather[i].solRad*0.45*4.6;
			if (strDate.find("/")==string::npos)
			{
				weather[i].jday = atoi(strDate.c_str());
				date.tm_year = weather[i].year;// - 1900;  // tm year starts from 1900, see time.h
			}
			else
			{
                date.tm_mon = atoi(strDate.substr(0,2).c_str())-1; //January = 0
                date.tm_mday = atoi(strDate.substr(3,2).c_str());
				date.tm_year = atoi(strDate.substr(6,2).c_str());
	//			date.tm_year = initInfo.year - 1900;  // tm year starts from 1900, see time.h
	//			curDateTime = mktime(&date);
	//			date = *localtime(&curDateTime);
			    weather[i].jday = date.tm_yday + 1; //January 1 = 0
			}
			if (strTime.find(":")==string::npos)
			{
				if (atof(strTime.c_str()) <= 1.0) // time values are written as fraction between 0 and 1 (1 being hour 24)
				{
					weather[i].time = atof(strTime.c_str());
				}
				else
				{
					weather[i].time = atof(strTime.c_str())/2400; // Old campbell datalogger output time in 4 digit form (i.e., 1200) without a colon. Convert it to fraction
				}

			}
			else
			{
				date.tm_hour = atoi(strTime.substr(0,strTime.find(":")).c_str());
				date.tm_min = atoi(strTime.substr(strTime.find(":")+1,2).c_str());
//				curDateTime = mktime(&date);
//				date = *localtime(&curDateTime);
				weather[i].time = (date.tm_hour + date.tm_min/60.0)/24.0; // normalize time of day between 0 and 1
			}
			sun->SetVal(weather[i].jday, initInfo.latitude, initInfo.longitude);
			weather[i].daytime = weather[i].jday + weather[i].time;
			weather[i].dayLength = sun->daylength();

/*	
			ostr << setiosflags(ios::right) 
				<< setiosflags(ios::fixed)
				<< setw(5) << setprecision(0) << weather[i].year
				<< setw(5) << setprecision(0) << weather[i].jday
				<< setw(7) << setprecision(0) << i
				<< setw(7) << setprecision(2) << weather[i].time*24.0
				<< setw(7) << setprecision(2) << weather[i].airT
				<< setw(7) << setprecision(2) << weather[i].soilT
				<< setw(7) << setprecision(2) << weather[i].dayLength
				<< setw(7) << setprecision(2) << weather[i].daytime
				<< endl; 
*/
		}

		count++;
//		if(wfs.peek() <= 0) break;
	}
//	ostr.close();
	wfs.close();
	delete sun;

}


void CController::createOutputFiles()
{
    char dname[120]; int i, q;
    for (i=0;i <= sizeof(outputFile);i++)
    {
    	if (outputFile[i] == '\0')
    		break;
    }
    for (q=i;q >= 0;q--)
    {
    	if (outputFile[q] == '.')
    	{
    		break;
    	}
    }
    for (int y=0;y<=q;y++)
    {
    	dname[y] = outputFile[y];
    }
    dname[q+1] = '\0';
    strcpy(cropFile, dname);
	strcpy(logFile, dname);

	strcat(cropFile, "crp");
    strcat(logFile, "log");
	ofstream ostr(logFile, ios::out);
	ostr << "*** Notes and Warnings ***\n";
}

int CController::run(char * fn)
{
	runFile = fn;
	initialize();
    readWeatherFile();

	cout << "Running simulation... " << endl;

	int i = 0, DAP = 0;
	while (weather[i].jday < initInfo.sowingDay)
	{
		i++;
	}
	while ((weather[i].year == initInfo.year1 && weather[i].jday >= initInfo.sowingDay) || (weather[i].year == initInfo.year2 && weather[i].jday <= initInfo.endDay))
	{
		iCur = i; 
		if (weather[i].jday != weather[i-1].jday) ++DAP;
		plant->update(weather[i]);
//		if (FLOAT_EQ(weather[i].time,0.5))
		{
			plant->writeNote(weather[i]);
			outputToCropFile(DAP);

		}

		if (plant->get_develop()->Matured()) break;
		i++;
//		time->step();
	}

	return 0;
}


void CController::outputToCropFile(int DAP)
{
	if FLOAT_EQ(weather[iCur].time, 0.5)
	{
			ofstream ostr(cropFile, ios::app);
			ostr << setiosflags(ios::fixed) 
				<< setiosflags(ios::left)
				<< setw(3) << DAP
                << setiosflags(ios::right)
				<< setw(9) << weather[iCur].year
 				<< setw(6) << weather[iCur].jday 
				<< setw(8) << setprecision(3) << weather[iCur].time*24.0
				<< setw(8) << setprecision(2) << plant->get_develop()->get_LvsAppeared()
				<< setw(8) << setprecision(2) << plant->calcGreenLeafArea()
				<< setw(8) << setprecision(2) << plant->calcGreenLeafArea()*initInfo.plantDensity/(100*100)
				<< setw(10) << setprecision(2) << weather[iCur].PFD
				<< setw(10) << setprecision(2) << weather[iCur].solRad
				<< setw(8) << setprecision(2) << weather[iCur].airT
				<< setw(8) << setprecision(2) << plant->get_tmpr()
				<< setw(10) << setprecision(3) << plant->get_Pn()
				<< setw(10) << setprecision(3) << plant->get_ET()
				<< setw(8) << setprecision(3) << plant->get_mass()
				<< setw(8) << setprecision(3) << plant->get_shootMass()
				<< setw(8) << setprecision(3) << plant->get_bulbMass()
				<< setw(8) << setprecision(3) << plant->get_leafMass()
				<< setw(8) << setprecision(3) << plant->get_stalkMass()
				<< setw(8) << setprecision(3) << plant->get_rootMass()
				<< setw(8) << setprecision(3) << plant->get_CH2O_pool()
				<< setw(8) << setprecision(3) << plant->get_CH2O_reserve()
				<< endl;
	}

	if (plant->getNote() !="")
	{
			ofstream logStr(logFile, ios::app);
			logStr << setiosflags(ios::fixed)
				<< setiosflags(ios::left)
				<< setw(3) << DAP
                << setiosflags(ios::right)
				<< setw(9) << weather[iCur].year
 				<< setw(6) << weather[iCur].jday 
				<< setw(8) << setprecision(3) << weather[iCur].time*24.0
                << setiosflags(ios::left)
				<< setw(20)<< setiosflags(ios::skipws) << plant->getNote()
				<<endl;
	}
	return;
}
