#include <cmath>
#include <algorithm>
#include "development.h"
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
	initLeafNo = totLeafNo = juvLeafNo = youngestLeaf = 7;
	curLeafNo =1;
	LvsAtFI = 1;
	phyllochron = 100;
	DVS = 0.0;
    devPhase = Seed;
    BBCH = _0;
}

CDevelopment::CDevelopment(const TInitInfo& info)
{
	LvsInitiated = LvsAppeared = LvsExpanded=0;
    GerminationRate = EmergenceRate = LvsInitiated = LvsAppeared = LvsExpanded = Scape =0;
	GDDsum = GDD_bulb = dGDD= minBulbingDays = phyllochronsFromFI =0;
	GDD_rating = 1000;
	Rmax_LIR = Rmax_LTAR = Rmax_Germination = Rmax_Emergence =0;
	T_base = 0.0;  T_opt = 30.0; T_ceil = 40.0;
	initLeafNo = totLeafNo = juvLeafNo = youngestLeaf = 7;
	curLeafNo =1;
	LvsAtFI = 1;
	initInfo = info;
	dt = initInfo.timeStep/MINUTESPERDAY; //converting minute to day decimal, 1= a day
	DVS = 0.0;
    devPhase = Seed;
    BBCH = _0;
	setParms();
}

CDevelopment::~CDevelopment(void)
{
}

void CDevelopment::setParms() // dt in days
{
	totLeafNo = juvLeafNo = initLeafNo;
	Rmax_LTAR = initInfo.maxLTAR; //maximal true leaf tip appearance rate at Topt, From 2011 greenhouse and growth chamber experiments using Korean Mountain, LTAR is a good phenotype that can be easily determined by experiments so normalize other rates in relation to this, SK, Nov 2012
	Rmax_LIR = initInfo.maxLIR; // leaf initiation rate
    Rmax_Germination = 1.0; // Assume it takes 1/R_max day to break dormancy and germinate at T_opt
	Rmax_Emergence = Rmax_LTAR ; // 1/days to emerge for seed leaf (coleoptile)
	T_base = 0;
	T_opt  = initInfo.Topt;
	T_ceil = initInfo.Tceil; //from maize, 43 = 31+12
	T_cur = 15;
	phyllochron = initInfo.phyllochron;
	LvsInitiated = initLeafNo;
	GDD_rating = initInfo.GDD_rating;
	minBulbingDays = 100;
	germination.done = emergence.done=initInfo.beginFromEmergence; // set germination and emergence state to correspond with initInfo input.
	if (emergence.done)
	{
		// set initial leaf appearance to 1, not 0, to better describe stroage effect (2016-11-14: KDY, SK, JH)
		LvsAppeared = 1;
	}
}


