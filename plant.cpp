#pragma once
#include "stdafx.h"
#include "plant.h"
#include "gas_exchange.h"
#include "lightenv.h"
#include "radiation.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#define PRIMORDIA 4   //leaf primordia initiated in a clove, Meredith (2008)
#define CO2_MW 44.0098
#define C_MW 12.011
#define CH2O_MW 30.03

using namespace std;

CPlant::CPlant()
{
	nodalUnit = NULL;
	bulb = NULL;
	roots = NULL;
	scape = NULL;
	develop = NULL;
	seedMass = 3.0; // seed weight g/seed
	C_conc = 0.45; // 45% C
	seed_reserve = seedMass*C_conc;
	CH2O = (mass*C_conc)/C_MW*CH2O_MW;
	CH2O_pool = 0.0; // Short-term CH2O pool
	CH2O_reserve =0.0; // Long-term CH2O pool
	CH2O_demand = CH2O_supply = 0.0;
	maintRespiration = 0.0;

	sowingDay = 1.0;
	age = 0.0;
	N = 0.0;
	mass = 0.0; //initial plant mass, start from zero excluding the propagule
	nodeNumber = 0;
	finalNodeNumber = 0;
	photosynthesis_net=photosynthesis_gross = transpiration = assimilate = wateruse = temperature = 0.0;
	leafArea =greenLeafArea = senescentLeafArea = potentialLeafArea = 0.0;
	stalkMass = leafMass = bulbMass = rootMass = shootMass= 0.0;
}

CPlant::CPlant(const TInitInfo& info )
{
	nodalUnit = NULL;
	bulb = NULL;
	roots = NULL;
	scape = NULL;
	develop = NULL;
	seedMass = 3.0;
	// seed garlic (clove) weight g/seed, Korean Mountain used for 2011 experiment had an average seed mass (dry weight verified by Jigs) of 2.87g. Garlic cloves have about 2:1 ratio of moisture:dry matter.
	// 7-10-2013, SK
	C_conc = 0.45; // 45% C
	seed_reserve = seedMass*C_conc;
	CH2O = (mass*C_conc)/C_MW*CH2O_MW;; // For assimilates accounting in the form of carbohydrates 
	CH2O_pool = 0.0;
	CH2O_reserve = 0.0;
	CH2O_demand = CH2O_supply = 0.0;
	maintRespiration = 0.0;

	sowingDay = 1.0;
	mass = 0.0; //initial plant mass, start from zero excluding the propagule
	age = 0.0;
	N = 0.0;
	initInfo = info;
	roots = new CRoots();
	bulb = new CBulb();
	scape = new CScape();
	develop = new CDevelopment(initInfo);
	nodalUnit = new CNodalUnit[initInfo.genericLeafNo+10]; // create enough leaf nodes for now, to be replaced by dynamic collection
	for (int i=0; i <= PRIMORDIA; i++) // leaf[0] is a coleoptile
	{
		nodalUnit[i].initialize(i, develop);
       	nodeNumber = i;
	}
	finalNodeNumber = info.genericLeafNo;
	photosynthesis_net =photosynthesis_gross = transpiration = assimilate =  0.0;
	leafArea =greenLeafArea = senescentLeafArea = potentialLeafArea = 0.0;
	stalkMass = leafMass = bulbMass = rootMass = shootMass= 0.0;
	temperature = develop->get_Tcur();
}

CPlant::~CPlant() 
{
	if (nodalUnit != NULL) delete [] nodalUnit;
	if (roots != NULL) delete roots;
	if (develop != NULL) delete develop;
	if (bulb != NULL) delete bulb;
	if (scape != NULL) delete scape;
}

