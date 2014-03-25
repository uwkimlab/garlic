#pragma once
#include "organ.h"

class CBulb :
	public COrgan
{
public:
	CBulb(void);
	~CBulb(void);
private:
	unsigned int totalCloves;
	double cloveWeight;
};
