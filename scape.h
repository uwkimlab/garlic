#pragma once
#include "..\garlic\organ.h"

class CScape :
	public COrgan
{
public:
	CScape(void);
	~CScape(void);
private:
	unsigned int totalInflorescence;
	double spatheWeight;
};
