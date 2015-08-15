#include "StdAfx.h"
#include "math.h"
#include "development.h"
#include "radiation.h"
#include <iostream>
#include <string>
//#using <mscorlib.dll>
using namespace std;

CDevelopment::CDevelopment(void)
{
	LvsInitiated = LvsAppeared = LvsExpanded=0;
    GerminationRate = EmergenceRate = LvsInitiated = LvsAppeared = LvsExpanded = Scape =0;
	GDDsum = GDD_bulb = dGDD = minBulbingDays = phyllochronsFromFI = 0;
	GDD_rating = 1000;
	Rmax_LIR = Rmax_LTAR = Rmax_Germination = Rmax_Emergence =0;
	T_base = 0.0;  T_opt = 30.0; T_ceil = 40.0; T_cur = 25.0;
	totLeafNo = juvLeafNo = 10;
	initLeafNo =  youngestLeaf = 4;
	curLeafNo =1; 
	LvsAtFI = 1;
	phyllochron = 100;
	DVS = 0.0;
}

CDevelopment::CDevelopment(const TInitInfo& info)
{
	LvsInitiated = LvsAppeared = LvsExpanded=0;
    GerminationRate = EmergenceRate = LvsInitiated = LvsAppeared = LvsExpanded = Scape =0;
	GDDsum = GDD_bulb = dGDD= minBulbingDays = phyllochronsFromFI =0;
	GDD_rating = 1000;
	Rmax_LIR = Rmax_LTAR = Rmax_Germination = Rmax_Emergence =0;
	T_base = 0.0;  T_opt = 30.0; T_ceil = 40.0; 
	totLeafNo = juvLeafNo = info.genericLeafNo;
	initLeafNo =  youngestLeaf = 4;
	curLeafNo =1; 
	LvsAtFI = 1;
	initInfo = info;
	dt = initInfo.timeStep/MINUTESPERDAY; //converting minute to day decimal, 1= a day
	DVS = 0.0;
	setParms();
}

CDevelopment::~CDevelopment(void)
{
}

void CDevelopment::setParms() // dt in days
{
	initLeafNo = 4;
	totLeafNo = juvLeafNo=initInfo.genericLeafNo;
	Rmax_LTAR = initInfo.maxLTAR; //maximal true leaf tip appearance rate at Topt, From 2011 greenhouse and growth chamber experiments using Korean Mountain, LTAR is a good phenotype that can be easily determined by experiments so normalize other rates in relation to this, SK, Nov 2012
	Rmax_Germination = Rmax_LTAR; // Assume germination rate is the same as LTAR, assume dormancy is broken
	Rmax_Emergence = Rmax_LTAR ; // 1/days to emerge for seed leaf (coleoptile) 
	Rmax_LIR = Rmax_LTAR*3/2; // lear initiation rate
	T_base = 0;
	T_opt  = initInfo.Topt;
	T_ceil = initInfo.Tceil; //from maize, 43 = 31+12
	T_cur = 15;
	phyllochron = initInfo.phyllochron;
	LvsInitiated = initLeafNo;
	GDD_rating = initInfo.GDD_rating;
	minBulbingDays = 100;
	// set germination and emergence state to correspond with initInfo input
	// if emergence date is given, set leaves appeared to start from 1 and estimate leaves initiated at emergence, SK, 8-15-2015
	if (initInfo.beginFromEmergence) 
	{
		germination.done = emergence.done= true;	
		germination.daytime = emergence.daytime = initInfo.emergence;
		LvsAppeared = 1.0;
//		LvsInitiated += LvsAppeared*Rmax_LIR/Rmax_LTAR; //need to calculate actual rates to adjust based on temperatures between sowing and emergence
	}
}


