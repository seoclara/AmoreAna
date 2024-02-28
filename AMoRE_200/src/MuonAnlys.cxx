// Muon analysis code for design-R2
#include "BackgroundAnlys.hxx"

using namespace std;

Int_t g_entryID; // Variables for debug output
Int_t g_fileID;

// Static informations
const Double_t hit_evt_window = 100 * ms;
const Double_t hit_evt_Eth    = 0;
const Int_t flush_modular     = 1000;
const char *tree_name         = "event_tree";

template <typename startCType, typename endCType>
inline TParticleInfo *
AddAnAssociatedParticle(map<Int_t, TParticleInfo *> &target, startCType &trackTbl,
		const endCType &endTrackTbl, const vector<Int_t> &idxLookupTbl,
		const vector<Int_t> &endIdxLookupTbl, Int_t addedTID) {
	Int_t examingTID      = addedTID;
	auto trkReaderWithTID = [&trackTbl, &idxLookupTbl](Int_t TID) -> TTrack * {
		return static_cast<TTrack *>((trackTbl[idxLookupTbl[TID]]));
	};
	auto endTrkReaderWithTID = [&endTrackTbl, &endIdxLookupTbl](Int_t TID) -> TTrack * {
		return static_cast<TTrack *>(endTrackTbl[endIdxLookupTbl[TID]]);
	};
	list<Int_t> history;
	map<Int_t, TParticleInfo *>::iterator findRes = target.find(examingTID);
	while (examingTID && findRes == target.end()) {
		history.push_front(examingTID);
		examingTID = trkReaderWithTID(examingTID)->GetParentID();
		findRes    = target.find(examingTID);
	}
	TParticleInfo *prevParticle = (examingTID == 0) ? nullptr : (*findRes).second;
	for (auto nowTID : history) {
		TTrack *startInfo          = trkReaderWithTID(nowTID);
		TTrack *endInfo            = endTrkReaderWithTID(nowTID);
		TParticleInfo *newParticle = new TParticleInfo(*startInfo, *endInfo);
		target[nowTID]             = newParticle;
		if (prevParticle != nullptr) newParticle->AddAncestors(prevParticle);
		prevParticle = newParticle;
	}
	if (prevParticle == nullptr) cout << "HOEE: " << addedTID << endl;
	return prevParticle;
}

