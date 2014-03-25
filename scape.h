#pragma once
#include "organ.h"

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
