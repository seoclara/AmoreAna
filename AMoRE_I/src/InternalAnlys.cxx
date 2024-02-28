// Background simulation code for AMoRE-I

// 06/2015 - Created by E.J.Jeon for CUPG4sim
// 09/2016 - revised by E.J.Jeon for CUPG4sim
// 04/2021 - revised by Jeewon Seo 

#include "InternalAnlys.hxx"

using namespace std;
using namespace MCAnlysDef::AMoRE_I;

int GetGroupNo(int PDGcode);
double Eres(double E, double para0, double para1);

void InternalAnlys(Int_t runno, const char *path, const char *filenameformat, const char *outpath, Int_t numpertask, const char *isotopeName, const char *crystal_type) 
{
	Int_t fill_count = 0;
	// Static information
	constexpr const char *tree_name = "event_tree";

	// input files adding -------------------------------
	TChain *fInput = new TChain(tree_name);
	for (int i = runno * numpertask; i < (runno + 1) * numpertask; i++){
		cout << "Adding: " << i + 1 << endl;
		fInput->Add(Form(Form("%s/%s", path, filenameformat), i));
	}

	// Set input reader variables -----------------------
	EvtInfo             *readInfo     = new EvtInfo;
	Primary             *readPrimary  = new Primary;
	EvtTrack            *readTrack    = new EvtTrack;
	EvtStep             *readStep     = new EvtStep;
	DetectorArray_Amore *readMDArray  = new DetectorArray_Amore;
	std::vector<TTrack *> *readEndTrack = new std::vector<TTrack *>;

	//TStep *readSingleStep;
	TClonesArray *tclVertex = readPrimary->GetVertex();
	TClonesArray *tclTrack  = readTrack->GetTrack();
	TClonesArray *tclStep   = readStep->GetStep();

	fInput->SetBranchAddress("EVENTINFO", &readInfo);
	fInput->SetBranchAddress("PRIMARY", &readPrimary);
	fInput->SetBranchAddress("TRACK", &readTrack);
	fInput->SetBranchAddress("STEP", &readStep);
	fInput->SetBranchAddress("MDSD", &readMDArray);
	fInput->SetBranchAddress("EndTrack", &readEndTrack);
	Bool_t enableET = (fInput->GetBranch("EndTrack") != nullptr);

	// output tree variables ----------------------------
	Int_t entryID, fileID, nPrimVrtx;
	Int_t evt_ID, evt_Type, groupNo, decayType;
	Double_t evt_UT, evt_deltaUT;
	Double_t stepTotEdep,crystalTotEdep;
	Int_t primPDGcode[max_nprimary];
	Double_t primX0[max_nprimary];
	Double_t primY0[max_nprimary];
	Double_t primZ0[max_nprimary];
	Double_t stepEdep[max_ncrystals];
	Double_t edepAlpha[max_ncrystals];
	Double_t edepBeta[max_ncrystals];
	Double_t AlphaTime[max_ncrystals];
	Double_t BetaTime[max_ncrystals];
	Double_t AlphaResol[max_ncrystals];
	Double_t BetaResol[max_ncrystals];
	Double_t crystalMass[max_ncrystals]; 
	Double_t crystalEdep[max_ncrystals];
	Double_t edepResol[max_ncrystals];
	Double_t edepQuenched[max_ncrystals];
	Bool_t singleHitTag;
	Bool_t multipleHitTag;
	Bool_t AlphaTag[max_ncrystals];
	Bool_t ReAlphaTag[max_ncrystals];

	Double_t alphaPathL;
	Double_t alphaKE;

	// Set output tree -----------------------------------
	TFile *fOutput = new TFile(Form("%s/result_%s_%s_run%d.root", outpath, crystal_type, isotopeName, runno), "RECREATE");
	TTree *fOTree  = new TTree("Internal", "Internal Production");
	// ...... file information
	fOTree->Branch("fileID", &fileID, "fileID/I");
	fOTree->Branch("entryID", &entryID, "entryID/I");
	//........ event information
	fOTree->Branch("evt_ID",&evt_ID,"evt_ID/I");
	fOTree->Branch("evt_UT",&evt_UT,"evt_UT/D");
	fOTree->Branch("evt_deltaUT",&evt_deltaUT,"evt_deltaUT/D");
	fOTree->Branch("evt_Type",&evt_Type,"evt_Type/I");
	//........ Primary vertex information
	fOTree->Branch("nPrimVrtx", &nPrimVrtx, "nPrimVrtx/I");
	fOTree->Branch("groupNo", &groupNo, "groupNo/I"); 
	fOTree->Branch("decayType", &decayType, "decayType/I");
	fOTree->Branch("primPDGcode", primPDGcode, Form("primPDGcode[%zu]/I",max_nprimary));
	fOTree->Branch("primX0", primX0, Form("primX0[%zu]/D",max_nprimary));
	fOTree->Branch("primY0", primY0, Form("primY0[%zu]/D",max_nprimary));
	fOTree->Branch("primZ0", primZ0, Form("primZ0[%zu]/D",max_nprimary));
	//....... step information
	fOTree->Branch("stepTotEdep",&stepTotEdep, "stepTotEdep/D");
	fOTree->Branch("stepEdep",stepEdep,Form("stepEdep[%lu]/D",max_ncrystals));
	fOTree->Branch("edepAlpha", edepAlpha, Form("edepAlpha[%lu]/D",max_ncrystals)); 
	fOTree->Branch("edepBeta", edepBeta, Form("edepBeta[%lu]/D",max_ncrystals)); 
	fOTree->Branch("AlphaTime",AlphaTime,Form("AlphaTime[%lu]/D",max_ncrystals));
	fOTree->Branch("BetaTime",BetaTime,Form("BetaTime[%lu]/D",max_ncrystals));
	fOTree->Branch("AlphaResol", AlphaResol, Form("AlphaResol[%lu]/D",max_ncrystals)); 
	fOTree->Branch("BetaResol", BetaResol, Form("BetaResol[%lu]/D",max_ncrystals)); 
	//....... MDSD information
	fOTree->Branch("crystalTotEdep",&crystalTotEdep,"crystalTotEdep/D");
	fOTree->Branch("singleHitTag", &singleHitTag, "singleHitTag/O"); 
	fOTree->Branch("multipleHitTag", &multipleHitTag, "multipleHitTag/O"); 
	fOTree->Branch("crystalMass", crystalMass, Form("crystalMass[%zu]/D",max_ncrystals)); 
	fOTree->Branch("crystalEdep", crystalEdep,Form("crystalEdep[%zu]/D",max_ncrystals)); 
	fOTree->Branch("edepResol", edepResol, Form("edepResol[%zu]/D",max_ncrystals)); 
	fOTree->Branch("edepQuenched", edepQuenched, Form("edepQuenched[%zu]/D",max_ncrystals)); 
	fOTree->Branch("AlphaTag",AlphaTag,Form("AlphaTag[%zu]/O",max_ncrystals));
	fOTree->Branch("ReAlphaTag",ReAlphaTag,Form("ReAlphaTag[%zu]/O",max_ncrystals));

	fOTree->Branch("alphaPathL",&alphaPathL,"alphaPathL/D");
	fOTree->Branch("alphaKE",&alphaKE,"alphaKE/D");

	// Event Loop start --------------------------------
	Int_t entries = fInput->GetEntries();
	//Int_t entries = 1000;
	Int_t curec, entrycnt = 0;
	Double_t par[2];
	Double_t alpha_time[max_ncrystals];
	Double_t beta_time[max_ncrystals];

	Bool_t preAlphatag[max_ncrystals];
	Bool_t preReAlphatag[max_ncrystals];
	for (int ha = 0; ha < max_ncrystals; ha++){
		preAlphatag[ha] = false; 
		preReAlphatag[ha] = false;
	}

	int setup;
	double fadcEresPars[2];
	double sadcEresPars[2];

	for(int i = 0; i < entries; i++){
		fInput->GetEntry(i);

		Bool_t skipThisEvt = false;
		Int_t pdgcode;
		TVector3 sPos;
		TVector3 fPos;
		TVector3 alphaPath;
		alphaKE=0;

		if (entrycnt <= i){
			curec = ((TTree *)fInput->GetFile()->Get(tree_name))->GetEntriesFast();
			entrycnt += curec;
			const char *now_filedesc = fInput->GetFile()->GetName();
			TObjArray *tokened_nfd   = TString(now_filedesc).Tokenize("/");
			const char *now_filename = static_cast<TObjString *>(tokened_nfd->At(tokened_nfd->GetEntriesFast() - 1))->String().Data();
			sscanf(now_filename, filenameformat, &fileID);
			cout << "NOWFILE: " << now_filename << " FILENO: " << fileID << " NOWINDEX: " << i << " CURFILEENTRIES: " << curec << endl;
		}
		entryID = i - entrycnt + curec;

		// EVENT INFORMATION --------------------------------
		evt_ID = readInfo->GetEventID();
		evt_UT = readInfo->GetUT();
		evt_deltaUT = readInfo->GetDeltaUT();
		evt_Type = readInfo->GetEventType();


		// PRIMARY READING ---------------------------------
		groupNo = 0;
		decayType = 0;
		for(int iv = 0; iv < max_nprimary; iv++){
			primPDGcode[iv] = 0;
			primX0[iv] = 0.;
			primY0[iv] = 0.;
			primZ0[iv] = 0.;
		}

		int nvrtx = tclVertex->GetEntriesFast();
		nPrimVrtx = nvrtx;
		if(nvrtx>max_nprimary) nvrtx=max_nprimary;

		for(int iv = 0; iv < nvrtx; iv++){
			Vertex *readSingleVertex = static_cast<Vertex *>(tclVertex->At(iv));
			const char *vname = readSingleVertex->GetVolumeName();
			const char *pname = readSingleVertex->GetParticleName();
			pdgcode = readSingleVertex->GetPDGcode();
			primPDGcode[iv] = pdgcode;
			groupNo = GetGroupNo(pdgcode);
			primX0[iv] = readSingleVertex->GetX0();
			primY0[iv] = readSingleVertex->GetY0();
			primZ0[iv] = readSingleVertex->GetZ0();
			if(strstr(readSingleVertex->GetParticleName(),"alpha")) decayType = 1;
			else if(decayType!=1 && (strstr(pname,"e-") || strstr(pname,"anti_nu_e"))) decayType = 2;
			else if(decayType!=1 && decayType!=2) decayType = 0;
			//cout << "particle name : " << pname << ", decayType : " << decayType << endl;
		}

		for(int ic = 0; ic < max_ncrystals; ic++){
			stepEdep[ic] = 0;
			edepAlpha[ic] = 0.;
			edepBeta[ic] = 0.;
			alpha_time[ic] = 0.;
			beta_time[ic] = 0.;
			AlphaTime[ic] = 0.;
			BetaTime[ic] = 0.;
			AlphaResol[ic] = 0.;
			BetaResol[ic] = 0.;
			AlphaTag[ic] = false;
			ReAlphaTag[ic] = false;
		}
		// STEP READING ----------------------------------
		stepTotEdep= 0.;
		Int_t stepnum = tclStep->GetEntriesFast();
		for(int ist = 0; ist < stepnum; ist++){
			TStep *readSingleStep = (TStep*)tclStep->At(ist);
			const Int_t nowTID    = readSingleStep->GetTrackID();
			const char *pname = readSingleStep->GetParticleName();

			constexpr size_t bufferMaxSize      = 15;
			constexpr const char *checkerString = "_Crystal_PV";
			const char *volName                 = readSingleStep->GetVolumeName();
			const size_t volNameLength          = strlen(volName);
			const size_t checkerLength          = strlen(checkerString);
			pdgcode = readSingleStep->GetPDGcode();

			if(volNameLength > checkerLength && strcmp(volName + volNameLength - checkerLength, checkerString) == 0){
				size_t cmonum;
				char buffer[bufferMaxSize] = {'\0'};
				if (volNameLength - checkerLength > bufferMaxSize){
					Error(__func__, "Wrong Crystal PV name, cmonum will be 17. (volname: %s)", volName);
					cmonum = 17;
				}
				strncpy(buffer, volName, volNameLength - checkerLength);
				const char *crystalName      = strtok(buffer, "_");
				const char *crystalNumberStr = strtok(NULL, "_");
				int crystalNumber    = atoi(crystalNumberStr);
				int realchannel = realChannelList[crystalNumber];
				if(strcmp(crystalNameList[realchannel], crystalName) == 0){
					//cmonum = crystalNumber;
					cmonum = realchannel;
				} 

				Double_t nowEdep = readSingleStep->GetEnergyDeposit();
				if(nowEdep > 0.){
					stepEdep[cmonum] += nowEdep;
					stepTotEdep += nowEdep;

					if( !strcmp(pname,"alpha") || pdgcode>20040 ){
						if(readSingleStep->GetGlobalTime()<=500000.){
							edepAlpha[cmonum] += nowEdep;
							alpha_time[cmonum] += nowEdep * readSingleStep->GetGlobalTime();
							if(!strcmp(pname,"alpha") && readSingleStep->GetTrackID()==1) {
								if(sPos.X()==0 && sPos.Y()==0 && sPos.Z()==0) {
									sPos.SetXYZ(readSingleStep->GetX(),readSingleStep->GetY(),readSingleStep->GetZ());
									alphaKE = readSingleStep->GetKineticEnergy();
								}
								fPos.SetXYZ(readSingleStep->GetX(),readSingleStep->GetY(),readSingleStep->GetZ());
							}
						}
					}
					else{
						if(readSingleStep->GetGlobalTime()<=500000.){
							edepBeta[cmonum] += nowEdep;
							beta_time[cmonum] += nowEdep * readSingleStep->GetGlobalTime();
						}
					}
				}
			}
		}

		alphaPath = fPos-sPos;
		alphaPathL = alphaPath.Mag();

		for(int dd = 0; dd < max_ncrystals; dd++ ){
			if (edepAlpha[dd]==0) AlphaTime[dd] = 0;
			if (edepAlpha[dd]>0) {
				AlphaTime[dd] = alpha_time[dd]/edepAlpha[dd];
			}
			if (edepBeta[dd]==0) BetaTime[dd] = 0;
			if (edepBeta[dd]>0) BetaTime[dd] = beta_time[dd]/edepBeta[dd];
		}

		for(int tt = 0; tt < max_ncrystals; tt++){
			par[0] = 4.40146e-03;
			par[1] = -5.24207e-03;
			AlphaResol[tt] = Eres(edepAlpha[tt],par[0],par[1]);
			par[0] = 5.25349e-03;
			par[1] = 1.66365e-01;
			BetaResol[tt] = Eres(edepBeta[tt],par[0],par[1]);
		}

		// MDSD READING ----------------------------------
		par[0] = 5.25349e-03;
		par[1] = 1.66365e-01;
		crystalTotEdep = 0.;
		singleHitTag   = false;
		multipleHitTag = false;

		for(int ic = 0; ic < max_ncrystals; ic++){
			DetectorModule_Amore &readSingleModule = readMDArray->GetDetectorModule(ic);
			crystalMass[ic] = crystalMassList[ic];
			crystalEdep[ic] = readSingleModule.GetCrystalEdep();
			crystalTotEdep += readSingleModule.GetCrystalEdep();
			edepResol[ic] = Eres(crystalEdep[ic],par[0],par[1]);
			edepQuenched[ic] = readSingleModule.GetQuenchedCrystalEdep();
			//printf("Edep single module: %f, crystalEdep : %f \n", readSingleModule.GetCrystalEdep(), crystalEdep[ic]); 
		}

		// EVENT TAGGING ---------- singlehit/multiplehit tagging
		for(int ic = 0; ic < max_ncrystals; ic++){
			if(crystalEdep[ic]>0){
				(crystalTotEdep == crystalEdep[ic]) ? singleHitTag = true : multipleHitTag = true;
			}
		}

		// EVENT TAGGING ---------- alpha/beta tagging
		for(int qq = 0; qq < max_ncrystals; qq++){
			if( preAlphatag[qq] && (edepAlpha[qq]+edepBeta[qq])>=3.024 && (edepAlpha[qq]+edepBeta[qq])<=3.044 ) AlphaTag[qq] = true;
			else AlphaTag[qq] = false; 

			if( preReAlphatag[qq] && (AlphaResol[qq]+BetaResol[qq])>=3.024 && (AlphaResol[qq]+BetaResol[qq])<=3.044 )	ReAlphaTag[qq] = true;
			else ReAlphaTag[qq] = false; 
		}
		for (int dd = 0; dd < max_ncrystals; dd++){
			if(edepAlpha[dd] >= 6.00 && edepAlpha[dd] <= 6.2) preAlphatag[dd] = true;
			else preAlphatag[dd] = false;

			if(AlphaResol[dd] >= 6.00 && AlphaResol[dd]<=6.2 ) preReAlphatag[dd] = true;
			else preReAlphatag[dd] = false;
		}
		fOTree->Fill();
	}
	fOutput->cd();
	fOTree->Write();
	fOutput->Close();
}