void MuonAnlys(const int runno, const char *path, const char *fileformat, const char *outpath, const int numpertask) 
{
#ifndef __CLING__
	cout << "COMPILED VERSION" << endl;
#endif

	////// INPUT FILE VARIABLES ===============================
	TChain *fInput = new TChain(tree_name);
	for (Int_t i = runno * numpertask; i < (runno + 1) * numpertask; i++) {
		cout << "Adding: " << i + 1 << endl;
		fInput->Add(Form(Form("%s/%s", path, fileformat), i));
	}

	TGSD *readTGSD      = new TGSD;
	EvtTrack *readTrack = new EvtTrack;
	EvtStep *readStep   = new EvtStep;
	EvtInfo *readInfo   = new EvtInfo;
	vector<TTrack *> *readEndTrack = new std::vector<TTrack *>;

	fInput->SetBranchAddress("EVENTINFO", &readInfo);
	fInput->SetBranchAddress("TGSD", &readTGSD);
	fInput->SetBranchAddress("TRACK",&readTrack);
	fInput->SetBranchAddress("STEP", &readStep);
	fInput->SetBranchAddress("EndTrack", &readEndTrack);

	TClonesArray *tclCell  = readTGSD->GetCell();
	TClonesArray *tclStep  = readStep->GetStep();
	TClonesArray *tclTrack = readTrack->GetTrack();

	TStep *readSingleStep;
	TTrack *readSingleTrack;

	////// OUTPUT FILE VARIABLES ================================
	Int_t fileID, entryID, eventID;
	Double_t edepCMO;
	Double_t edepWT;
	Double_t edepPS[number_of_scintillators];
	Bool_t VetoTouched, WTTouched, CMOHitted, SingleCMOHit;
	map<Int_t, TParticleInfo *> AssociatedParticleColl;
	vector<TCMOStepInfo *> CMOStepColl;
	vector<TCMOHitInfo *> CMOHitColl;
	TObjArray StartAtWT;
	TObjArray EndAtWT;
	TObjArray StartAtVeto;
	TObjArray EndAtVeto;

	TFile *fOutput = new TFile(Form("%s/result_run%d.root", outpath, runno), "RECREATE");
	TTree *fOTree  = new TTree("MuonAnlys", "Muon analysis production", 5);
	//// file information
	fOTree->Branch("FileID", &fileID, "FileID/I");
	fOTree->Branch("EntryID", &entryID, "EntryID/I");
	fOTree->Branch("EventID", &eventID, "EventID/I");
	fOTree->Branch("EdepCMO", &edepCMO, "EdepCMO/D");
	fOTree->Branch("EdepWT", &edepWT, "EdepWT/D");
	fOTree->Branch(Form("EdepPS[%d]",number_of_scintillators), &edepPS,Form("EdepPS[%d]/D",number_of_scintillators));
	fOTree->Branch("WTTouched", &WTTouched, "WTTouched/O");
	fOTree->Branch("VetoTouched", &VetoTouched, "VetoTouched/O");
	fOTree->Branch("CMOHitted", &CMOHitted, "CMOHitted/O");
	fOTree->Branch("SingleCMOHit", &SingleCMOHit, "SingleCMOHit/O");
	fOTree->Branch("ParticleColl", &AssociatedParticleColl, 1000000); // Collection of CMO steps
	fOTree->Branch("CMOStepColl", &CMOStepColl, 1000000);             // Collection of CMO steps
	fOTree->Branch("CMOHitColl", &CMOHitColl, 1000000);               // Collection of CMO hits
	fOTree->Branch("StartAtWT", &StartAtWT);
	fOTree->Branch("EndAtWT", &EndAtWT);
	fOTree->Branch("StartAtVeto", &StartAtVeto);
	fOTree->Branch("EndAtVeto", &EndAtVeto);

	////// START EVENT LOOP ======================================
	Int_t fill_count     = 0;
	Int_t curec, entrycnt = 0;
	Int_t CellTotNum = -1;
	Int_t i, j, k;

	Int_t entries = fInput->GetEntries();

	if (entries > 0) { //// Getting the number of crystal cells
		fInput->GetEntry(0);
		CellTotNum = tclCell->GetEntries();
	}
	for (i = 0; i < entries; i++) { //// start event loop
		fInput->GetEntry(i);
		
		//// reset and initialize the variable for each events
		for (auto nowAP : AssociatedParticleColl)
			delete nowAP.second;
		AssociatedParticleColl.clear();

		for (auto nowStep : CMOStepColl)
			delete nowStep;
		CMOStepColl.clear();

		for (auto nowHit : CMOHitColl)
			delete nowHit;
		CMOHitColl.clear();

		StartAtWT.Clear();
		EndAtWT.Clear();
		StartAtVeto.Clear();
		EndAtVeto.Clear();

		WTTouched  = false;
		VetoTouched  = false;
		CMOHitted    = false;
		SingleCMOHit = true;

		edepCMO = 0.;
		edepWT = 0.;
		for(j = 0; j < number_of_scintillators; j++)
		{
			edepPS[j] = 0.;
		}

		Int_t TotTrackN = tclTrack->GetEntriesFast();
		Int_t TotStepN = tclStep->GetEntries();

		//// File and event ID numbering_________________________
		if (entrycnt <= i) { 
			curec = ((TTree *)fInput->GetFile()->Get(tree_name))->GetEntriesFast();
			entrycnt += curec;
			const char *now_filedesc = fInput->GetFile()->GetName();
			TObjArray *tokened_nfd   = TString(now_filedesc).Tokenize("/");
			const char *now_filename =
				static_cast<TObjString *>(tokened_nfd->At(tokened_nfd->GetEntriesFast() - 1))
				->String()
				.Data();
			sscanf(now_filename, fileformat, &fileID);
			g_fileID = fileID;
			cout << "NOWFILE: " << now_filename << " FILENO: " << fileID << " NOWINDEX: " << i
				<< " CURFILEENTRIES: " << curec << endl;
		}
		entryID   = i - entrycnt + curec;
		g_entryID = entryID;
		eventID = readInfo->GetEventID();

		//// start looking track information _________________________
		vector<Int_t> TrackIdxLookupTbl;
		vector<Int_t> EndTrackIdxLookupTbl;
		vector<Int_t> TrackStartsAt;
		vector<vector<int>> ChildrenList;

		if (TotStepN > 0) {
			Bool_t skipThisEvt = false;
			CMOHitted          = true;

			TrackStartsAt.resize(TotTrackN + 2);
			fill(TrackStartsAt.begin(), TrackStartsAt.end(), -1);

			TrackIdxLookupTbl.resize(TotTrackN + 2);
			EndTrackIdxLookupTbl.resize(TotTrackN + 2);
			ChildrenList.resize(TotTrackN + 2);

			for (j = 0; j < TotTrackN; j++) {
				auto readSingleEndTrack = (*readEndTrack)[j];
				readSingleTrack         = static_cast<TTrack *>(tclTrack->At(j));

				if (readSingleTrack->GetTrackID() >= TotTrackN + 1) {
					cout << "===================" << endl;
					cout << "WARNING! TOO MANY TRACKS! : " << TotTrackN << " "
						<< readSingleTrack->GetTrackID() << endl;
					cout << "This event will be skipped" << endl;
					cout << "FileID: " << fileID << " | EntryID: " << entryID << endl;
					cout << "===================" << endl;
					skipThisEvt = true;
					break;
				} else {
					TrackIdxLookupTbl[readSingleTrack->GetTrackID()]       = j;
					EndTrackIdxLookupTbl[readSingleEndTrack->GetTrackID()] = j;
					ChildrenList[readSingleTrack->GetParentID()].push_back(
							readSingleTrack->GetTrackID());
				}
				//// saving particles which are started at Water tank
				if (strstr(readSingleTrack->GetVolumeName(), "HatWaterTank") != nullptr) {
					TParticleInfo *addedParticle = AddAnAssociatedParticle(
							AssociatedParticleColl, (*tclTrack), (*readEndTrack), TrackIdxLookupTbl,
							EndTrackIdxLookupTbl, readSingleTrack->GetTrackID());
					TRef *newAddedParticleRef = new TRef(addedParticle);
					StartAtWT.AddLast(newAddedParticleRef);
				}
				//// saving particles which are ended at Water tank
				if (strstr(readSingleEndTrack->GetVolumeName(), "HatWaterTank") != nullptr) {
					TParticleInfo *addedParticle = AddAnAssociatedParticle(
							AssociatedParticleColl, (*tclTrack), (*readEndTrack), TrackIdxLookupTbl,
							EndTrackIdxLookupTbl, readSingleTrack->GetTrackID());
					TRef *newAddedParticleRef = new TRef(addedParticle);
					EndAtWT.AddLast(newAddedParticleRef);
				}
				//// saving particles which are started at Plastic scintillator
				if (strstr(readSingleTrack->GetVolumeName(), "PlasticScint") != nullptr) {
					TParticleInfo *addedParticle = AddAnAssociatedParticle(
							AssociatedParticleColl, (*tclTrack), (*readEndTrack), TrackIdxLookupTbl,
							EndTrackIdxLookupTbl, readSingleTrack->GetTrackID());
					TRef *newAddedParticleRef = new TRef(addedParticle);
					StartAtVeto.AddLast(newAddedParticleRef);
				}
				//// saving particles which are ended at Plastic scintillator
				if (strstr(readSingleEndTrack->GetVolumeName(), "PlasticScint") != nullptr) {
					TParticleInfo *addedParticle = AddAnAssociatedParticle(
							AssociatedParticleColl, (*tclTrack), (*readEndTrack), TrackIdxLookupTbl,
							EndTrackIdxLookupTbl, readSingleTrack->GetTrackID());
					TRef *newAddedParticleRef = new TRef(addedParticle);
					EndAtVeto.AddLast(newAddedParticleRef);
				}
			}
			if (skipThisEvt) continue;

			//// save the edep on the crystal
			edepCMO = readTGSD->GetTotEdep();

			//// start looking STEP information _______________________________
			Int_t prevTrkID = -1, prevCMONum = -1;
			Int_t nowExamTrkID    = -1;
			Int_t prevExamedTrkID = -1;

			for (j = 0; j < TotStepN; j++) {
				readSingleStep     = static_cast<TStep *>(tclStep->At(j));
				const Int_t nowTID = readSingleStep->GetTrackID();

				if(readSingleStep->GetTrackID() == 1 && strstr(readSingleStep->GetVolumeName(), "HatWaterTank")) WTTouched = true;
				if(readSingleStep->GetTrackID() == 1 && strstr(readSingleStep->GetVolumeName(), "PlasticScint")) VetoTouched = true;

				if (TrackStartsAt[nowTID] == -1) TrackStartsAt[nowTID] = j;

				//// Edep on veto material recording 
				if(strstr(readSingleStep->GetVolumeName(), "HatWaterTank") && readSingleStep->GetEnergyDeposit()>0){
					edepWT += readSingleStep->GetEnergyDeposit();
				} else if(strstr(readSingleStep->GetVolumeName(), "PlasticScint") && readSingleStep->GetEnergyDeposit()>0){
					TString volumename = readSingleStep->GetVolumeName();
					Ssiz_t upto = volumename.First("_");
					int end = upto-13;
					TString found( volumename(13,end) );
					int id = found.Atoi();
					edepPS[id] += readSingleStep->GetEnergyDeposit();
				}
				
				//// save the associated particle which make cmo hit
				if (strstr(readSingleStep->GetVolumeName(), "physCrystalCell") != NULL) {
					Int_t cmonum;
					sscanf(readSingleStep->GetVolumeName(), "physCrystalCell%d", &cmonum);

					if ((nowTID != prevTrkID) || (cmonum != prevCMONum)) {
						AddAnAssociatedParticle(AssociatedParticleColl, (*tclTrack),
								(*readEndTrack), TrackIdxLookupTbl,
								EndTrackIdxLookupTbl, nowTID);
					}
					TParticleInfo *nowParticle = AssociatedParticleColl[nowTID];
					CMOStepColl.push_back(new TCMOStepInfo(nowParticle, *readSingleStep, cmonum));
					prevTrkID  = nowTID;
					prevCMONum = cmonum;
				}
			}

			//// SAVE the step and particle collection _______________________
			vector<Int_t> CMOStepCollSortedIdx(CMOStepColl.size(), 0);
			k = 0;
			for (auto &now : CMOStepCollSortedIdx) now = k++;
			sort(CMOStepCollSortedIdx.begin(), CMOStepCollSortedIdx.end(),
					[&CMOStepColl](int a, int b) -> bool {
					return CMOStepColl[a]->GetTime() < CMOStepColl[b]->GetTime();
					});
			stable_sort(CMOStepCollSortedIdx.begin(), CMOStepCollSortedIdx.end(),
					[&CMOStepColl](int a, int b) -> bool {
					return CMOStepColl[a]->GetCMONum() < CMOStepColl[b]->GetCMONum();
					});

			CMOHitColl.push_back(new TCMOHitInfo(hit_evt_window, hit_evt_Eth));
			for(auto &nowStep : CMOStepColl) 
			{
				TCMOHitInfo *nowHit = CMOHitColl.back();
				if(!(nowHit->AddStep(nowStep))) 
				{
					if(nowHit->TerminateHit()) CMOHitColl.push_back(new TCMOHitInfo(hit_evt_window, hit_evt_Eth));
					else nowHit->Clear();
					CMOHitColl.back()->AddStep(nowStep);
				}
			}

			if(!(CMOHitColl.back()->TerminateHit())) 
			{
				CMOHitColl.back()->Clear();
				CMOHitColl.pop_back();
			}
		}

		Int_t cmoNumOfPrevHit = (*CMOHitColl.begin())->GetCMONum();
		for(auto &nowHit : CMOHitColl) {
			if(nowHit->GetCMONum() != cmoNumOfPrevHit) SingleCMOHit = false;
		}

		if(AssociatedParticleColl.size() != 0) {
			fOTree->Fill();
			if (flush_modular != 0 && ((++fill_count) % flush_modular == 0)) {
				fOTree->FlushBaskets();
				fOutput->Flush();
			}
		}
	}
	fOTree->Write();
	fOutput->Close();
	cout << "END" << endl;
}

int main(int, char *argv[]) {
	MuonAnlys(std::stoi(argv[1]), argv[2], argv[3], argv[4], stoi(argv[5]));
	return 0;
}
