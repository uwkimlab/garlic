#pragma once
#ifndef _WEATHER_H_
#define _WEATHER_H_

struct TWeather
{
public:
	TWeather()
	{
		year=2000,jday=1, time=0.0, daytime = jday+time, CO2=370.0, airT=20.0, PFD=0.0, solRad=0.0,
		RH=50.0, wind=1.5, rain=0.0, dayLength=12.0, soilT = airT;
	}
	int year;
	int jday;
	double time, daytime;
	double CO2, airT, PFD, solRad, RH, wind, rain, dayLength, soilT;
};
#endif