int CDevelopment::update(const TWeather& wthr)
{
	double Jday = wthr.jday;
    T_cur = wthr.airT;
//    cout << "airT : " << wthr.airT << " T_cur: " << T_cur << " date: " << wthr.daytime << " daylength: " << wthr.dayLength << endl;
	// use soil temperature for early growth, but decide not to use (2016-11-14: KDY, SK, JH)
	//if (LvsAppeared < initLeafNo/2) T_cur = wthr.soilT;


//	double dt = initInfo.timeStep/(24*60); //converting minute to day decimal, 1= a day

	if (!germination.done)
	{
		//TODO: implement germination rate model of temperature.
		// for now assume it germinates immidiately after sowing
		GerminationRate += beta_fn(T_cur, Rmax_Germination, T_opt, T_ceil)*dt;
		if (GerminationRate >= 1.0)
		{
            germination.done = true;
			germination.daytime = wthr.daytime;
            devPhase = Seedling;
            BBCH = _11;
			cout << "* Germination: BBCH = " << BBCH << " " << Jday << endl;
		}
	}
	else // if (germination.done)
	{
		if(!emergence.done)
		{
			EmergenceRate += beta_fn(T_cur, Rmax_Emergence, T_opt, T_ceil)*dt;
			if(EmergenceRate > 1.0)
			{
				emergence.done = true;
				emergence.daytime = wthr.daytime;
                cout << "* Emergence: BBCH = " << BBCH << " " << Jday << endl;

			}
		}
		if (!floralInitiation.done)
		{
            //int LvsAdded = beta_fn(T_cur, Rmax_LIR, T_opt, T_ceil)*dt;
            //LvsInitiated += LvsAdded; // beta_fn(T_cur, Rmax_LIR, T_opt, T_ceil)*dt;
            LvsInitiated += beta_fn(T_cur, Rmax_LIR, T_opt, T_ceil)*dt;
            int critPPD = initInfo.critPPD; // this may have to be optimized in combination with Rmax_LIR to match the total leaf no for each cv, 6/21/16 SK, KY, JH
            int maxLeafNo = 20;

			//if (LvsInitiated >= initLeafNo)
			// inductive phase begins after juvenile stage and ends with floral initiation (bolting), garlic is a short day plant
			//	if (!coldstorage.done) addLeafNo = addLeafNo * 1.5; // continue to develop leaves when no vernalization is done
            totLeafNo = min(maxLeafNo, (int) LvsInitiated); // cap the total leaves at 20
            curLeafNo = totLeafNo;

			if ((wthr.dayLength >= critPPD && wthr.jday <= 171) || totLeafNo >= maxLeafNo) // Summer solstice
			{
				youngestLeaf = totLeafNo;
				curLeafNo = youngestLeaf;
				floralInitiation.done =true;
			    floralInitiation.daytime = wthr.daytime;
				LvsInitiated = youngestLeaf;
                LvsAtFI = LvsAppeared;

                cout << "* Floral initiation: BBCH = " << BBCH << " " << Jday << endl;

			}
		}
		else if (floralInitiation.done) // bulbing begins one phyllochron after floral initiation in bolting cultivars of garlic, see Meredith 2008
		{
			GDD_bulb += calcGDD(T_cur)*dt;
			if (GDD_bulb >= phyllochron && (!bulbing.done))
			{
				bulbing.done = true;
			    bulbing.daytime = wthr.daytime;
				cout << "* Bulbing begins: BBCH = " << BBCH << " " << Jday << endl; // with floral initiation, apical dominance is released and normal bulb formation begins with clove initiation from lateral buds
                devPhase = Bulbing;
                BBCH = _41;
			}
			// if (bulbing.done) DVS = DVS + beta_fn(T_cur, 1.0, T_opt, T_ceil)*dt/minBulbingDays; // to be used for C partitoining time scaling, see Plant.cpp

		}


		if ((LvsAppeared < (int) LvsInitiated))
		{
			LvsAppeared += beta_fn(T_cur, Rmax_LTAR, T_opt, T_ceil)*dt;
			if (LvsAppeared >= (int) LvsInitiated)
			{
                LvsAppeared = (int) LvsInitiated;
			}
        }

		if (((int) LvsAppeared >= (int) LvsInitiated) && floralInitiation.done && (!flowering.done || !scapeRemoval.done))
		{
			Scape += beta_fn(T_cur, Rmax_LTAR, T_opt, T_ceil)*dt; // Scape development completes after final leaf tip appeared + 5 phyllochrons

			if (Scape > 1.0 && (!scapeAppear.done && !scapeRemoval.done)) // Scape is visible after equivalent time to 1 LTARs
			{
                scapeAppear.done = true;
			    scapeAppear.daytime = wthr.daytime;
                devPhase = Bulbing;
                BBCH = _53;
				cout << "* Scape Tip Visible: BBCH = " << BBCH << " " << Jday  << LvsAppeared << LvsInitiated << endl;
			}

			if (wthr.daytime >= (get_initInfo().scapeRemovalDay +0.5) && scapeAppear.done && !scapeRemoval.done)
			{
				scapeRemoval.daytime = wthr.daytime;;
				scapeRemoval.done = true;
                devPhase = Bulbing;
				cout << "* Scape Removed and Bulb Maturing: BBCH = " << BBCH << " " << Jday  << endl;
			}

			if (Scape >= 2.0 && !flowering.done && !scapeRemoval.done)
			{
                flowering.done = true;
			    flowering.daytime = wthr.daytime;
                devPhase = Flowering;
                BBCH = _65;
				cout << "* Inflorescence Visible and Flowering: BBCH = " << BBCH << " " << Jday  << endl;
            }
            if (Scape >= 3.0 && !bulbiling.done && !scapeRemoval.done)
            {
                bulbiling.done = true;
                bulbiling.daytime = wthr.daytime;
                devPhase = Fruiting;
                BBCH = _81;
                cout << "* Bulbil and Bulb Maturing: BBCH = " << BBCH << " " << Jday  << endl;

            }


		}

	}

//	if (!maturity.done)
	{
        dGDD = calcGDD(T_cur)*dt;
		GDDsum += dGDD;
        DVS += (beta_fn(T_cur, Rmax_LTAR, T_opt, T_ceil)*dt)/(totLeafNo); // DVS counter. Relative to LTAR. Reaches 1.0 when flowering and > 1.0 after flowering.

        if (GDDsum >= GDD_rating && !maturation.done)
		{
			maturation.done = true;
			maturation.daytime = wthr.daytime;

			cout << "* Ready for harvest: BBCH = " << BBCH << " " << Jday << endl;
		}

	}

	return 0;
}


double CDevelopment::beta_fn(double t, double R_max, double t_opt, double t_ceil)
{
//Generalized Temperature Response Model
	double f1, g1, h1, alpha;
	const double t_base = 0, beta=1.0;
	f1 = max(0.0, (t - t_base))/(t_opt-t_base);
	g1 = max(0.0, (t_ceil-t))/(t_ceil-t_opt);
	alpha = beta*(t_opt-t_base)/(t_ceil-t_opt);

	h1 = max(0.0, R_max*pow(f1,alpha)*pow(g1,beta));
	return h1;
}


double CDevelopment::calcGDD (double T_avg)
{
	double const T_base = 4.0;
	double const T_max = 40;
	return max(0.0, min(T_avg,T_max)-T_base);
}
