#include "stdafx.h"
#include "leaf.h"
#include "weather.h"
#include <cmath>
#define MINUTESPERDAY (24*60);
using namespace std;
CLeaf::CLeaf(): COrgan()
{
	rank = totalLeaves = 0;
	length=width = area = SLA = ptnArea = plastochrons = GDD2mature = 0.0;
	greenArea = senescentArea = 0;
	initiated = appeared = mature = senescing = dead = dropped = false;
	phase1Delay = phase2Duration=ptnLength=ptnWidth = 0;
	elongRate = 0.564; // cm of elongation per GDD for corn cv. DEA, Fournier 1998
	elongAge = 0.0;
    WLRATIO = 0.05;
	A_LW = 0.75;
	
}

CLeaf::CLeaf(int n, CDevelopment * dv): COrgan(dv->get_initInfo())
{
	rank = n;
//	initInfo = dv->get_initInfo();
	length=width = area = SLA = ptnArea = plastochrons = GDD2mature = 0.0;
	totalLeaves = dv->get_totalLeaves();
	greenArea = senescentArea = 0;
	initiated = appeared = mature = senescing = dead = dropped = false;
	phase1Delay = phase2Duration=ptnLength=ptnWidth = 0;
	elongRate = 5;
	elongAge = 0.0;
	WLRATIO = 0.05;
	A_LW = 0.75;
}

void CLeaf::initialize (CDevelopment * dv) 
// set potential leaf area of the current rank, Fournier and Andrieu (1998)
{
	COrgan::initialize();
	totalLeaves = dv->get_totalLeaves();
	const double k = 0.0, l_b = 0; //k adjusts LM_min in response to total leaves, l_b sets the rank of seedling leaf (cotyledon) as base leaf
    double LM_min = dv->get_initInfo().maxLeafLength; //min length of largest leaf after full elongation	
	double L_max = sqrt(LM_min*LM_min + k*(totalLeaves - dv->get_initInfo().genericLeafNo));

	double l_t, l_pk; 
	l_t = totalLeaves+0.5; // the rank of top leaf plus 0.5, representing the rank of scape
	l_pk = 0.65 * l_t; // occurence of the longest leaf in relation to the rank of l_t

	elongRate = dv->get_initInfo().maxElongRate; // assumed cm per day at optimal temperature (T_opt) at peak age (t_pk)
	ptnLength = dv->beta_fn(rank, L_max, l_pk, l_t);
	set_growthDuration(ptnLength/elongRate); // shortest growth (linear phase) duration in physiological time when grown under constant optimal T 
//	set_prolificDuration(ptnLength/elongRate); //longest possible prolific period in physiological time  under constant optimal T 
//	set_agingDuration(ptnLength/agingRate); // shortest growth (linear phase) duration in physiological time when grown under constant optimal T 

    phase1Delay = 0; //__max(0.0, rank); //Fournier's value : -5.16+1.94*rank;
	double W_max = L_max*WLRATIO;
	double LA_max = L_max*W_max*A_LW;
	ptnArea = ptnLength*ptnLength*WLRATIO*A_LW;
	initiated = true;
}
void CLeaf::update(CDevelopment * dv)
{
	COrgan::set_temperature(dv->get_Tcur());
	COrgan::update();
	elongate(dv);
	senescence(dv);
    greenArea = area-senescentArea;


}


void CLeaf::elongate(CDevelopment * dv)
//leaf elongation based on Fournier and Andrieu (1998)
{
	double dD = dv->get_initInfo().timeStep/MINUTESPERDAY; // cm per degreeday for var DEA
	double T = dv->get_Tcur();

	if (dv->get_LvsAppeared() >= rank && !appeared) 
	{
		appeared = true;
	}

	double t_e = get_growthDuration();
	double t_pk = t_e/2;
	double T_opt = dv->get_Topt();
	double T_effect = (T/T_opt*exp(1.0-T/T_opt)); //temperature effect on final leaf size

	if (appeared && !mature)
	{
		elongAge += dv->beta_fn(T, 1.0, dv->get_Topt(), dv->get_Tceil())*dD; // Todo: implement Parent and Tardieu (2011, 2012) approach for leaf elongation in response to T and VPD, and normalized at 20C, SK, Nov 2012
		// elongAge indicates where it is now along the elongation stage or duration. duration is determined by totallengh/maxElongRate which gives the shortest duration to reach full elongation in the unit of days.
		elongAge = __min(t_e, elongAge);

		length = __max(0.0, ptnLength*(1.0 + (t_e-elongAge)/(t_e-t_pk))*pow(elongAge/t_e, (t_e/(t_e-t_pk))));
		double dL = elongRate*__max(0.0, (t_e-elongAge)/(t_e-t_pk)*pow(elongAge/t_pk,t_pk/(t_e-t_pk)))*dD;
	//	length += dL;
		width = length*WLRATIO;
		area = length*width*A_LW;
		if (length >= ptnLength || dL <= 0.0) 
		{
			mature = true;
			set_GDD2mature (get_physAge());
			expanding = false;
		}
		else expanding = true;

	}
	return;
}

void CLeaf::senescence(CDevelopment * dv)
{
	double dD = dv->get_initInfo().timeStep/MINUTESPERDAY;
	double T = dv->get_Tcur();
	double stayGreen = 1.5;  // stay green for 75% of growth period after peaking before senescence begins

	if (mature && get_physAge() >= get_GDD2mature()*stayGreen)
	{
		senescing = true;
	}
	
	if (senescing && !dead)
	{
    	agingArea = (elongRate*elongRate*WLRATIO*A_LW)/stayGreen; // aging rate in refernece to elongation rate adjusted by stayGreen trait
		double dA = agingArea*dv->calcGDD(T)*dD;
		if (senescentArea >= area) {senescentArea = area; dead = true;} else senescentArea += dA; 
	}
	else if (dead && get_physAge() >= get_GDD2mature()*stayGreen*2)
	{
		dropped = true;
	}

	return;
}


CLeaf::~CLeaf() {}