void CPlant::update(const TWeather & weather)
{
	develop->update(weather);
	finalNodeNumber = develop->get_youngestLeaf();
	double T_effect = 0.0;

	if (!develop->Germinated())
	{
		temperature = develop->get_Tcur();
		return;
	}

	else if(develop->get_LvsAppeared() <= 1.0)
	{
		temperature = develop->get_Tcur();
		for (int i = 1; i <= develop->get_LvsInitiated() ; i++)
		{
			if(!nodalUnit[i].isInitiated())
			{
				nodalUnit[i].initialize(i,develop);
                nodeNumber = i;
			}
			else
			{
				nodalUnit[i].update(develop);
			}
		}
		seed_reserve = seedMass*C_conc/C_MW*CH2O_MW; //CH2O reserved in the propagule (e.g., starch in endosperm of seeds)
		T_effect = develop->beta_fn(develop->get_Tcur(), 1.0, develop->get_Topt(), develop->get_Tceil());
		double dCH2O = seed_reserve*T_effect*develop->get_dt()*(1.0/50.0); // assume it takes 50 days to exhaust seed reserve at Topt before emergence
		// if divide by integer fraction (i.e., 1/15), it automatically typcast dCH2O to integer.
		CH2O_pool += dCH2O;
		calcPotentialLeafArea();
		calcLeafArea();
		calcSenescentLeafArea();
		calcGreenLeafArea();
		calcGasExchange(weather);
		calcMaintRespiration(weather);
		CH2O_allocation(weather);
		seedMass = (seed_reserve)*(C_MW/CH2O_MW)/C_conc;
	

			// from germination to emergence, C supply is from the seed
		return;
	}

	else // if(!develop->Matured())
	{
		for (int i = 1; i <= develop->get_LvsInitiated(); i++)
		{
			if(!nodalUnit[i].isInitiated())
			{
				nodalUnit[i].initialize(i,develop);
                nodeNumber = i;
			}
			else
			{
				nodalUnit[i].update(develop);
			}
		}
		calcPotentialLeafArea();
		calcLeafArea();
		calcSenescentLeafArea();
		calcGreenLeafArea();
		calcGasExchange(weather);
		if (weather.time < 0.0)
		{
//			CH2O_reserve += __max(0, CH2O_pool);
//			CH2O_pool = 0.0; //reset shorterm C_poot to zero at midnight, needs to be more mechanistic
		}
		else
		{
			double dCH2O;
			if (seed_reserve >= 0.0)
			{
				dCH2O = seed_reserve*T_effect*(1.0/10.0)*develop->get_dt(); //10% available
				seed_reserve -= dCH2O;
			}
			else
			{
				dCH2O = 0.0;
				seed_reserve = 0.0;
			}
	
            CH2O_pool += (assimilate + dCH2O); // gCH2O per plant
			calcMaintRespiration(weather);
			CH2O_allocation(weather);
		}
		set_mass();
 	}
}

void CPlant::set_mass()
//TODO: allocate biomass into individual organs, currently it is allocated as a bulk to leaf, stem, and so on
//so individual leaf doesn't have mass but the first or the last one has it all
{
	double m = 0;

	double agefn = (greenLeafArea/potentialLeafArea); // as more leaves senesce living leaf biomass should go down
	{
		double leaf = 0.0;
		// apply leaf and sheath senescence and degradation. this is to mimic the loss of structural parts
        leaf = this->get_nodalUnit()->get_leaf()->get_mass();
		double sheath = this->get_nodalUnit()->get_stem()->get_mass();
		this->get_nodalUnit()->get_leaf()->set_mass(leaf*agefn); 
		this->get_nodalUnit()->get_stem()->set_mass(sheath*agefn); 
	}

	stalkMass = this->get_nodalUnit()->get_stem()->get_mass() + this->get_scape()->get_mass() + (CH2O_pool+CH2O_reserve)*(C_MW/CH2O_MW)/C_conc; 
	leafMass = this->get_nodalUnit()->get_leaf()->get_mass();
	rootMass = this->get_roots()->get_mass();
	bulbMass = this->get_bulb()->get_mass();
	shootMass = stalkMass + leafMass + bulbMass;
	mass = shootMass + rootMass;
	return;
}


double CPlant::calcLeafArea()
{
	double area = 0.0; 
	double dL = 0.0;
	for (int i = 0; i <= develop->get_LvsAppeared(); i++)
	{
		dL = nodalUnit[i].get_leaf()->get_area();
		area += dL ;
	}
	leafArea = area;
	return area;
}

double CPlant::calcGreenLeafArea()
{
	double area = 0.0;
	for (int i = 0; i <= develop->get_LvsAppeared(); i++)
	{
		area += nodalUnit[i].get_leaf()->get_greenArea();
	}
	greenLeafArea = area;
	return area;
}

double CPlant::calcSenescentLeafArea()
{
	double area = 0.0;
	for (int i = 0; i <= develop->get_LvsAppeared(); i++)
	{
		area += nodalUnit[i].get_leaf()->get_senescentArea();
	}
	senescentLeafArea = area;
	return area;
}

