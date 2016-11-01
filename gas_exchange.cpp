/* Leaf Gas-Exchange Unit */
//
// This unit simulates garlic leaf gas-exchange characteristics
// based on a coupled model of photosynthesis-stomatal conductance-energy balance
// See Kim and Lieth (2003) Ann. Bot for details

#include "stdafx.h"
#include "gas_exchange.h"
#include <cmath>
#include <algorithm>
#include <stdlib.h>

using namespace std;

#define R 8.314  // idealgasconstant
#define maxiter 100
#define epsilon 0.97
#define sbc 5.6697e-8
#define scatt 0.15 //leaf reflectance + transmittance
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
inline double Square(double a) { return a * a; }
inline double Min(double a, double b, double c) {return (min(min(a,b),c));}

CGas_exchange::CGas_exchange()
{
}

CGas_exchange::~CGas_exchange()
{
}

void CGas_exchange::getParms()
{
//gas-exchange parameter estimates for hardneck garlic 'Japanese Mountain' from outdoor plot experiments conducted in Seattle, WA in 2012. Sep 2012. SK
	Parms.EaTPU     =       47100;
	Parms.EaVc     =        52157.3;
	Parms.Eaj      =        23997.6;
	Parms.Hj       =       200000;
	Parms.Sj       =          616.4;
	Parms.TPU25    =          16.03;
	Parms.Vcm25    =         108.4;
	Parms.Jm25     =         169.0;
	Parms.Rd25     =          1.08;
	Parms.gamma25  =         42.75; //CO2 compensation point in the absence of day respiration, value from Bernacchi (2001)
	Parms.Ear      =        49390;
	Parms.Eag      =        37830;
    Parms.g0 = 0.096;
    Parms.g1 =  6.824;
}


void CGas_exchange::GasEx(void)
{
    double Tleaf_old, Tleaf_new;
    int   iter=1;
	Tleaf = Tair;
    Tleaf_new = 0; Tleaf_old = Tair;
	Ci = 0.7*CO2;
	gb = gbw();
	gs = gsw();
    A_net = (CO2-Ci)/(1.57/gs+1.37/gb)*Press/100;
    while ((fabs(Tleaf_old -Tleaf_new)>0.001) && (iter < maxiter))
    {
	    Tleaf_old = Tleaf;
	    C3Photosynthesis();
		EnergyBalance();
		Tleaf_new = Tleaf;
		iter2 =++iter; //iter=iter+1, iter2=iter;
    }
}


