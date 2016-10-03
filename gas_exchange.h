#pragma once
double QuadSolnUpper (double a, double b, double c );
double QuadSolnLower (double a, double b, double c );
double minh(double fn1,double fn2, double theta);

class CGas_exchange
{
public:
	CGas_exchange(void);
	~CGas_exchange(void);

private:
  double PFD, R_abs, Tair, CO2, RH, wind, age, SLA, width, Press, N;
   void GasEx();
   void C3Photosynthesis();
   void C4Photosynthesis();
   void EnergyBalance();
   void getParms();
   double gsw();
   double gbw();
   double Es(double T);
   double Slope(double T);
public:
  void SetVal(double PFD, double Tair, double CO2, double RH, double wind, double Press, double width);
  void SetVal(double PFD, double solRad, double Tair, double CO2, double RH, double wind,  double Press, double width);
  struct tparms
  {
	  double TPU25, Vcm25, Jm25, Vpm25, Rd25, gamma25, EaVp, EaTPU, EaVc, Eaj, Sj, Hj, Ear, Eag, g0, g1;
  } Parms;
  double A_gross, A_net, ET, Tleaf, Ci, gs, gb, Rd, iter1, iter2;
};
