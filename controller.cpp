#pragma once
#include "stdafx.h"
#include "controller.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <iomanip>
#include <cmath>
#include <time.h>
#include <stdlib.h>
#ifndef FLOAT_EQ
#define EPSILON 0.0001   // floating point comparison tolerance
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
void CController::readline(istream& is, char *s, streamsize n)
{
	is.getline(s, n, '\n');
	int i = strlen(s) - 1;
	if (s[i] == '\r') {
		s[i] = '\0';
	}
}
//**********************************************************************
void CController::initialize()
{
	cout <<setiosflags(ios::left) << endl
		<< " ***********************************************************" << endl
		<< " *               Garlic Crop Simulation Model              *" << endl
		<< " *                     VERSION  0.1.01                     *" << endl
		<< " *        Soo-Hyung Kim, Jennifer Hsiao, Kyungdahm Yun     *" << endl
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
		readline(fstr, weatherFile, sizeof(weatherFile));
		readline(fstr, initFile, sizeof(initFile));
		readline(fstr, outputFile, sizeof(outputFile));
        readline(fstr, parmFile, sizeof(parmFile));
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
			<< setw(10) << "Date"
 			<< setw(5) << "DOY"
 			<< setw(4) << "DAP"
            << setw(8) << "DVS"
			<< setw(8) << "time"
			<< setw(8) << "LeavesI"
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
			<< setw(8) << "sol_C"
			<< setw(9) << "reserv_C"
		    << endl;

	}


	try
	{
		ifstream cfs(initFile, ios::in);
        char buf[255];

		if (!cfs)
		{
			throw "Initialization File not found.";
		}
		cfs.getline(initInfo.description, sizeof(initInfo.description),'\n');
		cfs >> initInfo.cultivar >> initInfo.phyllochron >> initInfo.genericLeafNo >> initInfo.maxLeafLength >> initInfo.maxElongRate >> initInfo.maxLTAR >> initInfo.maxLIR >> initInfo.Topt >> initInfo.Tceil >> initInfo.critPPD;
		cfs >> initInfo.latitude >> initInfo.longitude >> initInfo.altitude;
		cfs >> initInfo.year1 >> initInfo.beginDay >> initInfo.sowingDay >> initInfo.emergence >> initInfo.plantDensity >> initInfo.year2 >> initInfo.scapeRemovalDay >> initInfo.endDay;
		cfs >> initInfo.CO2 >> initInfo.timeStep;
        cfs >> initInfo.Rm >> initInfo.Yg;
        if (cfs.eof()) cfs.close();
        else
        {
            string line;
            int col=0, row=0;
            double x=0.0;
            while (cfs.good())
            {
                while (getline(cfs,line))
                {
                    istringstream streamA(line);
                    col = 0;
                    while(streamA >> x)
                    {
                        initInfo.partTable[row][col] = x;
                        col++;
                    }
                    if (col>0) row++;
                }
            }
            if (cfs.eof()) cfs.close();
        }


        cout << "Reading initialization file : " << initFile << endl <<endl;
		cout << setiosflags(ios::left)
			<< setw(10)	<< "Cultivar: " << initInfo.cultivar << endl
			<< setw(6)	<< "phyllochron: " << initInfo.phyllochron << endl
			<< setw(6)	<< "T_opt (deg C): " << initInfo.Topt << endl
			<< setw(6)	<< "T_ceil (deg C): " << initInfo.Tceil << endl
			<< setw(6)	<< "max. elongation rate (cm/day): " << initInfo.maxElongRate << endl
			<< setw(6)	<< "max. leaf length (cm): " << initInfo.maxLeafLength << endl
			<< setw(6)	<< "max. leaf tip appearance rate (leaves/day): " << initInfo.maxLTAR << endl
			<< setw(6)	<< "max. leaf initiation rate (leaves/day): " << initInfo.maxLIR << endl
			<< setw(6) << "begin year: " << initInfo.year1 << endl
			<< setw(6) << "Sowing day: " << initInfo.sowingDay << endl
			<< setw(6) << "Days to emergence (DAP): " << initInfo.emergence << endl
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
	// assign adequate dimension for reading weather records
	if (year_begin < year_end)
	{
		sim_days = (366*fabs(year_end-year_begin)) + lastDayOfSim; // todo: need to account for leap years using date time functions
	}
	else if (year_begin == year_end)
	{
		sim_days = lastDayOfSim;
	}


	cropEmerged = false;
	cropHarvested = false;
	if (initInfo.emergence >= 1.0)
// if days to emergence  from init file <= 0, them model simulats emergence date. Otherwise, input of a value  > 1.0 should be provided as observed "days to emergence" SK, Dec 2015
// if emergence date is given on or before the sowing date, then simulation starts from sowing date and simulates emergence date
//TODO: take care of the case in which emergence date occurs in year 2, SK, 8-13-2015
	{
		initInfo.beginFromEmergence = true;
		cropEmerged = true;
	}

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
	date.tm_year = initInfo.year1;
	//__time64_t curDateTime; // this type is valid until year 3000, see help

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
			weather[i].CO2 = initInfo.CO2;
//			weather[i].soilT = weather[i].airT;
			weather[i].PFD = weather[i].solRad*0.45*4.6;
			if (strDate.find("/")==string::npos)
			{
				weather[i].jday = atoi(strDate.c_str());
				date.tm_year = weather[i].year - 1900;  // tm year starts from 1900, see time.h
                date.tm_yday = weather[i].jday;
			}
			else
			{
                date.tm_mon = atoi(strDate.substr(0,2).c_str())-1; //January = 0
                date.tm_mday = atoi(strDate.substr(3,2).c_str());
				date.tm_year = atoi(strDate.substr(6,2).c_str())-1900;
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
			sun->SetVal(weather[i].jday, weather[i].time, initInfo.latitude, initInfo.longitude, initInfo.altitude, weather[i].solRad);
			weather[i].daytime = weather[i].jday + weather[i].time;
			weather[i].dayLength = sun->GetDayLength();

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

int CController::run(const char * fn)
{
	runFile = fn;
	initialize();
    readWeatherFile();
    int i = 0, DAP = 0;

	int beginSim = initInfo.sowingDay;

	cout << "Running simulation... " << endl;

	while (weather[i].jday < beginSim)
	{
		i++;
	}
	while (!(weather[i].year == initInfo.year2 && weather[i].jday > initInfo.endDay))
	{
		iCur = i;
		if (i > 0 && weather[i].jday != weather[i-1].jday) ++DAP;
        if (DAP >= initInfo.emergence)
        {
            plant->update(weather[i]);
//		if (FLOAT_EQ(weather[i].time,0.5))
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
        char datebuf[20];
        struct tm curDate = {};
        curDate.tm_mon = 0;
        curDate.tm_year = weather[iCur].year-1900;  // tm year starts from 1900, see time.h
        curDate.tm_mday = weather[iCur].jday;

//  if tm_mon = 0, mktime ignores tm_wday and tm_yday and only translates tm_mday as the day of the year

        time_t curDate_t = mktime(&curDate);

        strftime(datebuf, sizeof(datebuf), "%F", localtime(&curDate_t));

			ofstream ostr(cropFile, ios::app);
			ostr << setiosflags(ios::fixed)
				<< setiosflags(ios::left)
				<< setw(11) << datebuf
                << setiosflags(ios::right)
				<< setw(4) << weather[iCur].jday
                << setw(4) << DAP
                << setw(8) << setprecision(3) << plant->get_develop()->get_DVS()
				<< setw(8) << setprecision(3) << weather[iCur].time*24.0
				<< setw(8) << setprecision(2) << plant->get_develop()->get_LvsInitiated()
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
                << setw(8) << setprecision(3) << plant->get_develop()->get_DVS()
				<< setw(8) << setprecision(3) << weather[iCur].time*24.0
                << setiosflags(ios::left)
				<< setw(20)<< setiosflags(ios::skipws) << plant->getNote()
				<<endl;
	}
	return;
}