void CGas_exchange::C3Photosynthesis(void)    //Incident PFD, Air temp in C, CO2 in ppm, RH in percent
{
    const double    f = 0.15;             //spectral correction
    const double    O = 210;             // gas units are mbar
    const double    theta = 0.7;
//    const double    scatt = 0.15;
    const double       Kc25 = 404.9;    //* Michaelis constant of rubisco for CO2 of C3 plants, ubar, from Bernacchi et al. (2001) */
    const double       Ko25 = 278.4;    //* Michaelis constant of rubisco for O2, mbar, from Bernacchi et al., (2001) */
    const long      Eac = 79430;   // Activation energy for Kc, Bernacchi (2001)
	const long      Eao = 36380;   // Activation energy for Ko, Bernacchi (2001)
    const double    beta = 0.99; //* smoothing factor */

	double Kc, Ko, Km, Ia, I2, Tk, TPU, Jmax, Vcmax,  gamma, Rd, J, Ac, Aj, Ap, An, Ag, Ca, P;
	double Ci_next, Ci_last, newCi, d_newCi, d_A;
    int iter;

//* Light response function parameters */
	Ia = PFD*(1-scatt);    //* absorbed irradiance */
	I2 = Ia*(1-f)/2;    //* light (PFD) effectively absorbed by PSII */

	Tk = Tleaf + 273.0;
	P  = Press/100;
	Ca = CO2*P; //* conversion to partial pressure */
	Kc = Kc25*exp(Eac*(Tleaf-25)/(298*R*(Tleaf+273)));
	Ko = Ko25*exp(Eao*(Tleaf-25)/(298*R*(Tleaf+273)));
	Km = Kc*(1+O/Ko); //* effective M-M constant for Kc in the presence of O2 */
	Rd = Parms.Rd25*exp(Parms.Ear*(Tleaf-25)/(298*R*(Tleaf+273)));
	TPU = Parms.TPU25*exp(Parms.EaTPU*(Tleaf-25)/(298*R*(Tleaf+273)));
	Vcmax = Parms.Vcm25*exp(Parms.EaVc*(Tleaf-25)/(298*R*(Tleaf+273)));
	Jmax = Parms.Jm25*exp(((Tk-298)*Parms.Eaj)/(R*Tk*298))*(1+exp((Parms.Sj*298-Parms.Hj)/(R*298)))/(1+exp((Parms.Sj*Tk-Parms.Hj)/(R*Tk)));
	gamma = Parms.gamma25*exp(Parms.Eag*(Tleaf-25)/(298*R*(Tleaf+273)));

	Ci_next = Ci;
	iter = 1;
	Ci_last = 0;
	while ((fabs(Ci_next-Ci_last) > 0.01) && (iter < maxiter))
	{
		Ac = (Vcmax*(Ci-gamma))/(Ci+Km);
		J =  (((I2 + Jmax) - sqrt(Square(I2+Jmax) - 4*I2*(Jmax)*theta)) / (2*theta)) ;
		Aj = J*(Ci-gamma)/(4*(Ci+2*gamma));
		Ap = 3*TPU;

		if (((Square(I2+Jmax) - 4*I2*Jmax*theta) > 0) && (Ci > gamma))
		{
			An = min(min(Ac,Aj),Ap)-Rd;
			if (min(Ac,min(Aj,Ap)) == Ap) d_A = 0;
			else if (min(Ac,min(Aj,Ap)) == Ac) d_A = Vcmax*(Km+gamma)/(Square(Ci+Km));
			else d_A = 3*J*gamma/(4*Square(Ci+2*gamma));
		}
		gs = gsw();
		newCi = min(max(0.0, Ca - An*(1.6/gs + 1.37/gb)*P), 2*Ca);
		d_newCi = -d_A*(1.6/gs+1.37/gb)*P;
		Ci_next = Ci-(newCi-Ci)/(d_newCi-1);
		Ci_last = Ci;
		if (Ci_next > 0) Ci = Ci_next; else Ci = Ci_last;
		iter++;
	}
	iter1 = iter;
	gs = gsw();
	if (Tair > 0.0) A_net = An; else A_net = Rd;
	A_net = An;
	A_gross = max(0.0, A_net + Rd); // gets negative when PFD = 0, Rd needs to be examined, 10/25/04, SK
}


void CGas_exchange::EnergyBalance()
// see Campbell and Norman (1998) pp 224-225
// because Stefan-Boltzman constant is for unit surface area by denifition,
// all terms including sbc are multilplied by 2 (i.e., gr, thermal radiation)
{
    const long lamda = 44000;
    const double psc = 6.66e-4;
    const double Cp = 29.3; // thermodynamic psychrometer constant and specific hear of air
	double gha, gv, gr, ghr, psc1, Ea, VPD, thermal_air, Ti, Ta;
    Ta = Tair;
    Ti = Tleaf;
    gha = gb*(0.135/0.147);  // heat conductance, gha = 1.4*.135*sqrt(u/d), u is the wind speed in m/s}
    gv = gs*gb/(gs+gb);
    gr = 2*(4*epsilon*sbc*pow(273+Ta,3)/Cp); // radiative conductance, 2 account for both sides
    ghr = gha + gr;
    thermal_air = 2*epsilon*sbc*pow(Ta+273,4); // emitted thermal radiation
    psc1 = psc*ghr/max(0.01, gv); // apparent psychrometer constant
    VPD = Es(Ta)*(1-RH); // vapor pressure deficit
    Ea = Es(Ta)*RH; // ambient vapor pressure
//    Tleaf = Ta + (psc1/(Slope(Ta) + psc1))*((R_abs-thermal_air)/(ghr*Cp)-VPD/(psc1*Press)); //eqn 14.6b linearized form using first order approximation of Taylor series
	Tleaf = Ta + (R_abs-thermal_air-lamda*gv*VPD/Press)/(Cp*ghr+lamda*Slope(Ta)*gv);
	//    ET = gv*(Es(Tleaf)-Ea)/Press*1000; // in mmol m-2 s-1
    ET = max(0.0, 1000*gv*((Es(Tleaf)-Ea)/Press));
    // accounting for additional transp. because of mass flow, see von Caemmerer and Farquhar (1981)
}