int CDevelopment::update(const TWeather& wthr)
{
	double Jday = wthr.jday;
    T_cur = wthr.airT;
//cout << "airT : " << wthr.airT << " T_cur" << T_cur <<endl;
//	if (LvsAppeared < 4) T_cur = wthr.soilT;


    double addLeafNo;
//	double dt = initInfo.timeStep/(24*60); //converting minute to day decimal, 1= a day

	if (!germination.done) 
	{
		//TODO: implement germination rate model of temperature.
		// for now assume it germinates immidiately after sowing
		GerminationRate += beta_fn(T_cur, Rmax_Germination, T_opt, T_ceil)*dt;
		if (GerminationRate >= 0)
		{
            germination.done = true;
			germination.daytime = wthr.daytime;
			cout << "* Germinated: " << Jday << endl;
		}
	}
	else // if (germination.done)
	{
		if(!emergence.done)
		{
			EmergenceRate += beta_fn(T_cur, Rmax_Emergence, T_opt, T_ceil)*dt;
			if(EmergenceRate >= 1.0)
			{
				emergence.done = true;
				emergence.daytime = wthr.daytime;
				cout << "* Emergence :" << Jday << endl;

			}
		}
		if (!floralInitiation.done)
		{
			LvsInitiated += beta_fn(T_cur, Rmax_LIR, T_opt, T_ceil)*dt;
			DVS = LvsInitiated/totLeafNo;

			curLeafNo = (int) LvsInitiated;
			if (LvsInitiated > juvLeafNo)
			// inductive phase begins after juvenile stage and ends with floral initiation (bolting), garlic is a short day plant
			{
				addLeafNo = __max(0, 0.1*(juvLeafNo-initLeafNo)*(wthr.dayLength-12.0)); 
			//	if (!coldstorage.done) addLeafNo = addLeafNo * 1.5; // continue to develop leaves when no vernalization is done
				totLeafNo = __min(20, juvLeafNo + addLeafNo); // cap the total leaves at 20
				LvsAtFI = LvsAppeared;

			}
			if (LvsInitiated >= totLeafNo)
			{
				youngestLeaf = (int) LvsInitiated;
				curLeafNo = youngestLeaf;
				floralInitiation.done =true;
			    floralInitiation.daytime = wthr.daytime;
				LvsInitiated = youngestLeaf;
				DVS = 1.0;
				cout << "* Floral initiation: " << Jday << endl;

			}
		}
		else if (floralInitiation.done) // bulbing begins one phyllochron after floral initiation in bolting cultivars of garlic, see Meredith 2008
		{
			GDD_bulb += calcGDD(T_cur)*dt;
			if (GDD_bulb >= phyllochron && (!bulbing.done))
			{
				bulbing.done = true;
			    bulbing.daytime = wthr.daytime;
				cout << "* Bulbing begins: " << Jday << endl; // with floral initiation, apical dominance is released and normal bulb formation begins with clove initiation from lateral buds
			}
			if (bulbing.done) DVS = DVS + beta_fn(T_cur, 1.0, T_opt, T_ceil)*dt/minBulbingDays; // to be used for C partitoining time scaling, see Plant.cpp

		}

			
		if ((germination.done) && (LvsAppeared < (int) LvsInitiated))
		{ 
			LvsAppeared += beta_fn(T_cur, Rmax_LTAR, T_opt, T_ceil)*dt;

			if (LvsAppeared >= (int) LvsInitiated)
			{
                LvsAppeared = (int) LvsInitiated;
			}
		}

		if (((int) LvsAppeared >= (int) LvsInitiated) && (!flowering.done || !scapeRemoval.done))
		{
			Scape += beta_fn(T_cur, Rmax_LTAR, T_opt, T_ceil)*dt; // Scape development completes after final leaf tip appeared + 5 phyllochrons

			if (Scape >= 2.0 && (!scapeAppear.done && !scapeRemoval.done)) // Scape is visible after equivalent time to 2 LTARs
			{
                scapeAppear.done = true;
			    scapeAppear.daytime = wthr.daytime;
				cout << "* Scape Tip Visible: " << Jday  << endl;
			}

			if (wthr.daytime >= (get_initInfo().scapeRemovalDay +0.5) && scapeAppear.done && !scapeRemoval.done) 
			{
				scapeRemoval.daytime = wthr.daytime;;
				scapeRemoval.done = true;
				cout << "* Scape Removed: " << Jday  << endl;				
			}

			if (Scape >= 5.0 && !flowering.done && !scapeRemoval.done)
			{
                flowering.done = true;
			    flowering.daytime = wthr.daytime;
				cout << "* Inflorescence Visible and Flowering: " << Jday  << endl;
			}


		}

	}

//	if (!maturity.done)
	{
		dGDD = calcGDD(T_cur)*dt;
		GDDsum += dGDD;
		if (GDDsum >= GDD_rating && (!maturation.done))
		{
			maturation.done = true;
			maturation.daytime = wthr.daytime;
			cout << "* Matured" << endl;
		}

	}

	return 0;
}


double CDevelopment::beta_fn(double t, double R_max, double t_opt, double t_ceil)
{
//Generalized Temperature Response Model
	double f1, g1, h1, alpha;
	const double t_base = 0, beta=1.0;
	f1 = __max(0,(t - t_base))/(t_opt-t_base);
	g1 = __max(0, (t_ceil-t))/(t_ceil-t_opt);
	alpha = beta*(t_opt-t_base)/(t_ceil-t_opt);

	h1 = __max(0.0, R_max*pow(f1,alpha)*pow(g1,beta));
	return h1;
}


double CDevelopment::calcGDD (double T_avg)
{
	double const T_base = 4.0;
	double const T_max = 40;
	return __max(0, __min(T_avg,T_max)-T_base);
}