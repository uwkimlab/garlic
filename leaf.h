#pragma once
#ifndef _LEAF_H_
#define _LEAF_H_
#include "..\garlic\organ.h"
#include "..\garlic\weather.h"
#include "..\garlic\development.h"

class CLeaf: public COrgan
//leaf blade, call it "leaf" for convenience
{
	friend class CSheath;
public:
	CLeaf();
	CLeaf(int rank, CDevelopment * dv); // take current leaf rank and total leaf number to calculate potentialArea
	~CLeaf();

	bool isInitiated() {return initiated;}
	bool isVisible() {return appeared;}
	bool isMature() {return mature;}
	bool isAging() {return senescing;}
	bool isDead() {return dead;}
	bool isDropped() {return dropped;}
	double get_area(){return area;}
	double get_greenArea() {return greenArea;}
	double get_senescentArea() {return senescentArea;}
	double get_potentialArea(){return ptnArea;}
	double get_length() {return length;}
	double get_SLA() {return SLA;}
	double get_GDD2mature() {return GDD2mature;}


	void initialize(CDevelopment * dv);
	void set_area(double x) {area=x;}
	void set_length(double x) {length=x;}
	void set_SLA(double x) {SLA=x;}
	void set_GDD2mature(double x) {GDD2mature=x;}
	void update(CDevelopment *);
	void senescence(CDevelopment *);
	void elongate(CDevelopment *);

private:
	CLeaf(const CLeaf&);
	bool initiated, appeared, expanding, mature, senescing, dead, dropped;
	int rank;
	int totalLeaves;
	double ptnArea; // potential leaf area
	double area; // actual leaf area
	double greenArea;
	double senescentArea;
	double length;
	double width;
	double SLA;
	double plastochrons; // Fournier and Andrieu (1998), used for delay between initiation and elongation
	double phase1Delay, phase2Duration, elongAge, elongRate, seneAge, agingArea, GDD2mature ;
	double ptnLength, ptnWidth;
	double WLRATIO, A_LW; // Area/(L*W)
};
#endif