double CGas_exchange::gsw()  // stomatal conductance for water vapor in mol m-2 s-1
{
	double Ds, aa, bb, cc, hs, Cs, gamma, tmp;
	gamma = Parms.gamma25*exp(Parms.Eag*(Tleaf-25)/(298*R*(Tleaf+273)));
    Cs = CO2 - (1.37*A_net/gb); // surface CO2 in mole fraction
	if (Cs == gamma) Cs = gamma + 1;
    aa = Parms.g1*A_net/Cs;
    bb = Parms.g0+gb-(Parms.g1*A_net/Cs);
    cc = (-RH*gb)-Parms.g0;
    hs = QuadSolnUpper(aa,bb,cc);
	if (hs > 1) hs = 1; else if (hs <= 0.10) hs = 0.10;    //preventing bifurcation
    Ds = (1-hs)*Es(Tleaf); // VPD at leaf surface
    tmp = (Parms.g0+Parms.g1*(A_net*hs/Cs));
	if (tmp < Parms.g0) return Parms.g0;
	else return tmp;
}

double CGas_exchange::gbw(void)
{
	const double stomaRatio = 1.0; // maize is an amphistomatous species, assume 1:1 (adaxial:abaxial) ratio.
	double ratio;
	double d;
	ratio = Square(stomaRatio+1)/(Square(stomaRatio)+1);
    d = width*0.72; // characteristic dimension of a leaf, leaf width in m
  //  return 1.42; // total BLC (both sides) for LI6400 leaf chamber
    return (1.4*0.147*sqrt(max(0.1,wind)/d))*ratio;
	// multiply by 1.4 for outdoor condition, Campbell and Norman (1998), p109
	// multiply by ratio to get the effective blc (per projected area basis), licor 6400 manual p 1-9
}

double CGas_exchange::Es (double T) //Campbell and Norman (1998), p 41 Saturation vapor pressure in kPa
{
	double tmp = T;
    return (0.611*exp(17.502*tmp/(240.97+tmp)));
}

double CGas_exchange::Slope(double T) // slope of the sat vapor pressure curve: first order derivative of Es with respect to T
{
	const double b= 17.502; const double c= 240.97;
	double tmp = T;
	return (Es(tmp)*(b*c)/Square(c+tmp)/Press);
}

void CGas_exchange::SetVal(double PFD, double SolRad, double Tair, double CO2, double RH, double wind,  double Press, double width)
{

	this->PFD = PFD;
    double PAR = (PFD/4.55);
    double NIR = SolRad - PAR;
    this->R_abs = (1-scatt)*PAR + 0.15*NIR + 2*(epsilon*sbc*pow(Tair+273,4)); // times 2 for projected area basis
	// shortwave radiation (PAR (=0.85) + NIR (=0.15) solar radiation absorptivity of leaves: =~ 0.5
    this->CO2 = CO2;
    this->RH = min(100.0, max(RH, 10.0))/100;
    this->Tair = Tair;
    this->width = width;
	this->wind = wind;
    this->Press = Press;
    getParms();
    GasEx();
}

void CGas_exchange::SetVal(double PFD, double Tair, double CO2, double RH, double wind,  double Press, double width)
{
	this->PFD = PFD;
    double PAR = (PFD/4.55);
    double NIR = PAR; // If total solar radiation unavailable, assume NIR the same energy as PAR waveband
    this->R_abs = (1-scatt)*PAR + 0.15*NIR + 2*(epsilon*sbc*pow(Tair+273,4)); // times 2 for projected area basis
	// shortwave radiation (PAR (=0.85) + NIR (=0.15) solar radiation absorptivity of leaves: =~ 0.5
    this->CO2 = CO2;
    this->RH = min(100.0, max(RH, 10.0))/100;
    this->Tair = Tair;
    this->width = width;
	this->wind = wind;
    this->Press = Press;
    getParms();
    GasEx();
}

double minh(double fn1,double fn2, double theta)
{
	double x;
    x = ((fn1+fn2)*(fn1+fn2)-4*theta*fn1*fn2);
    if (x<0) return min(fn1,fn2);
    else if (theta==0.0) return fn1*fn2/(fn1+fn2);
    else return ((fn1 + fn2) - sqrt(x))/(2*theta); // hyperbolic minimum
}

double QuadSolnUpper (double a, double b, double c )
{
    if (a==0) return 0;
	else if ((b*b - 4*a*c) < 0) return -b/a;   //imaginary roots
    else  return (-b+sqrt(b*b-4*a*c))/(2*a);
}

double QuadSolnLower (double a, double b, double c )
{
    if (a==0) return 0;
	else if ((b*b - 4*a*c) < 0) return -b/a;   //imaginary roots
    else  return (-b-sqrt(b*b-4*a*c))/(2*a);
}