double CPlant::calcPotentialLeafArea()
{
	double area = 0.0;
	for (int i = 0; i <= develop->get_LvsAppeared(); i++)
	{
		area += nodalUnit[i].get_leaf()->get_potentialArea();
	}
	potentialLeafArea = area;
	return area;
}

void CPlant::calcGasExchange(const TWeather & weather)
{
	const double tau = 0.50; // atmospheric transmittance, to be implemented as a variable
	const double LAF = 0.7; // leaf angle factor for garlic canopy, from Rizzalli et al. (2002),  X factor in Campbell and Norman (1998)
//	const double plantsPerMeterSquare = 8.0;
	const double leafwidth = 1.5; //to be calculated when implemented for individal leaves
	const double atmPressure= 100.0; //kPa, to be predicted using altitude
	double activeLeafRatio = greenLeafArea/leafArea;
	double LAI = greenLeafArea*initInfo.plantDensity/(100.0*100.0);

	CGas_exchange * sunlit = new CGas_exchange();
	CGas_exchange * shaded = new CGas_exchange();

// setPFD(weather.jday, weather.time, initInfo.latitude, initInfo.longitude, initInfo.altitude, tau, weather.PFD, LAI, LAF);
// setRad(int jday, double tm, double Lati, double Longi, double I0, double LAI, double LAF, bool IsObsPFD);
//TODO: lightenv.dll needs to be translated to C++. It slows down the execution, 3/16/05, SK
	radTrans(weather.jday, weather.time, initInfo.latitude, initInfo.longitude, weather.solRad, LAI, LAF);
//	radTrans2(weather.jday, weather.time, initInfo.latitude, initInfo.longitude, weather.solRad, weather.PFD, LAI, LAF);
	sunlit->SetVal(sunlitPFD(), weather.airT, weather.CO2, weather.RH, 
		        weather.wind, atmPressure, leafwidth);
	shaded->SetVal(shadedPFD(), weather.airT, weather.CO2, weather.RH, 
		        weather.wind, atmPressure, leafwidth);
	photosynthesis_gross = (sunlit->A_gross*sunlitLAI() + shaded->A_gross*shadedLAI());//plantsPerMeterSquare;
	photosynthesis_net = (sunlit->A_net*sunlitLAI() + shaded->A_net*shadedLAI());
//	photosynthesis_net = sunlit->A_net;
	transpiration = (sunlit->ET*sunlitLAI() + shaded->ET*shadedLAI());//plantsPerMeterSquare;
//	transpiration = sunlit->ET;
//	photosynthesis_gross = sunlit->A_gross*LAI;
//	photosynthesis_net = sunlitPFD();
//	transpiration = sunlit->ET*LAI;
	temperature = sunlit->Tleaf;
	assimilate = (photosynthesis_gross*CH2O_MW/1.0e6)*(60.0*initInfo.timeStep)/initInfo.plantDensity; // gCH2O per plant

	delete sunlit;
	delete shaded;
}