int GetGroupNo(int PDGcode)
{
	int grpno = 0;
	char szBuf[128];
	sprintf(szBuf,"%i", PDGcode);

	if (((!strncmp(szBuf,"1000922380",9))) ||  //U238: 92238
			((!strncmp(szBuf,"1000902340",9))) ||  //Th234: 90234
			((!strncmp(szBuf,"1000912340",9))) ||  //Pa234: 91234
			((!strncmp(szBuf,"1000922340",9))))    //U234: 92234
	{		grpno = 11;  }
	else if (((!strncmp(szBuf,"1000902300",9)))) //Th230: 90230
	{		grpno = 12;  }
	else if ( ((!strncmp(szBuf,"1000882260",9))) ) //Ra226: 88226
	{		grpno = 13;	}
	else if (((!strncmp(szBuf,"1000862220",9))) ||  //Rn222: 86222
			((!strncmp(szBuf,"1000842180",9))) ||  //Po218: 84218
			((!strncmp(szBuf,"1000852180",9))) ||  //At218: 85218
			((!strncmp(szBuf,"1000822140",9))) ||  //Pb214: 82214
			((!strncmp(szBuf,"1000832140",9))) ||  //Bi214: 83214
			((!strncmp(szBuf,"1000842140",9))) ||  //Po214: 84214
			((!strncmp(szBuf,"1000822100",9))) ||  //Pb210: 82210
			((!strncmp(szBuf,"1000812100",9))) )   //Tl210: 81210
	{		grpno = 14;  }
	else if (((!strncmp(szBuf,"1000832100",9))) ||  //Bi210: 83210
			((!strncmp(szBuf,"1000842100",9))) ||  //Po210: 84210
			((!strncmp(szBuf,"1000802060",9))) ||  //Hg206: 80206
			((!strncmp(szBuf,"1000812060",9))) ||  //Tl206: 81206
			((!strncmp(szBuf,"1000822060",9))) )   //Pb206: 82206
	{		grpno = 15;	}
	else if ( ((!strncmp(szBuf,"1000902320",9))) || //Th232: 90232
			((!strncmp(szBuf,"1000882280",9))) )  //Ra228: 88228
	{		grpno = 21;  }
	else if ( ((!strncmp(szBuf,"1000892280",9))) ||  //Ac228: 89228
			((!strncmp(szBuf,"1000902280",9))) )   //Th228: 90228
	{		grpno = 22;	}
	else if ( ((!strncmp(szBuf,"1000882240",9))) ||  //Ra224: 88224
			((!strncmp(szBuf,"1000862200",9))) ||  //Rn220: 86220
			((!strncmp(szBuf,"1000842160",9))) ||  //Po216: 84216
			((!strncmp(szBuf,"1000822120",9))) ||  //Pb212: 82212
			((!strncmp(szBuf,"1000832120",9))) ||  //Bi212: 83212
			((!strncmp(szBuf,"1000842120",9))) ||  //Po212: 84212
			((!strncmp(szBuf,"1000812080",9))) ||  //Tl208: 81208
			((!strncmp(szBuf,"1000822080",9))) )   //Pb208: 82208
	{		grpno = 23;	}
	else if ( ((!strncmp(szBuf,"1000190400",9))) ||  //K40: 19040 
			((!strncmp(szBuf,"1000180400",9))) ||  //Ar40: 18040
			((!strncmp(szBuf,"1000200400",9))) )   //Ca40: 20040
	{		grpno = 31;	}
	else if ( ((!strncmp(szBuf,"1000922350",9))) ||  //U235: 92235
			((!strncmp(szBuf,"1000902310",9))) ||  //Th231: 90231
			((!strncmp(szBuf,"1000912310",9))) )   //Pa231: 91231
	{		grpno = 41;	}
	else if ( ((!strncmp(szBuf,"1000892270",9))) ||  //Ac227: 89227
			((!strncmp(szBuf,"1000902270",9))) ||  //Th227: 90227
			((!strncmp(szBuf,"1000872230",9))) ||  //Fr223: 87223
			((!strncmp(szBuf,"1000882230",9))) ||  //Ra223: 88223
			((!strncmp(szBuf,"1000852190",9))) ||  //At219: 85219
			((!strncmp(szBuf,"1000862190",9))) ||  //Rn219: 86219
			((!strncmp(szBuf,"1000832150",9))) ||  //Bi215: 83215
			((!strncmp(szBuf,"1000842150",9))) ||  //Po215: 84215
			((!strncmp(szBuf,"1000852150",9))) ||  //At215: 85215
			((!strncmp(szBuf,"1000822110",9))) ||  //Pb211: 82211
			((!strncmp(szBuf,"1000832110",9))) ||  //Bi211: 83211
			((!strncmp(szBuf,"1000842110",9))) ||  //Po211: 84211
			((!strncmp(szBuf,"1000812070",9))) ||  //Tl207: 81207
			((!strncmp(szBuf,"1000822070",9))) )   //Pb207: 82207
	{		grpno =42;	}
	else  { grpno = 0;	}

	return grpno;
}

double Eres(double E, double para0, double para1)
{
	TRandom3 rnd(0);
	E *= 1000.;
	//double sigma = para0 * TMath::Power(E, 0.5) + para1 * E;
	double sigma = para1 * TMath::Power(E, 0.5) + para0 * E;
	double Eresol = rnd.Gaus(E, sigma);
	return Eresol /= 1000.;
}

int main(int, char *argv[]) 
{
	InternalAnlys(stoi(argv[1]), argv[2], argv[3], argv[4], stoi(argv[5]), argv[6], argv[7]);
	return 0;
}
