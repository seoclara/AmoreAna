#ifndef _TPrimaryInfo__H_
#define _TPrimaryInfo__H_ 1

#include <iostream>
#include <vector>
#include <TObject.h>

class TPrimaryInfo
{
	public:
		TPrimaryInfo()
		{
			primX0 = 0.;
			primY0 = 0.;
			primZ0 = 0.;

			primPDGcode = 0;
			groupNo = 0;

			primParticleName[0] = '0';
			primVolumeName[0] = '0';
		};
		virtual ~TPrimaryInfo(){};

		void setPrimX0(double x0){ primX0 = x0; };
		void setPrimY0(double y0){ primY0 = y0; };
		void setPrimZ0(double z0){ primZ0 = z0; };
		void setPDGcode(int pdg){ primPDGcode = pdg; };
		void setGroupNo(int groupno){ groupNo = groupno; };
		void setPrimParticleName(const char *primname){ sprintf(primParticleName,"%s",primname); };
		void setPrimVolumeName(const char *primvname){ sprintf(primVolumeName,"%s",primvname); };

		double getPrimX0(){ return primX0; };
		double getPrimY0(){ return primY0; };
		double getPrimZ0(){ return primZ0; };
		int getPDGcode(){ return primPDGcode; };
		int getGroupNo(){ return groupNo; };
		const char * getPrimParticleName(){ return primParticleName; };
		const char * getPrimVolumeName(){ return primVolumeName; };

	private:
		double primX0;
		double primY0;
		double primZ0;

		int primPDGcode;
		int groupNo;

		char primParticleName[30];
		char primVolumeName[100];

	ClassDef(TPrimaryInfo,1)
};
#endif
