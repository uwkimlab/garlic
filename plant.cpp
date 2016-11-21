#include "plant.h"
#include "gas_exchange.h"
#include "radtrans.h"
//#include "radiation.h"

#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>
#define PRIMORDIA 4   //leaf primordia initiated in a clove, Meredith (2008)
#define CO2_MW  44.0098
#define C_MW  12.011
#define CH2O_MW  30.03

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
	CH2O = (mass*C_conc)/C_MW*CH2O_MW;
	CH2O_pool = 0.0; // Short-term CH2O pool
    seed_reserve = seedMass*C_conc/C_MW*CH2O_MW; //CH2O reserved in the propagule (e.g., starch in endosperm of seeds)
    CH2O_reserve = seed_reserve; // Long-term CH2O pool
    CH2O_ns = 0.0;
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
	stemMass = scapeMass = stalkMass = leafMass = bulbMass = rootMass = shootMass= 0.0;

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
    seed_reserve = seedMass*C_conc/C_MW*CH2O_MW; //CH2O reserved in the propagule (e.g., starch in endosperm of seeds)
    CH2O = (mass*C_conc)/C_MW*CH2O_MW;; // For assimilates accounting in the form of carbohydrates
	CH2O_pool = 0.0;
    CH2O_reserve = seed_reserve; // Long-term CH2O pool
    CH2O_ns = 0.0;
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
	//if emegence date is given in the init file, then begin from that date
	if (initInfo.beginFromEmergence)
	{
		develop->germination.done = true;
        develop->germination.daytime = initInfo.sowingDay + initInfo.emergence;
		develop->emergence.done = true;
        develop->emergence.daytime = initInfo.sowingDay + initInfo.emergence;
        seedMass = seedMass*0.8;
    }

    partition = new TPartition[10];
    int maxLeafNo = 20; // total possible leaves for garlic, needs to be linked to that in development.cpp, 6/21/16, SK, KY, JH
	nodalUnit = new CNodalUnit[maxLeafNo]; // create enough leaf nodes for now, to be replaced by dynamic collection
	for (int i=0; i <= PRIMORDIA; i++) // leaf[0] is a coleoptile
	{
		nodalUnit[i].initialize(i, develop);
       	nodeNumber = i;
    }


	finalNodeNumber = info.genericLeafNo;
	photosynthesis_net =photosynthesis_gross = transpiration = assimilate =  0.0;
	leafArea =greenLeafArea = senescentLeafArea = potentialLeafArea = 0.0;
	stemMass = scapeMass = stalkMass = leafMass = bulbMass = rootMass = shootMass= 0.0;
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
	if (!develop->Germinated())
	{
		temperature = develop->get_Tcur();
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
        CH2O_pool += assimilate; // gCH2O per plant
		calcMaintRespiration(weather);
		CH2O_allocation(weather);
		set_mass();
        if (develop->get_devPhase() > Juvenile  && FLOAT_EQ(senescentLeafArea,leafArea))
        {
            develop->maturation.done = develop->BulbInitiated();
            develop->maturation.daytime=weather.daytime;
            cout << "* Ready for harvest: BBCH = " << develop->get_BBCH() << " " << weather.jday << endl;

        }

 	}
}

