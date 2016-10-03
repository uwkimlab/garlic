#include "stdafx.h"
#include "nodalunit.h"
#include "weather.h"

CNodalUnit::CNodalUnit()
{
	rank = 0; // coleoptile
	leaf = NULL;
	stem = NULL; // stem here is pseudostem of leaf sheath in garlic
	initiated = growing = aging = terminated = false;
}
void CNodalUnit::initialize(int n, CDevelopment * dv)
{
	rank = n;
	leaf = new CLeaf(n, dv);
	stem = new CStem(n);
	leaf->initialize(dv);
	stem->initialize();
	initiated = true;
}

CNodalUnit::~CNodalUnit()
{
	if (leaf !=NULL) delete leaf;
	if (stem !=NULL) delete stem;
//	delete sheath;
//	delete internode;
}

void CNodalUnit::update(CDevelopment * dv)
{
	leaf->update(dv);
	stem->update(dv);
//	stem->grow(weather);
	mass = leaf->get_mass() + stem->get_mass();
}
