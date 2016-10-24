#pragma once
#ifndef _LEAF_H_
#define _LEAF_H_
#include "organ.h"
#include "weather.h"
#include "development.h"

class CLeaf: public COrgan
//leaf blade, call it "leaf" for convenience
{
	friend class CSheath;
public:
	CLeaf();
	CLeaf(int rank, CDevelopment * dv); // take current leaf rank and total leaf number to calculate potentialArea
	~CLeaf();

	bool isInitiated() const {return initiated;}
	bool isVisible() const {return appeared;}
	bool isMature() const {return mature;}
	bool isAging() const {return senescing;}
	bool isDead() const {return dead;}
	bool isDropped() const {return dropped;}
	double get_area() const {return area;}
	double get_greenArea() const {return greenArea;}
	double get_senescentArea() const {return senescentArea;}
	double get_potentialArea() const {return ptnArea;}
	double get_length() const {return length;}
	double get_potentialLength() const {return ptnLength;}
	double get_SLA() const {return SLA;}
	double get_GDD2mature() const {return GDD2mature;}


	void initialize(CDevelopment * dv);
	void set_area(double x) {area=x;}
	void set_length(double x) {length=x;}
	void set_SLA(double x) {SLA=x;}
	void set_GDD2mature(double x) {GDD2mature=x;}
	void update(CDevelopment *);
	void update_potentials(CDevelopment *);
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