void CPlant::set_mass()
//TODO: allocate biomass into individual organs, currently it is allocated as a bulk to leaf, stem, and so on
//so individual leaf doesn't have mass but the first or the last one has it all
{

	double agefn = (greenLeafArea/potentialLeafArea); // as more leaves senesce living leaf biomass should go down
	{
		double leaf = 0.0;
		// apply leaf and sheath senescence and degradation. this is to mimic the loss of structural parts
        leaf = this->get_nodalUnit()->get_leaf()->get_mass();
		double sheath = this->get_nodalUnit()->get_stem()->get_mass();
		this->get_nodalUnit()->get_leaf()->set_mass(leaf*agefn);
		this->get_nodalUnit()->get_stem()->set_mass(sheath*agefn);
	}

    CH2O_ns = (CH2O_pool+CH2O_reserve);
    mass_nsc = CH2O_ns*((C_MW/CH2O_MW)/C_conc);
	stemMass = this->get_nodalUnit()->get_stem()->get_mass();
	scapeMass = this->get_scape()->get_mass();
	stalkMass = stemMass + scapeMass;
	leafMass = this->get_nodalUnit()->get_leaf()->get_mass();
	rootMass = this->get_roots()->get_mass();
	bulbMass = this->get_bulb()->get_mass()  + mass_nsc; // NSC is assumed to be stored in bulb. It would be seed garlic before bulbing and new bulb after bulbing.
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

int CPlant::getMatureLeafNumber() const
{
	int count = 0;
	for (int i = 1; i <= develop->get_LvsAppeared(); i++)
	{
		if (nodalUnit[i].get_leaf()->isMature()) {
			count++;
		}
	}
	return count;
}

double CPlant::getMatureLeafNumberSmooth() const
{
	double count = 0;
	CLeaf *previous_leaf = NULL;
	for (int i = 1; i <= develop->get_LvsAppeared(); i++)
	{
		CLeaf *leaf = nodalUnit[i].get_leaf();
		if (leaf->isMature()) {
			count += 1;
		} else {
			if (previous_leaf && previous_leaf->isMature()) {
				count += leaf->get_maturity();
			}
		}
		previous_leaf = leaf;
	}
	return count;
}

double CPlant::getTotalMaturity() const
{
	double total_maturity = 0;
	for (int i = 1; i <= develop->get_LvsAppeared(); i++)
	{
		CLeaf *leaf = nodalUnit[i].get_leaf();
		total_maturity += leaf->get_maturity();
	}
	return total_maturity;
}

int CPlant::getSenescentLeafNumber() const
{
	int count = 0;
	for (int i = 1; i <= develop->get_LvsAppeared(); i++)
	{
		CLeaf *leaf = nodalUnit[i].get_leaf();
		double senescence_ratio = leaf->get_senescentArea() / leaf->get_area();
		if (senescence_ratio >= 0.5) {
			count++;
		}
	}
	return count;
}

void CPlant::calcGasExchange(const TWeather & weather)
{
	const double LAF = 0.7; // leaf angle factor for garlic canopy, from Rizzalli et al. (2002),  X factor in Campbell and Norman (1998)
	const double leafwidth = 1.5; //to be calculated when implemented for individal leaves
	const double atmPressure= 100.0; //kPa, to be predicted using altitude
	double LAI = greenLeafArea*initInfo.plantDensity/(100.0*100.0);

	CGas_exchange * sunlit = new CGas_exchange();
	CGas_exchange * shaded = new CGas_exchange();
    CSolar * sun = new CSolar();
    CRadTrans * light = new CRadTrans();

// setPFD(weather.jday, weather.time, initInfo.latitude, initInfo.longitude, initInfo.altitude, tau, weather.PFD, LAI, LAF);
// setRad(int jday, double tm, double Lati, double Longi, double I0, double LAI, double LAF, bool IsObsPFD);
//TODO: lightenv.dll needs to be translated to C++. It slows down the execution, 3/16/05, SK
// radTrans(weather.jday, weather.time, initInfo.latitude, initInfo.longitude, weather.solRad, LAI, LAF);
    sun->SetVal(weather.jday, weather.time, initInfo.latitude, initInfo.longitude, initInfo.altitude, weather.solRad);
    light->SetVal(*sun , LAI, LAF);
//	radTrans2(weather.jday, weather.time, initInfo.latitude, initInfo.longitude, weather.solRad, weather.PFD, LAI, LAF);
	sunlit->SetVal(light->Qsl(), weather.airT, weather.CO2, weather.RH,
		        weather.wind, atmPressure, leafwidth);
	shaded->SetVal(light->Qsh(), weather.airT, weather.CO2, weather.RH,
		        weather.wind, atmPressure, leafwidth);
	photosynthesis_gross = (sunlit->A_gross*light->LAIsl() + shaded->A_gross*light->LAIsh());//plantsPerMeterSquare;
	photosynthesis_net = (sunlit->A_net*light->LAIsl() + shaded->A_net*light->LAIsh());
//	photosynthesis_net = sunlit->A_net;
	transpiration = (sunlit->ET*light->LAIsl() + shaded->ET*light->LAIsh());//plantsPerMeterSquare;
//	transpiration = sunlit->ET;
//	photosynthesis_gross = sunlit->A_gross*LAI;
//	photosynthesis_net = sunlitPFD();
//	transpiration = sunlit->ET*LAI;
	temperature = sunlit->Tleaf;
	assimilate = (photosynthesis_gross*CH2O_MW/1.0e6)*(60.0*initInfo.timeStep)/initInfo.plantDensity; // gCH2O per plant

	delete sunlit;
	delete shaded;
    delete sun;
    delete light;
}

void CPlant::CH2O_allocation(const TWeather & w)
// this needs to be f of temperature, source/sink relations, nitrogen, and probably water
// a valve function is necessary because assimilates from CPool cannot be dumped instantanesly to parts
// this may be used for implementing feedback inhibition due to high sugar content in the leaves
// The following is based on Grant (1989) AJ 81:563-571
{

    for (int i = Seed; i <= Dead; i++)
    {
        partition[i].BBCH = (BBCH_code) (int) initInfo.partTable[i][0];
        partition[i].root = initInfo.partTable[i][1];
        partition[i].shoot = initInfo.partTable[i][2];
        partition[i].leaf = initInfo.partTable[i][3];
        partition[i].sheath = initInfo.partTable[i][4];
        partition[i].scape = initInfo.partTable[i][5];
        partition[i].bulb = initInfo.partTable[i][6];
    }

   double tmprEffect = develop->beta_fn(develop->get_Tcur(), 1.0, develop->get_Topt(), develop->get_Tceil());

   double grofac = 0.2/(60/initInfo.timeStep); // translocation limitation and lag, assume it takes 1 hour to complete (1=no lag, same as the time step), 0.2=5 hrs
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
        CH2O_supply = max(CH2O_pool*tmprEffect*grofac, 0.0);
		//temprEffect and grofac are valve functions that determin the rate of assimilate transport to each part.
		//Any remaining assimilates not used to supply CH2O to other parts need to remain in leaves as short-term soluble C
		//Alternatively, this could be dumpted to long-term reserve first (but this would need another valve), and still remaining CH2O can stay in the leaves, SK, 7-16-13
        CH2O_pool = max(0.0, CH2O_pool - CH2O_supply);
	}
	else
	if (CH2O_reserve >= 0.0)
	{
		// conversion and translocation from long term reserve should be less efficient, apply nother glucose conversion factor
		// translocation from the soluble sugar reserve
		// deplete CH2O_pool first and recharge

		CH2O_recharge = max(CH2O_reserve* tmprEffect*grofac, 0.0);
		CH2O_pool += CH2O_recharge;
		CH2O_reserve = max(0.0, CH2O_reserve - CH2O_recharge);
        CH2O_supply = min(CH2O_pool*tmprEffect*grofac, 0.0);
        CH2O_pool = max(0.0, CH2O_pool - CH2O_supply);

	}
	else
	{
	    CH2O_pool = 0.0;
		CH2O_reserve = 0.0;
		CH2O_supply = 0.0;
	}

    // convFactor = 1/1.43; // equivalent to Yg, Goudriaan and van Laar (1994)
	double Yg = initInfo.Yg; // synthesis efficiency, ranges between 0.7 to 0.76 for corn, see Loomis and Amthor (1999), Grant (1989), McCree (1988) -- SK, Nov 2012
    double CH2O_net =Yg*(CH2O_supply-maintRespiration); //Net carbohydrate to allocate after all respiratory losses
	partition[develop->get_devPhase()].shoot = min(0.95, 0.80 + 0.20*scale);
    partition[develop->get_devPhase()].root = 1.0 - partition[develop->get_devPhase()].shoot;


       double rootPart = CH2O_net*partition[develop->get_devPhase()].root;
       double shootPart = CH2O_net*partition[develop->get_devPhase()].shoot;
       double leafPart = shootPart*partition[develop->get_devPhase()].leaf;
       double sheathPart = shootPart*partition[develop->get_devPhase()].sheath;
       double scapePart = shootPart*partition[develop->get_devPhase()].scape;
       double bulbPart = shootPart*partition[develop->get_devPhase()].bulb;

 		//Eventually, allocation to long-term reserve should be done after partitioning to all organs, this means each organ needs it own demand fn with potential sink capacity and a valve function (rate equation), 7-16-13, SK
		//alternatively, use valve functions only including a valve regulating the transport rate to long-term reserve, and whatever remaining in short-term pool to be stored in leaves
		//for now LT reserve also gets a min portion of assimilates, the way partitioning is done here now is rudimentary and teleonomic without empirical data to support from specifically designed experiments
		//this method is referred to as allometric method linked with dvs as described in Gourdriaan and van Laar (1996) and Penning de Vries and van Laar (1982)
       if (develop->BulbInitiated() && !develop->Matured())
       {
           const int maxCloveNo = 18; // assumed maximum cloves per bulb
           double maxCloveFillRate = 0.1*(initInfo.timeStep/(24*60)); // max clove filling rate = 0.1g clove-1 day-1, assumed
           CH2O_demand = maxCloveNo*maxCloveFillRate*tmprEffect;
           const double NSC_ratio = 0.2; //Nonstructural carbo allocated to bulb that can be used as long-term C reserve
           bulbPart -= NSC_ratio*bulbPart;
           CH2O_reserve += NSC_ratio*bulbPart;
       }

    double pseudostemPart = sheathPart;

    this->get_nodalUnit()->get_leaf()->import_CH2O(leafPart); // not working for each leaf until implementing an array iteration method or use it as public member
    this->get_nodalUnit()->get_stem()->import_CH2O(pseudostemPart);
    this->get_scape()->import_CH2O(scapePart);
    this->get_roots()->import_CH2O(rootPart);
    this->get_bulb()->import_CH2O(bulbPart);
}


void CPlant::calcMaintRespiration(const TWeather & w)
// based on McCree's paradigm, See McCree(1988), Amthor (2000), Goudriaan and van Laar (1994)
// units very important here, be explicit whether dealing with gC, gCH2O, or gCO2
{
	const double Q10 = 2.0; // typical Q10 value for respiration, Loomis and Amthor (1999) Crop Sci 39:1584-1596
	double dt = initInfo.timeStep/(24*60);
	const double maintCoeff = initInfo.Rm; // 0.015 gCH2O g-1DM day-1 at 20C is for young plants, Goudriaan and van Laar (1994) Wageningen textbook p 54, 60-61
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
    if (FLOAT_EQ(develop->bulbiling.daytime, w.daytime)){s = "Bulbil Growing";}
    if (FLOAT_EQ(develop->maturation.daytime,w.daytime)){s = "Matured";}

	if (s != "")
	{
        oStr << s;
	}
	//note.swap(oStr.str());
	note = oStr.str();
}
