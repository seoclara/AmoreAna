///// Background analysis code for AMoRE-II (phase-I)
///// Modified at 18.Nov.2021.
///// Bug fixed at 29.Feb.2024.

#include "BackgroundAnlys.hxx"

using namespace std;

int GetGroupNo(int PDGcode);

void BackgroundAnlys(Int_t runno, const char *path, const char *filenameformat, const char *outpath, Int_t numpertask, const char *isotopeName, const char *phase)
{
	Int_t fill_count = 0;
	///// Static information -------------------------------
	const Double_t hit_evt_window = 100 * ms;
	const Double_t hit_evt_Eth = 10;
	constexpr const char *tree_name = "event_tree";
	TCMOHitInfo::SetDefaultEventWindow(hit_evt_window);
	TCMOHitInfo::SetDefaultEnergyThreshold(hit_evt_Eth);

	///// input files adding -------------------------------
	TChain *fInput = new TChain(tree_name);
	for (int i = runno * numpertask; i < (runno + 1) * numpertask; i++)
	{
		cout << "Adding: " << i + 1 << endl;
		fInput->Add(Form(Form("%s/%s.root", path, filenameformat), i));
		// fInput->Add(Form(Form("%s/%s_1.root", path, filenameformat), i));
	}

	///// Set input reader variables -----------------------
	EvtInfo *readInfo = new EvtInfo;
	Primary *readPrimary = new Primary;
	EvtTrack *readTrack = new EvtTrack;
	EvtStep *readStep = new EvtStep;
	std::vector<TTrack *> *readEndTrack = new std::vector<TTrack *>;

	TClonesArray *tclVertex = readPrimary->GetVertex();
	TClonesArray *tclTrack = readTrack->GetTrack();
	TClonesArray *tclStep = readStep->GetStep();

	fInput->SetBranchAddress("EVENTINFO", &readInfo);
	fInput->SetBranchAddress("PRIMARY", &readPrimary);
	fInput->SetBranchAddress("TRACK", &readTrack);
	fInput->SetBranchAddress("STEP", &readStep);
	fInput->SetBranchAddress("EndTrack", &readEndTrack);
	Bool_t enableET = (fInput->GetBranch("EndTrack") != nullptr);

	///// output tree variables ----------------------------
	int entryID, fileID, nPrimVrtx;
	int evt_ID, evt_Type, groupNo;
	int evt_UT, evt_deltaUT; // time in nanosecond
	double stepTotEdep;

	std::vector<TPrimaryInfo> primColl;
	std::vector<TDecayInfo> decayColl;

	///// Set output tree -----------------------------------
	TFile *fOutput = new TFile(Form("%s/repro_%s_%s_run%d.root", outpath, phase, isotopeName, runno), "RECREATE");
	TTree *fOTree = new TTree("Background", "Background Analysis Production");
	///// ...... file information
	fOTree->Branch("fileID", &fileID, "fileID/I");
	fOTree->Branch("entryID", &entryID, "entryID/I");
	/////........ event information
	fOTree->Branch("evt_ID", &evt_ID, "evt_ID/I");
	fOTree->Branch("evt_UT", &evt_UT, "evt_UT/D");
	fOTree->Branch("evt_deltaUT", &evt_deltaUT, "evt_deltaUT/D");
	fOTree->Branch("evt_Type", &evt_Type, "evt_Type/I");
	/////........ Primary vertex information
	fOTree->Branch("nPrimVrtx", &nPrimVrtx, "nPrimVrtx/I");
	fOTree->Branch("primColl", &primColl);
	///// ..... Decay information from step
	fOTree->Branch("stepTotEdep", &stepTotEdep, "stepTotEdep/D");
	fOTree->Branch("decayColl", &decayColl);

	///// EVENT LOOP --------------------------------------
	int entries = fInput->GetEntries();
	int curec, entrycnt = 0;
	bool preAlphatag[max_ncrystals];
	for (int ha = 0; ha < max_ncrystals; ha++)
	{
		preAlphatag[ha] = false;
	}

	for (int i = 0; i < entries; i++)
	{
		// for( int i = 0; i < 10000; i++ ) {
		fInput->GetEntry(i);
		// if(i%20000 ==0) {
		// i += 80001;
		//}

		primColl.clear();
		decayColl.clear();

		if (entrycnt <= i)
		{
			curec = ((TTree *)fInput->GetFile()->Get(tree_name))->GetEntriesFast();
			entrycnt += curec;
			const char *now_filedesc = fInput->GetFile()->GetName();
			TObjArray *tokened_nfd = TString(now_filedesc).Tokenize("/");
			const char *now_filename = static_cast<TObjString *>(tokened_nfd->At(tokened_nfd->GetEntriesFast() - 1))->String().Data();
			sscanf(now_filename, filenameformat, &fileID);
			cout << "NOWFILE: " << now_filename << " FILENO: " << fileID << " NOWINDEX: " << i << " CURFILEENTRIES: " << curec << endl;
		}
		entryID = i - entrycnt + curec;

		///// TRACK CHECK ----------------------------------
		bool skipThisEvt = false;
		Int_t TotTrackN = tclTrack->GetEntriesFast();
		for (int j = 0; j < TotTrackN; j++)
		{
			auto readSingleEndTrack = (*readEndTrack)[j];
			TTrack *readSingleTrack = static_cast<TTrack *>(tclTrack->At(j));
			if (readSingleTrack->GetTrackID() >= TotTrackN + 1)
			{
				cout << "===================" << endl;
				cout << "WARNING! TOO MANY TRACKS! : " << TotTrackN << " "
					 << readSingleTrack->GetTrackID() << endl;
				cout << "This event will be skipped" << endl;
				cout << "FileID: " << fileID << " | EntryID: " << entryID << endl;
				cout << "===================" << endl;
				skipThisEvt = true;
				break;
			}
		}
		if (skipThisEvt)
			continue;

		///// EVENT INFORMATION --------------------------------
		evt_ID = readInfo->GetEventID();
		evt_UT = readInfo->GetUT();
		evt_deltaUT = readInfo->GetDeltaUT();
		evt_Type = readInfo->GetEventType();

		///// PRIMARY READING ---------------------------------
		groupNo = -1;
		int ngroup = 0;
		int nvrtx = tclVertex->GetEntriesFast();
		nPrimVrtx = nvrtx;

		for (int iv = 0; iv < nvrtx; iv++)
		{
			Vertex *readSingleVertex = static_cast<Vertex *>(tclVertex->At(iv));
			const int pdgcode = readSingleVertex->GetPDGcode();

			/// When the primary particle is isotope in decay chain,
			/// the group in the chain will be checked and the group number will be saved.
			if (groupNo == -1)
			{
				groupNo = GetGroupNo(pdgcode);
				ngroup = 1;
			}
			else if (groupNo == 0 && GetGroupNo(pdgcode) != 0)
			{
				groupNo = GetGroupNo(pdgcode);
				ngroup = 1;
			}
			else if (groupNo > 0 && GetGroupNo(pdgcode) != 0)
			{
				printf("WORNING!!! TWO group in one events!\n");
				ngroup++;
			}

			/// isotop name saving (This is corresponding to the decayed particle from previous events)
			/// The deposited energy of this events is the energy of previous isotope's q-value when it is full peak.
			TPrimaryInfo primary;
			primary.setPDGcode(pdgcode);
			primary.setGroupNo(groupNo);
			primary.setPrimParticleName(readSingleVertex->GetParticleName());
			primary.setPrimVolumeName(readSingleVertex->GetVolumeName());
			primary.setPrimX0(readSingleVertex->GetX0());
			primary.setPrimY0(readSingleVertex->GetY0());
			primary.setPrimZ0(readSingleVertex->GetZ0());
			primColl.push_back(primary);
		}

		///// STEP READING ----------------------------------
		stepTotEdep = 0;
		double edepAlpha[max_ncrystals] = {0};
		double edepBeta[max_ncrystals] = {0};
		bool AlphaTag[max_ncrystals] = {0};

		int TotStepN = tclStep->GetEntriesFast();
		for (int ist = 0; ist < TotStepN; ist++)
		{
			TStep *readSingleStep = (TStep *)(tclStep->At(ist));
			const int nowTID = readSingleStep->GetTrackID();
			const int pdgcode = readSingleStep->GetPDGcode();
			const char *vname = readSingleStep->GetVolumeName();
			const char *pname = readSingleStep->GetParticleName();

			char buffer[30] = {'\0'};
			const size_t volNameLength = strlen(vname);
			// strncpy(buffer, vname + 15, volNameLength);
			strncpy(buffer, vname, sizeof(buffer));

			if (strstr(vname, "physCrystalCell"))
			{
				char *tempstr = strtok(buffer, "physCrystalCell");
				const char *towerNumberStr = strtok(tempstr, "_");
				const char *moduleNumberStr = strtok(NULL, "_");
				int towerNum = atoi(towerNumberStr);
				int moduleNum = atoi(moduleNumberStr);
				int crystalNum = towerNum * 9 + moduleNum;

				double nowEdep = readSingleStep->GetEnergyDeposit();
				if (nowEdep <= 0.)
					continue;

				stepTotEdep += nowEdep;
				if (strstr(pname, "alpha") || pdgcode > 20040)
				{
					if (readSingleStep->GetGlobalTime() <= 5000000.)
					{
						edepAlpha[crystalNum] += nowEdep;
					} // 5ms cut
				}
				else
				{
					if (readSingleStep->GetGlobalTime() <= 5000000.)
					{
						edepBeta[crystalNum] += nowEdep;
					}
				}
			}
		}

		///// EVENT TAGGING ----------- alpha/beta tagging
		for (int qq = 0; qq < max_ncrystals; qq++)
		{
			// if(preAlphatag[qq] && (edepAlpha[qq] + edepBeta[qq]) >=3.024 && (edepAlpha[qq] + edepBeta[qq])<=3.044 && evt_deltaUT*1e-09 <30*60 ) AlphaTag[qq] = true;
			if (preAlphatag[qq] && evt_deltaUT * 1e-09 < 30 * 60)
				AlphaTag[qq] = true;
			else
				AlphaTag[qq] = false;
		}
		for (int dd = 0; dd < max_ncrystals; dd++)
		{
			if (edepAlpha[dd] + edepBeta[dd] >= 6.207 - 0.05 && edepAlpha[dd] + edepBeta[dd] <= 6.207 + 0.05)
			{
				preAlphatag[dd] = true;
			}
			else
				preAlphatag[dd] = false;
		}

		///// DECAY INFORMATION SAVING
		for (int idt = 0; idt < max_ncrystals; idt++)
		{
			if (edepAlpha[idt] > 0 || edepBeta[idt] > 0)
			{
				TDecayInfo decay;
				decay.setDetID(idt);
				decay.setEdepAlpha(edepAlpha[idt]);
				decay.setEdepBeta(edepBeta[idt]);
				decay.setAlphaTag(AlphaTag[idt]);
				decayColl.push_back(decay);
			}
		}

		// if(decayColl.size()>0){ fOTree->Fill(); }
		fOTree->Fill();
	}
	fOutput->cd();
	fOTree->Write();
	fOutput->Close();
	cout << "END" << endl;
}

