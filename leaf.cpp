#include "leaf.h"
#include "weather.h"
#include <cmath>
#include <algorithm>
#define MINUTESPERDAY (24*60);
using namespace std;
CLeaf::CLeaf(): COrgan()
{
	rank = totalLeaves = 0;
	length=width = area = SLA = ptnArea = plastochrons = GDD2mature = 0.0;
	greenArea = senescentArea = 0;
	initiated = appeared = mature = senescing = dead = dropped = false;
	phase1Delay = phase2Duration=ptnLength=ptnWidth = 0;
	elongRate = 5; // cm of elongation per day at Topt
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
	update_potentials(dv);
	initiated = true;
}

void CLeaf::update(CDevelopment * dv)
{
	COrgan::set_temperature(dv->get_Tcur());
	COrgan::update();
	update_potentials(dv);
	elongate(dv);
	senescence(dv);
	greenArea = area-senescentArea;
}

void CLeaf::update_potentials(CDevelopment *dv)
{
	totalLeaves = dv->get_totalLeaves();
	const double k = 0.0, l_b = 0; //k adjusts LM_min in response to total leaves, l_b sets the rank of seedling leaf (cotyledon) as base leaf
    double LM_min = dv->get_initInfo().maxLeafLength; //min length of largest leaf after full elongation
	// no length adjustment necessary for garlic, unlike MAIZE (KY, 2016-10-12)
	//double L_max = sqrt(LM_min*LM_min + k*(totalLeaves - dv->get_initInfo().genericLeafNo));
	double L_max = LM_min;
	elongRate = dv->get_initInfo().maxElongRate; // assumed cm per day at optimal temperature (T_opt) at peak age (t_pk)

	double l_t, l_pk, a, b;
//	l_t = totalLeaves; // the rank of top leaf + scape
//	l_pk = 0.65 * l_t; // occurence of the longest leaf in relation to the rank of l_t
//	a = -0.1053; b = -0.0116; // shape parameters of the bell curve
//	ptnLength = L_max*exp(a*pow(rank-l_pk,2) + b*pow(rank-l_pk,3)); // bell curve is a better fit especially when L_max varies

	//for beta fn calibrated from JH's thesis for SP and KM varieties, 8/10/15, SK
	l_t = 1.64*totalLeaves; // totalLeaf # is normalized to be 1.0, Jennifer Hsiao's thesis (2015);
	l_pk = 0.88 * totalLeaves; // occurence of the longest leaf in relation to the rank of l_t
	ptnLength = dv->beta_fn(rank, L_max, l_pk, l_t);
	set_growthDuration(ptnLength/elongRate); // shortest growth (linear phase) duration in physiological time when grown under constant optimal T
//	set_prolificDuration(ptnLength/elongRate); //longest possible prolific period in physiological time  under constant optimal T
//	set_agingDuration(ptnLength/agingRate); // shortest growth (linear phase) duration in physiological time when grown under constant optimal T

    phase1Delay = 0; //max(0.0, rank); //Fournier's value : -5.16+1.94*rank;
	double W_max = L_max*WLRATIO;
	double LA_max = L_max*W_max*A_LW;
//	ptnArea = ptnLength*ptnLength*WLRATIO*A_LW;
	ptnArea = 0.639945 + 0.954957*ptnLength + 0.005920*ptnLength*ptnLength; // see JH's thesis
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

	if (appeared)
	{
		elongAge += dv->beta_fn(T, 1.0, dv->get_Topt(), dv->get_Tceil())*dD; // Todo: implement Parent and Tardieu (2011, 2012) approach for leaf elongation in response to T and VPD, and normalized at 20C, SK, Nov 2012
		// elongAge indicates where it is now along the elongation stage or duration. duration is determined by totallengh/maxElongRate which gives the shortest duration to reach full elongation in the unit of days.

		if (!mature) {
			double t = min(t_e, elongAge);
			length = max(0.0, ptnLength*(1.0 + (t_e-t)/(t_e-t_pk))*pow(t/t_e, (t_e/(t_e-t_pk))));
			double dL = elongRate*max(0.0, (t_e-t)/(t_e-t_pk)*pow(t/t_pk,t_pk/(t_e-t_pk)))*dD;
			length += dL;
			length = min(length, ptnLength);
			width = length*WLRATIO;
			//area = length*width*A_LW;
			area = 0.639945 + 0.954957*length + 0.005920*length*length; // from JH's thesis
			if (length >= ptnLength || dL <= 0.0) {
				mature = true;
				set_GDD2mature(get_physAge());
				expanding = false;
			} else {
				expanding = true;
			}
		}
	}
	return;
}

void CLeaf::senescence(CDevelopment * dv)
{
	double dD = dv->get_initInfo().timeStep/MINUTESPERDAY;
	double T = dv->get_Tcur();
	double T_opt = dv->get_Topt();
	double T_ceil = dv->get_Tceil();
	double stayGreen = dv->get_initInfo().stayGreen; // stay green for this value times growth period after peaking before senescence begins

	// Beta function didn't work well for tracking greenness because of supraoptimal temperature
	// elongAge is based on Beta, physAge is based on GDD
	// (2016-10-24: KDY, SK, JH)
	//if (mature && elongAge >= get_growthDuration()*stayGreen)
	if (mature && get_physAge() >= get_GDD2mature()*stayGreen)
	{
		senescing = true;
	}

	if (senescing && !dead)
	{
		double Q10 = 2.0;
		double q10fn = pow(Q10, (T - T_opt)/10);
		double agingRate = elongRate;
		double dL = q10fn*agingRate*dD; // aging rate (lengthwise) per day in refernece to elongation rate at T_opt adjusted by stayGreen trait
		// a peaked fn like beta fn not used here because aging should accelerate with increasing T not slowing down at very high T like growth,
		// instead a q10 fn normalized to be 1 at T_opt is used
		double dA = (dL/length)*area; //leaf area aging rate;
		senescentArea += dA;
		if (senescentArea >= area) {senescentArea = area; dead = true;}
	}
	else if (dead && get_physAge() >= get_GDD2mature())
	{

		dropped = true;
	}

	return;
}


double CLeaf::get_maturity() const
{
	double potential_length = this->get_potentialLength();
	if (potential_length > 0) {
		double maturity = this->get_length() / potential_length;
		return min(maturity, 1.0);
	} else {
		return 0;
	}
}

CLeaf::~CLeaf() {}
