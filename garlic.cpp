// Garlic Crop Simulation Model
// Author: Soo-Hyung Kim (soohkim@uw.edu), Univ. of Washington
// Date: November 2012 (ver. 1.01)

#pragma once
#include "stdafx.h"
#include "controller.h"
#define MINUTESPERDAY (24*60);

int main(int argc, char * argv[])
{

	CController* pSC;
	pSC = new CController();

	int ier = pSC->getErrStatus();
	int count = 0;
	char* runFile = "run.dat";

	if ( ier == 0 )
	{
		if (argc == 1) 
		{
			ier = pSC->run(runFile);
		}
		else
		{
			for (count = 1; count < argc; count++)
			{
				runFile = argv[count];
				ier = pSC->run(runFile);
			}
		}
	}
	delete pSC;

	return 0;
}