int GetGroupNo(int PDGcode)
{
	int grpno = 0;
	char szBuf[128];
	sprintf(szBuf, "%i", PDGcode);

	if (((!strncmp(szBuf, "1000922380", 9))) || // U238: 92238
		((!strncmp(szBuf, "1000902340", 9))) || // Th234: 90234
		((!strncmp(szBuf, "1000912340", 9))) || // Pa234: 91234
		((!strncmp(szBuf, "1000922340", 9))))	// U234: 92234
	{
		grpno = 11;
	}
	else if (((!strncmp(szBuf, "1000902300", 9)))) // Th230: 90230
	{
		grpno = 12;
	}
	else if (((!strncmp(szBuf, "1000882260", 9)))) // Ra226: 88226
	{
		grpno = 13;
	}
	else if (((!strncmp(szBuf, "1000862220", 9))) || // Rn222: 86222
			 ((!strncmp(szBuf, "1000842180", 9))) || // Po218: 84218
			 ((!strncmp(szBuf, "1000852180", 9))) || // At218: 85218
			 ((!strncmp(szBuf, "1000822140", 9))) || // Pb214: 82214
			 ((!strncmp(szBuf, "1000832140", 9))) || // Bi214: 83214
			 ((!strncmp(szBuf, "1000842140", 9))) || // Po214: 84214
			 ((!strncmp(szBuf, "1000822100", 9))) || // Pb210: 82210
			 ((!strncmp(szBuf, "1000812100", 9))))	 // Tl210: 81210
	{
		grpno = 14;
	}
	else if (((!strncmp(szBuf, "1000832100", 9))) || // Bi210: 83210
			 ((!strncmp(szBuf, "1000842100", 9))) || // Po210: 84210
			 ((!strncmp(szBuf, "1000802060", 9))) || // Hg206: 80206
			 ((!strncmp(szBuf, "1000812060", 9))) || // Tl206: 81206
			 ((!strncmp(szBuf, "1000822060", 9))))	 // Pb206: 82206
	{
		grpno = 15;
	}
	else if (((!strncmp(szBuf, "1000902320", 9))) || // Th232: 90232
			 ((!strncmp(szBuf, "1000882280", 9))))	 // Ra228: 88228
	{
		grpno = 21;
	}
	else if (((!strncmp(szBuf, "1000892280", 9))) || // Ac228: 89228
			 ((!strncmp(szBuf, "1000902280", 9))))	 // Th228: 90228
	{
		grpno = 22;
	}
	else if (((!strncmp(szBuf, "1000882240", 9))) || // Ra224: 88224
			 ((!strncmp(szBuf, "1000862200", 9))) || // Rn220: 86220
			 ((!strncmp(szBuf, "1000842160", 9))) || // Po216: 84216
			 ((!strncmp(szBuf, "1000822120", 9))) || // Pb212: 82212
			 ((!strncmp(szBuf, "1000832120", 9))) || // Bi212: 83212
			 ((!strncmp(szBuf, "1000842120", 9))) || // Po212: 84212
			 ((!strncmp(szBuf, "1000812080", 9))) || // Tl208: 81208
			 ((!strncmp(szBuf, "1000822080", 9))))	 // Pb208: 82208
	{
		grpno = 23;
	}
	else if (((!strncmp(szBuf, "1000190400", 9))) || // K40: 19040
			 ((!strncmp(szBuf, "1000180400", 9))) || // Ar40: 18040
			 ((!strncmp(szBuf, "1000200400", 9))))	 // Ca40: 20040
	{
		grpno = 31;
	}
	else if (((!strncmp(szBuf, "1000922350", 9))) || // U235: 92235
			 ((!strncmp(szBuf, "1000902310", 9))) || // Th231: 90231
			 ((!strncmp(szBuf, "1000912310", 9))))	 // Pa231: 91231
	{
		grpno = 41;
	}
	else if (((!strncmp(szBuf, "1000892270", 9))) || // Ac227: 89227
			 ((!strncmp(szBuf, "1000902270", 9))) || // Th227: 90227
			 ((!strncmp(szBuf, "1000872230", 9))) || // Fr223: 87223
			 ((!strncmp(szBuf, "1000882230", 9))) || // Ra223: 88223
			 ((!strncmp(szBuf, "1000852190", 9))) || // At219: 85219
			 ((!strncmp(szBuf, "1000862190", 9))) || // Rn219: 86219
			 ((!strncmp(szBuf, "1000832150", 9))) || // Bi215: 83215
			 ((!strncmp(szBuf, "1000842150", 9))) || // Po215: 84215
			 ((!strncmp(szBuf, "1000852150", 9))) || // At215: 85215
			 ((!strncmp(szBuf, "1000822110", 9))) || // Pb211: 82211
			 ((!strncmp(szBuf, "1000832110", 9))) || // Bi211: 83211
			 ((!strncmp(szBuf, "1000842110", 9))) || // Po211: 84211
			 ((!strncmp(szBuf, "1000812070", 9))) || // Tl207: 81207
			 ((!strncmp(szBuf, "1000822070", 9))))	 // Pb207: 82207
	{
		grpno = 42;
	}
	else
	{
		grpno = 0;
	}

	return grpno;
}

int main(int argc, const char *argv[])
{
	BackgroundAnlys(stoi(argv[1]), argv[2], argv[3], argv[4], stoi(argv[5]), argv[6], argv[7]);
	return 0;
}