void CPlant::CH2O_allocation(const TWeather & w)
// this needs to be f of temperature, source/sink relations, nitrogen, and probably water
// a valve function is necessary because assimilates from CPool cannot be dumped instantanesly to parts
// this may be used for implementing feedback inhibition due to high sugar content in the leaves
// The following is based on Grant (1989) AJ 81:563-571
{
   double b1=2.325152587; // Normalized (0 to 1) temperature response fn parameters, Pasian and Lieth (1990)
   double b2=0.185418876; // I'm using this because it can have broad optimal region unlike beta fn or Arrhenius eqn
   double b3=0.203535650;
   const double Td = 48.6;
   double shootPart = 0.0;
   double rootPart = 0.0;
   double leafPart = 0.0;
   double sheathPart = 0.0;
   double scapePart = 0.0;
   double reservePart = 0.0;
   double tunicPart = 0.0;
   double basalplatePart = 0.0;
   double clovePart = 0.0;

   double g1=1+exp(b1-b2*w.airT);
   double g2=0.0;
   if (w.airT<Td) g2=1-exp(-b3*(Td-w.airT));

   double tmprEffect = develop->beta_fn(develop->get_Tcur(), 1.0, develop->get_Topt(), develop->get_Tceil()); //g2/g1;
//   double tmprEffect = g2/g1;

   double grofac = 1.0/(60/initInfo.timeStep); // translocation limitation and lag, assume it takes 1 hour to complete (1=no lag, same as the time step), 0.2=5 hrs
   // this is where source/sink (supply/demand) valve can come in to play
   // 0.2 is value for hourly interval, Grant (1989)
	double scale = 0.0; // see Grant (1989), #of phy elapsed since TI/# of phy between TI and silking
	scale = develop->get_DVS();

	//		if (w.time == 0.0) std::cout << scale << endl;
	double CH2O_recharge;
    CH2O_supply = CH2O_recharge = 0.0;
	const double C_min = 1.0;


    if (CH2O_pool >= maintRespiration)
	{
        CH2O_supply = __max(CH2O_pool*tmprEffect*grofac, maintRespiration); 
		//temprEffect and grofac are valve functions that determin the rate of assimilate transport to each part.
		//Any remaining assimilates not used to supply CH2O to other parts need to remain in leaves as short-term soluble C
		//Alternatively, this could be dumpted to long-term reserve first (but this would need another valve), and still remaining CH2O can stay in the leaves, SK, 7-16-13
        CH2O_pool = __max(0.0, CH2O_pool - CH2O_supply);
	}
	else
	if (CH2O_reserve >= 0.0)
	{
		// conversion and translocation from long term reserve should be less efficient, apply nother glucose conversion factor
		// translocation from the soluble sugar reserve
		// deplete CH2O_pool first and recharge

		CH2O_recharge = __max(CH2O_reserve* tmprEffect*grofac, 0.0);
		CH2O_pool += CH2O_recharge; 
		CH2O_reserve = __max(0.0, CH2O_reserve - CH2O_recharge);
        CH2O_supply = __min(CH2O_pool*tmprEffect*grofac, maintRespiration);
        CH2O_pool = __max(0.0, CH2O_pool - CH2O_supply);

	}
	else
	{
	    CH2O_pool = 0.0;
		CH2O_reserve = 0.0;
		CH2O_supply = 0.0;
	}
	double Fraction = __min(0.925, 0.7 + 0.3*scale);
    // convFactor = 1/1.43; // equivalent to Yg, Goudriaan and van Laar (1994)
	double Yg = 0.7; // synthesis efficiency, ranges between 0.7 to 0.76 for corn, see Loomis and Amthor (1999), Grant (1989), McCree (1988) -- SK, Nov 2012
	shootPart = __max(0,Yg*(Fraction*(CH2O_supply-maintRespiration))); // gCH2O partitioned to shoot
	rootPart = __max(0,Yg*((1-Fraction)*(CH2O_supply-maintRespiration))); // gCH2O partitioned to roots

   if (!develop->Germinated())
   {
	   return;
   }
   else if (!develop->FlowerInitiated())
   {
	   leafPart = shootPart*0.9;
	   sheathPart = shootPart*0.1;
	   scapePart = 0.0;
	   reservePart = 0.0;
	   tunicPart = 0.0;
	   basalplatePart = 0.0;
	   clovePart = 0.0;
   }
   else if (!develop->Bulbing())
   {
	   leafPart = shootPart*0.85;
	   sheathPart = shootPart*0.1;
	   scapePart = shootPart*0.05;
	   reservePart = 0.0;
	   tunicPart = 0.0;
	   basalplatePart = 0.0;
	   clovePart = 0.0;
   }
   else if (!develop->ScapeAppeared()) // assume that 72% of C supply goes to leaf growth until bolting with spathe 
   {
	    leafPart = shootPart*__max(0.9*0.8,0);;
   	    sheathPart = shootPart*__max(0.9*0.1,0);
	    scapePart = shootPart*__max(0.9*0.1,0);
		basalplatePart = shootPart*__max(0.1*0.20,0);
		clovePart = shootPart*__max(0.1*0.7,0);
		tunicPart = shootPart*__max(0.1*0.05,0);
		reservePart = shootPart*__max(0.1*0.05,0); 
		//Eventually, allocation to long-term reserve should be done after partitioning to all organs, this means each organ needs it own demand fn with potential sink capacity and a valve function (rate equation), 7-16-13, SK
		//alternatively, use valve functions only including a valve regulating the transport rate to long-term reserve, and whatever remaining in short-term pool to be stored in leaves
		//for now LT reserve also gets a min portion of assimilates, the way partitioning is done here now is rudimentary and teleonomic without empirical data to support from specifically designed experiments
		//this method is referred to as allometric method linked with dvs as described in Gourdriaan and van Laar (1996) and Penning de Vries and van Laar (1982)

   }
   else if (!develop->ScapeRemoved())
   {
		leafPart = shootPart*__max(0.6*0.2,0);
		sheathPart = shootPart*__max(0.6*0.1,0);
		scapePart = shootPart*__max(0.6*0.7,0);
		reservePart = shootPart*__max(0.6*0.0,0);

		basalplatePart = shootPart*__max(0.4*0.20,0);
		clovePart = shootPart*__max(0.4*0.7,0);
		tunicPart = shootPart*__max(0.4*0.05,0);
   }
   else if (!develop->Matured())
   {
	   const int maxCloveNo = 18; // assumed maximum cloves per bulb
	   double maxCloveFillRate = 0.1*(initInfo.timeStep/(24*60)); // max clove filling rate = 0.1g clove-1 day-1, assumed
	   CH2O_demand = maxCloveNo*maxCloveFillRate*tmprEffect;
       clovePart = shootPart*0.6;
	   basalplatePart = shootPart*0.05;
	   tunicPart = shootPart*__max(0.05,0);

	   leafPart = shootPart*__max(0.1,0);
	   sheathPart = shootPart*__max(0.05,0);
	   scapePart = shootPart*__max(0.05,0);
	   reservePart = shootPart*0.1;

   }

   else
   {

   }
   double pseudostemPart = sheathPart + reservePart; // Assumes part of pseudostem serves as long-term CH2O reserve. In reality, this should include clove storage, and degradable portion of other structural parts, 7-15-13 SK
   double bulbPart = clovePart + basalplatePart + tunicPart;

   this->get_nodalUnit()->get_leaf()->import_CH2O(leafPart); // not working for each leaf until implementing an array iteration method or use it as public member
   this->get_nodalUnit()->get_stem()->import_CH2O(pseudostemPart);
   this->get_scape()->import_CH2O(scapePart);
   this->CH2O_reserve += __max(0.0, reservePart);
   this->get_roots()->import_CH2O(rootPart);
   this->get_bulb()->import_CH2O(bulbPart);

   double partSum = pseudostemPart + bulbPart + leafPart + scapePart; // checking the balance if sums up to shootPart
}


