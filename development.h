#pragma once
#ifndef _DEVELOPMENT_H_
#define _DEVELOPMENT_H_
#include "weather.h"
#include "initinfo.h"
#include <iostream>
#include <string>
using namespace std;


struct TStage
{
public:
    TStage() { V = 0.0; R = 0.0;}
    double V, R;
};

struct TEvent
{
public:
	TEvent() {daytime = 0.0;  done=false;}
	double daytime; // daytime this event begins
	bool done;
};

class CDevelopment
{
public:
    CDevelopment();
	CDevelopment(const TInitInfo&);
	~CDevelopment();
	void setParms();
	double beta_fn(double t, double R_max, double t_opt, double t_ceil);
	double calcGDD(double);
	int update(const TWeather&);
	TInitInfo get_initInfo() {return initInfo;}
	int get_youngestLeaf() {return youngestLeaf;}
	int get_totalLeaves() {return (int) totLeafNo +1;}
	double get_phyllochronsFromFI() {return phyllochronsFromFI;} //FI: floral initiation
	double get_LvsAtFI() {return LvsAtFI;}
	double get_LvsInitiated(){return LvsInitiated;}
	double get_LvsAppeared(){return LvsAppeared;}
	double get_GDDsum() {return GDDsum;}
	double get_dGDD() {return dGDD;}
	double get_Tcur() {return T_cur;}
	double get_Topt() {return T_opt;}
	double get_Tceil() {return T_ceil;}
	double get_Tbase() {return T_base;}
	double get_Rmax_LIR() {return Rmax_LIR;}
	double get_DVS() {return DVS;}
	EPhase get_devPhase() {return devPhase;}
    BBCH_code get_BBCH() {return BBCH;}
    void set_devPhase(EPhase phase) {devPhase = phase;}
    double get_dt() {return dt;}
	bool Germinated() {return germination.done;}
	bool Emerged() {return emergence.done;}
	bool FlowerInitiated() {return floralInitiation.done;}
	bool BulbInitiated() {return bulbing.done;}
	bool ScapeAppeared() {return scapeAppear.done;}
	bool Flowered() {return flowering.done;}
	bool ScapeRemoved() {return scapeRemoval.done;}
	bool Matured() {return maturation.done;}
	string getNote() {return note;}
	TEvent germination;
	TEvent emergence;
	TEvent floralInitiation;
	TEvent scapeAppear; // spathe appearance
	TEvent flowering;
	TEvent scapeRemoval;
	TEvent bulbing;  // Bulb initation
    TEvent bulbiling; //Bulbil development on inflorescence
	TEvent maturation;

private:
	CDevelopment(const CDevelopment&); // supressing shallow copy constructor
	int GDD_rating;
	double dGDD; // delta GDD
	double dt; // timestep
	double GDDsum; // cumulative GDD from sowing with Tbase= 8.0 and Topt = 34 as in CERES
	double GDD_bulb; // cumulative GDD after bulbing begins
	double Rmax_LIR, Rmax_LTAR, Rmax_Germination, Rmax_Emergence;
	double T_base, T_opt, T_ceil, T_cur;
	double totLeafNo, juvLeafNo, LvsAtFI, phyllochronsFromFI; //number of total, juvenile (genetic coeff) lvs, and lvs appbulbed at floral initiation
	double DVS; //developmental stage: 0=emergence, 1=bulbing begins, 2=bulb maturity
	double minBulbingDays; //minimum bulbing period in days; theoretically observable when constantaly growing under optimal temperature for bulbing
	double GerminationRate, EmergenceRate, LvsInitiated, LvsAppeared, LvsExpanded, Scape, phyllochron;
	int initLeafNo,  youngestLeaf, curLeafNo;
	string note;
	TInitInfo initInfo;
    EPhase devPhase;
    BBCH_code BBCH;

};
#endif
