#pragma once
#ifndef _INITINFO_H_
#define _INITINFO_H_
#define MINUTESPERDAY (24*60);
#ifndef FLOAT_EQ
#define EPSILON 0.001   // floating point comparison tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))
#define MINUTESPERDAY (24*60);
#endif

#include <string>

enum EPhase
{
//    Seed, Seedling, Juvenile, Bulbing, Flowering, Fruiting, Dead
    Seed, Vegetative, BulbGrowthWithScape, BulbGrowthWithoutScape, Dead
};
enum BBCH_code
{
    _0, _11, _41, _53, _65, _81, _97
//    '0', '11', '41', '53', '65', '81', '97'
};

// these phases correspond to two digits code of BBCH: Stage 0 (Germinate), 10 (leaf devl), 40 (buld devl), 50 (Infl. devl), 65 (full flowering; 50% open), 70 (fruiting), 80 (ripening), 97 (above ground dead) for bulbing vegetables in BBCH. See Feller et al., 1995.
// Code 65 (flowering) is meant to encompass fruting (70s) and ripening (80s)
// Code 97 (death) are considered to be an event rather than a period. 2-29-16 JH and SK
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
		initLeafNoAtHarvest = 5;
		initLeafNo = 7;
		genericLeafNo = 10;
        critPPD = 10.0;
		maxLeafLength = 50.0; // maximum length of the largest leaf grown at optimal T
		maxElongRate = 5.0; // maximum elongation rate in cm/day at optimal T
		stayGreen = 2.0; // stay green for this value times growth period after peaking before senescence begins
		storageDays = 132; // derives maximum leaf tip appearance rate depending on the planting dates
		maxLTARa = 0.3885; // asymptote for calculating maximum leaf tip appearance rate with sigmoid function
		maxLTAR = 0; // maximum leaf tip appearance rate per day, other developmental rates are expressed in relation to to this rate
		maxLIR = 0.5595 * maxLTAR;
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
		//HACK: until we move on to full C++11 compliance
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				partTable[i][j] = 0;
			}
		}

	}
	std::string description;
	std::string cultivar;
	short int GDD_rating; // GDD or GTI rating of the cv, see Stewart 1999 for conversion between MRMR and other ratings
	double initLeafNoAtHarvest; // leaf number already initated at the time of harvest
	double initLeafNo; // leaf number already initiated at the seed stage after stored for a period
	double genericLeafNo; // potential maximum number of leaves (for leaf length/area distribution)
	double Topt, Tceil, phyllochron, maxLeafLength, maxElongRate, stayGreen, storageDays, maxLTARa, maxLTAR, maxLIR, critPPD; //critical PPD for floral initiation
	double plantDensity;
	double latitude, longitude, altitude;
	double sowingDay, beginDay, emergence, endDay, scapeRemovalDay;
	double CO2;
	int year1, year2; // year to begin and year to end the simulation
	double timeStep;
	bool beginFromEmergence;
	double partTable[10][10];
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
