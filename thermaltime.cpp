#include "StdAfx.h"
#include "thermaltime.h"


CThermalTime::CThermalTime(void)
{
	Tcur = 25.0;
	Tbase = 4.0;
	Topt = 20.0;
	Tmax = 40.0;
	actualAge = sum = 0.0;
	dTmpr = 0.0;
	timeStep = 60.0;
}

void CThermalTime::initialize(double step)
{
	Tcur = 0.0;
	timeStep = step;
	actualAge = sum = 0.0;
	add(Tcur);
}

void CThermalTime::add(double x)
{
	Tcur = x;
	double dD = timeStep/MINUTESPERDAY;
	if (Tcur <= Tbase||Tcur >= Tmax)
	{
		dTmpr = 0.0;
	}
	else
	{
		dTmpr = (Tcur-Tbase);
	}

	sum += (dTmpr*dD);
	actualAge += dD;
}

void CThermalTime::update(double Tmpr, double step)
{
	timeStep = step;
	add(Tmpr);
}

CThermalTime::~CThermalTime(void)
{
}
