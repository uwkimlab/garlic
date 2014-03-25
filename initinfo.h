#pragma once
#ifndef _INITINFO_H_
#define _INITINFO_H_
#define MINUTESPERDAY (24*60);
#ifndef FLOAT_EQ
#define EPSILON 0.001   // floating point comparison tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))
#define MINUTESPERDAY (24*60);
#endif

struct TInitInfo
{
public:
	TInitInfo()
	{
//		char description[255] = "default";
//		char cultivar[20]="PI3733";
		GDD_rating = 1500;
		genericLeafNo=10;
		maxLeafLength = 45.0; // maximum length of the largest leaf grown at optimal T
		maxElongRate = 1.2; // maximum elongation rate in cm/day at optimal T
		maxLTAR = 0.23; // maximum leaf tip appearance rate per day, other developmental rates are expressed in relation to to this rate
		Topt = 20;
		latitude = 38.0; longitude = 0.0; altitude = 50.0;
		sowingDay = 300;
		beginDay = 300; endDay = 200; scapeRemovalDay = 150;
		year1 = 2000;
		year2 = 2000;
		timeStep=60.0;
		plantDensity = 50.0;
		CO2 = 390.0;
	}
	char description[255];
	char cultivar[20];
	short int GDD_rating; // GDD or GTI rating of the cv, see Stewart 1999 for conversion between MRMR and other ratings
	short int genericLeafNo; // leaf number at the end of juvenile phase independent of environmental ques of leaf initiation
	double Topt, maxLeafLength, maxElongRate, maxLTAR;
	double plantDensity;
	double latitude, longitude, altitude;
	double sowingDay, beginDay, endDay, scapeRemovalDay;
	double CO2;
	int year1, year2; // year to begin and year to end the simulation
	double timeStep;
};
#endif