void CPlant::calcMaintRespiration(const TWeather & w)
// based on McCree's paradigm, See McCree(1988), Amthor (2000), Goudriaan and van Laar (1994)
// units very important here, be explicit whether dealing with gC, gCH2O, or gCO2
{
	const double Q10 = 2.0; // typical Q10 value for respiration, Loomis and Amthor (1999) Crop Sci 39:1584-1596
	double dt = initInfo.timeStep/(24*60);
	const double maintCoeff = 0.015; // 0.015 gCH2O g-1DM day-1 at 20C is for young plants, Goudriaan and van Laar (1994) Wageningen textbook p 54, 60-61
	double agefn = (greenLeafArea+0.10)/(leafArea+0.10); // as more leaves senesce maint cost should go down, added 1 to both denom and numer to avoid division by zero
	double q10fn = pow(Q10,(w.airT - 20.0)/10);
	maintRespiration = agefn*q10fn*maintCoeff*mass*dt;// gCH2O gDM-1 day-1
}

void CPlant::writeNote(const TWeather & w)
{
	ostringstream oStr;
	string s = "";
	if (FLOAT_EQ(develop->germination.daytime,w.daytime)){s = "Germinated";}
	if (FLOAT_EQ(develop->emergence.daytime,w.daytime)){s = "Emergence";}
	if (FLOAT_EQ(develop->floralInitiation.daytime,w.daytime)){s = "Floral Initiation";}
	if (FLOAT_EQ(develop->flowering.daytime, w.daytime)){s = "Flowering";}
	if (FLOAT_EQ(develop->scapeAppear.daytime, w.daytime)){s = "Scape Appears";}
	if (FLOAT_EQ(develop->scapeRemoval.daytime, w.daytime)){s = "Scape Removed";}
	if (FLOAT_EQ(develop->bulbing.daytime, w.daytime)){s = "Begin Bulbing";}
	if (FLOAT_EQ(develop->maturation.daytime,w.daytime)){s = "Matured";}

	if (s != "")
	{
        oStr << s;
	}
	//note.swap(oStr.str());
	note = oStr.str();
}