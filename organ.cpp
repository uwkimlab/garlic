#pragma once
#include "stdafx.h"
#include "..\garlic\organ.h"
#define CO2_MW 44.0098
#define C_MW 12.011
#define CH2O_MW 30.03
#define C_CONC 0.45



COrgan::COrgan()
{
	age = physAge = mass = 0;
	CH2O=N=0;
	temperature = 25;
	growthDuration=10;
	longevity=50;
	GDD = NULL;
	GDD = new CThermalTime();
}

COrgan::COrgan(const TInitInfo& info)
{
	initInfo = info;
	temperature=25.0;
	CH2O=N=0;
	age = physAge = mass = 0;
	growthDuration=10;
	longevity=50;
	GDD = NULL;
	GDD = new CThermalTime();
}

COrgan::~COrgan()
{
	if (GDD != NULL) delete GDD;
}

void COrgan::initialize()
{
	if (GDD == NULL) GDD = new CThermalTime();
	GDD->initialize(initInfo.timeStep);
}
void COrgan::update()
{
	GDD->add(temperature);
	age = GDD->get_actualAge();
	physAge=GDD->get_sum();
	mass = __max(0.0, CH2O*(C_MW/CH2O_MW)/C_CONC); // C content = 45%, hard coded for now
}

void COrgan::import_CH2O(double dCH2O)
{
	CH2O += dCH2O;
	mass = __max(0.0, CH2O*(C_MW/CH2O_MW)/C_CONC); // C content = 45%, hard coded for now
}

void COrgan::import_N(double dN)
{
	N += dN;
}

void COrgan::respire()
// this needs to be worked on
// currently not used at all
{
	double Rm = 0.02; //maintenance respiration
	double Ka = 0.1; //growth respiration
	CH2O -= Ka*CH2O + Rm*CH2O;
}




