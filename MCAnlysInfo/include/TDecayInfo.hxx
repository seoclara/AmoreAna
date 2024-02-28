#ifndef _TDecayInfo__H_
#define _TDecayInfo__H_ 1

#include <iostream>
#include <vector>
#include <TObject.h>

class TDecayInfo
{
	public:
		TDecayInfo()
		{
			detID = 0;

			edepAlpha = 0.;
			edepBeta = 0.;

			tagAlpha = false;
		};
		virtual ~TDecayInfo(){};

		void setDetID(int id){ detID = id; };
		void setEdepAlpha(double edep){ edepAlpha = edep; };
		void setEdepBeta(double edep){ edepBeta = edep; };
		void setAlphaTag(bool tag){ tagAlpha = tag; };

		int getDetID(){ return detID; };
		double getEdepAlpha(){ return edepAlpha; };
		double getEdepBeta(){ return edepBeta; };
		bool getAlphaTag(){ return tagAlpha; };

	private:
		int detID;

		double edepAlpha;
		double edepBeta;

		bool tagAlpha;

	ClassDef(TDecayInfo,1)
};
#endif
