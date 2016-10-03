#pragma once
#ifndef _PLANT_H_
#define _PLANT_H_
#include "organ.h"
#include "nodalUnit.h"
#include "development.h"
#include "roots.h"
#include "bulb.h"
#include "scape.h"
#include <iostream>
#include <string>

struct TPartition
{
public:
    TPartition()
    {
        BBCH = _0; root = 0.0; shoot = 0.0; leaf = 0.0; sheath = 0.0; scape = 0.0; bulb = 0.0;
    }
    BBCH_code BBCH;
    double root, shoot, leaf, sheath, scape, bulb;
};

class CPlant
{
public:
	CPlant();
	CPlant(const TInitInfo&);
	~CPlant();

	CNodalUnit* get_nodalUnit() {return nodalUnit;}
	CBulb * get_bulb() {return bulb;}
	CScape * get_scape() {return scape;}
	CRoots * get_roots() {return roots;}

	CDevelopment * get_develop() {return develop;}

	int get_nodeNumber() {return nodeNumber;}
	int get_finalNodeNumber() {return finalNodeNumber;}

	double get_mass() {return mass;}
	double get_age() {return age;}
	double get_CH2O() {return CH2O;}
	double get_N() {return N;}
	double get_Pg() {return photosynthesis_gross;}
	double get_Pn() {return photosynthesis_net;}
	double get_assimilate() {return assimilate;}
	double get_ET() {return transpiration;}
	double get_tmpr() {return temperature;}
	double get_CH2O_pool() {return CH2O_pool;}
	double get_CH2O_reserve() {return CH2O_reserve;}
	double get_stalkMass() {return stalkMass;}
	double get_leafMass() {return leafMass;}
	double get_bulbMass() {return bulbMass;}
	double get_shootMass() {return shootMass;}
	double get_rootMass() {return rootMass;}
	string getNote() {return note;}

	TStage get_stage() {return stage;}

	void set_mass();
	void set_age(double x) {age=x;}
	void set_CH2O();
	void set_N();

	void update(const TWeather &);
	void update_mass();
	void calcGasExchange(const TWeather & weather);
	void calcMaintRespiration(const TWeather&);
    void set_partition(TPartition * part){partition = part;}

	double calcLeafArea();
	double calcGreenLeafArea();
	double calcPotentialLeafArea();
	double calcSenescentLeafArea();
	double calcPlantGreenLeafArea(); // empirical fit of plant green leaf area from SPAR 02 field exp

	void grow();
	void CH2O_allocation(const TWeather&);
	void writeNote(const TWeather &);

private:
	TInitInfo initInfo;
	CNodalUnit * nodalUnit;
	CBulb * bulb;
	CRoots * roots;
	CScape * scape;
	CDevelopment * develop;
	string note;
	int finalNodeNumber; //final number of nodes
	int nodeNumber; // currently initiated number of nodes
	double CH2O_pool; // shorterm C pool, g(CH2O)
	double CH2O_reserve; // longterm C pool
	double C_conc;
	double CH2O_demand;
	double CH2O_supply;
	double mass, seedMass, seed_reserve, stalkMass, leafMass, shootMass, rootMass, bulbMass; // this is redundant, but for convenience of access
	double maintRespiration;
	double sowingDay;
	double age;
	double CH2O; // total CH2O including structural and non-structural carbohydrates, g(CH2O)
    double CH2O_ns; //non-structural carbohydrates, g(CH2O)
    double mass_nsc; //biomass of non-structural carbohydrates, g(biomass)
	double N;
	double leafArea;
	double greenLeafArea;
	double senescentLeafArea;
	double potentialLeafArea;
	double photosynthesis_gross; // gros photosynthesis, umolCO2 m-2 s-1
	double photosynthesis_net; // gross photosynthesis, umolCO2 m-2 s-1
	double assimilate, wateruse; //assimilates flux, gCH2O per plant, water lost through transpiration in gH2O per plant per timestep
	double transpiration; //mmolH2O m-2 s-1
	double temperature;

	TStage stage;
    TPartition * partition;



};
#endif
