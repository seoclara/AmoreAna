#ifndef _TEdepInfo__H_
#define _TEdepInfo__H_ 1

#include <iostream>
#include <vector>
#include <TObject.h>

class TEdepInfo
{
	public:
		TEdepInfo() 
		{
			edepCell = 0.;
			cellID = -1;
		};
		virtual ~TEdepInfo(){};

		void setEdep(double edep){ edepCell = edep;};
		void setID(int id){ cellID = id;};

		double getEdep(){ return edepCell;};
		int getID(){ return cellID;};

	private:
		double edepCell;
	  int cellID;

	//TEdepInfo &operator=(const TEdepInfo &a);
	ClassDef(TEdepInfo,1)
};
#endif
