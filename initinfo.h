#pragma once
#ifndef _INITINFO_H_
#define _INITINFO_H_
#define MINUTESPERDAY (24*60);
#ifndef FLOAT_EQ
#define EPSILON 0.001   // floating point comparison tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))
#define MINUTESPERDAY (24*60);
#endif

enum EPhase
{
    Seed, Juvenile, Bulbing, ScapeVisible, Flowering, FruitGrowth, BulbGrowth, Aging
};
// these phases correspond to Stage 0 (Germinate), 1 (leaf devl), 4 (buld devl), 5 (Infl. devl), 6 (flowering), 7 (fruiting), 8 (ripening) for bulbing vegetables in BBCH. See Feller et al., 1995.
//BulbGrowth phase: Scape has been removed and carbohydrates go to growh bulb.
//FruitGrowth phase: Scape has not been removed and carbohydrates go to fill 1) underground bulb and 2) bulbils in the inflorescence.
struct TInitInfo
{
public:
	TInitInfo()
	{
//		char description[255] = "default";
//		char cultivar[20]="PI3733";
		GDD_rating = 2500;
		genericLeafNo=10;
		maxLeafLength = 50.0; // maximum length of the largest leaf grown at optimal T
		maxElongRate = 5.0; // maximum elongation rate in cm/day at optimal T
		maxLTAR = 0.25; // maximum leaf tip appearance rate per day, other developmental rates are expressed in relation to to this rate
		Topt = 30;
		Tceil = 43;
		phyllochron = 100.0;
		latitude = 38.0; longitude = 0.0; altitude = 50.0;
		sowingDay = emergence = 300;
		beginDay = 300; endDay = 200; scapeRemovalDay = 150;
		year1 = 2000;
		year2 = 2000;
		timeStep=60.0;
		plantDensity = 50.0;
		CO2 = 390.0;
		beginFromEmergence = false;
        Rm = 0.015;
        Yg = 0.75;
        
	}
	char description[255];
	char cultivar[20];
	short int GDD_rating; // GDD or GTI rating of the cv, see Stewart 1999 for conversion between MRMR and other ratings
	short int genericLeafNo; // leaf number at the end of juvenile phase independent of environmental ques of leaf initiation
	double Topt, Tceil, phyllochron, maxLeafLength, maxElongRate, maxLTAR;
	double plantDensity;
	double latitude, longitude, altitude;
	double sowingDay, beginDay, emergence, endDay, scapeRemovalDay;
	double CO2;
	int year1, year2; // year to begin and year to end the simulation
	double timeStep;
	bool beginFromEmergence;
    double partTable[10][10] = {{0}};
    double Rm, Yg; // maint respiration coeff and sysnthesis efficiency
    /*
    {
        {Seed, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {Juvenile, 0.0, 0.0, 0.9, 0.1, 0.0, 0.0},
        {Bulbing, 0.0, 0.0, 0.7, 0.1, 0.1, 0.1},
        {ScapeVisible, 0.0, 0.0, 0.2, 0.1, 0.2, 0.5},
        {Flowering, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5},
        {FruitGrowth, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5},
        {BulbGrowth, 0.0, 0.0, 0.0, 0.0, 0.2, 0.8},
        {Aging, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    };
    */
 
};
#endif