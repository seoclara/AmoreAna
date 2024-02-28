#ifndef _TSelectedTrack__H_
#define _TSelectedTrack__H_ 1

#include <iostream>
#include <vector>
#include <TObject.h>

class TSelectedTrack
{
	public:
		TSelectedTrack()
		{
			trackID = -1;
			kEnergy = 0.;
			NaIEdep = 0.;
			Passed = false;
			hitTag = false;

		};
		virtual ~TSelectedTrack(){};

		void setTrackID(int id){ trackID = id; };
		void setKineticEnergy(double ke){ kEnergy = ke; };
		void addEdep(double ed) { NaIEdep += ed; };
		void setBtoBTag(bool pass){ Passed = pass; };
		void setHitTag(bool hit){ hitTag = hit; };

		int getTrackID() { return trackID; };
		double getKineticEnergy() { return kEnergy; };
		double getNaIEdep() { return NaIEdep; };
		bool getBtoBTag() { return Passed; };
		bool getHitTag() { return hitTag; };
		
	private:
		int trackID;
		double kEnergy;
		double NaIEdep;
		bool Passed;
		bool hitTag;

	ClassDef(TSelectedTrack,1)
};
#endif
