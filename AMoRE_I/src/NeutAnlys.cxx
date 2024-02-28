// Neutron Analysis script for AMoRE--I
// Written by basehw(a.k.a. BaseHardware)
#include "NeutAnlys.hxx"

using namespace std;

template <typename startCType, typename endCType>
inline TParticleInfo *AddAnAssociatedParticle(map<Int_t, TParticleInfo *> &target,
                                              startCType &trackTbl, const endCType &endTrackTbl,
                                              const vector<Int_t> &idxLookupTbl,
                                              const vector<Int_t> &endIdxLookupTbl, Int_t addedTID,
                                              Bool_t endEnable = true) {
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
        TTrack *startInfo = trkReaderWithTID(nowTID);
        TParticleInfo *newParticle;
        if (endEnable) {
            TTrack *endInfo = endTrkReaderWithTID(nowTID);
            newParticle     = new TParticleInfo(*startInfo, *endInfo);
        } else
            newParticle = new TParticleInfo(*startInfo, *startInfo);
        target[nowTID] = newParticle;
        if (prevParticle != nullptr) newParticle->AddAncestors(prevParticle);
        prevParticle = newParticle;
    }
    if (prevParticle == nullptr) cout << "HOEE: " << addedTID << endl;
    return prevParticle;
}

void NeutAnlys(Int_t runno, const char *path, const char *filenameformat, const char *outpath,
               Int_t numpertask) {
#ifndef __CLING__
    cout << "COMPILED VERSION" << endl;
#endif
    // Static informations
    const Double_t hit_evt_window = 100 * ms;
    const Double_t hit_evt_Eth    = 0;
    const Int_t flush_modular     = 1000;
    const char *tree_name         = "event_tree";
    TCMOHitInfo::SetDefaultEventWindow(hit_evt_window);
    TCMOHitInfo::SetDefaultEnergyThreshold(hit_evt_Eth);

    // File input/output variables
    TChain *fInput;
    TFile *fOutput;
    TTree *fOTree;
    Int_t fill_count = 0;

    // event_tree reader variables
    EvtTrack *readTrack;
    EvtStep *readStep;
    std::vector<TTrack *> *readEndTrack;

    TClonesArray *tclTrack;
    TClonesArray *tclStep;

    TStep *readSingleStep;
    TTrack *readSingleTrack;

    // output tree variables
    Int_t fileid, entryid;
    Bool_t singleHit;
    map<Int_t, TParticleInfo *> AssociatedParticleColl;
    vector<TCMOStepInfo *> CMOStepColl;
    vector<TCMOHitInfo *> CMOHitColl;

    // control variables
    Int_t entries;
    Int_t curec, entrycnt = 0;
    Int_t i, j, k;
    Bool_t enableET;

    // input files adding
    fInput = new TChain(tree_name);
    for (i = runno * numpertask; i < (runno + 1) * numpertask; i++) {
        cout << "Adding: " << i + 1 << endl;
        fInput->Add(Form(Form("%s/%s", path, filenameformat), i));
    }
    entries = fInput->GetEntries();

    // Set output tree
    fOutput = new TFile(Form("%s/result_run%d.root", outpath, runno), "RECREATE");
    fOTree  = new TTree("NeutAnlys", "Amore-I Neutron analysis production", 5);
    fOTree->Branch("file_ID", &fileid, "file_ID/I");
    fOTree->Branch("entry_ID", &entryid, "entry_ID/I");
    fOTree->Branch("singleHit", &singleHit, "singleHit/O");
    fOTree->Branch("ParticleColl", &AssociatedParticleColl, 1000000);
    fOTree->Branch("CMOStepColl", &CMOStepColl, 1000000);
    fOTree->Branch("CMOHitColl", &CMOHitColl, 1000000);

    // Set reader variables
    readTrack    = new EvtTrack;
    readStep     = new EvtStep;
    readEndTrack = new std::vector<TTrack *>;

    tclTrack = readTrack->GetTrack();
    tclStep  = readStep->GetStep();

    fInput->SetBranchAddress("TRACK", &readTrack);
    fInput->SetBranchAddress("STEP", &readStep);
    fInput->SetBranchAddress("EndTrack", &readEndTrack);
    enableET = (fInput->GetBranch("EndTrack") != nullptr);

    for (i = 0; i < entries; i++) {
        fInput->GetEntry(i);

        Bool_t skipThisEvt = false;
        Int_t TotTrackN    = tclTrack->GetEntriesFast();
        Int_t TotStepN     = tclStep->GetEntriesFast();

        // Initialize output variables per events
        CMOStepColl.clear();
        CMOHitColl.clear();
        AssociatedParticleColl.clear();
        singleHit = true;

        // Track lookup table
        vector<Int_t> TrackIdxLookupTbl;
        vector<Int_t> EndTrackIdxLookupTbl;
        vector<vector<Int_t>> childList;

        if (entrycnt <= i) { // Job 별로 처리할 파일들 나누는 루틴
            curec = ((TTree *)fInput->GetFile()->Get(tree_name))->GetEntriesFast();
            entrycnt += curec;
            const char *now_filedesc = fInput->GetFile()->GetName();
            TObjArray *tokened_nfd   = TString(now_filedesc).Tokenize("/");
            const char *now_filename =
                static_cast<TObjString *>(tokened_nfd->At(tokened_nfd->GetEntriesFast() - 1))
                    ->String()
                    .Data();
            sscanf(now_filename, filenameformat, &fileid);
            cout << "NOWFILE: " << now_filename << " FILENO: " << fileid << " NOWINDEX: " << i
                 << " CURFILEENTRIES: " << curec << endl;
        }
        entryid = i - entrycnt + curec;

        childList.resize(TotTrackN + 2);
        TrackIdxLookupTbl.resize(TotTrackN + 2);
        EndTrackIdxLookupTbl.resize(TotTrackN + 2);

        for (j = 0; j < TotTrackN; j++) {
            TTrack *readSingleEndTrack;
            if (enableET) readSingleEndTrack = (*readEndTrack)[j];
            readSingleTrack = static_cast<TTrack *>(tclTrack->At(j));

            if (readSingleTrack->GetTrackID() >= TotTrackN + 1) {
                cout << "===================" << endl;
                cout << "WARNING! TOO MANY TRACKS! : " << TotTrackN << " "
                     << readSingleTrack->GetTrackID() << endl;
                cout << "This event will be skipped" << endl;
                cout << "FileID: " << fileid << " | EntryID: " << entryid << endl;
                cout << "===================" << endl;
                skipThisEvt = true;
                break;
            } else {
                TrackIdxLookupTbl[readSingleTrack->GetTrackID()] = j;
                if (enableET) EndTrackIdxLookupTbl[readSingleEndTrack->GetTrackID()] = j;
                childList[readSingleTrack->GetParentID()].push_back(readSingleTrack->GetTrackID());
            }
        }
        if (skipThisEvt) continue;

        // ============================== Hit generation
        for (j = 0; j < TotStepN; j++) {
            readSingleStep     = static_cast<TStep *>(tclStep->At(j));
            const Int_t nowTID = readSingleStep->GetTrackID();

            constexpr size_t bufferMaxSize      = 15;
            constexpr const char *checkerString = "_Crystal_PV";
            const char *volName                 = readSingleStep->GetVolumeName();
            const size_t volNameLength          = strlen(volName);
            const size_t checkerLength          = strlen(checkerString);

            if (volNameLength > checkerLength &&
                strcmp(volName + volNameLength - checkerLength, checkerString) == 0) {
                // Stepping 정보 정리하는 루틴
                size_t cmonum;
                char buffer[bufferMaxSize] = {'\0'};
                const char *crystalName, *crystalNumberStr;
                int crystalNumber;
                if (volNameLength - checkerLength > bufferMaxSize) {
                    Error(__func__, "Wrong Crystal PV name, cmonum will be 17. (volname: %s)",
                          volName);
                    cmonum = 17;
                }

                strncpy(buffer, volName, volNameLength - checkerLength);
                crystalName      = strtok(buffer, "_");
                crystalNumberStr = strtok(NULL, "_");
                crystalNumber    = atoi(crystalNumberStr);
                if (strcmp(crystalNameList[crystalNumber], crystalName) == 0) {
                    cmonum = crystalNumber;
                } else {
                    Error(__func__,
                          "Crystal name from fStep.VolumeName and predefined name in the list with "
                          "an index from fStep.VolumeName is different. Crystal name from fStep "
                          "variable will be ignored. (fStep.VolumeName: %s, Volume name from "
                          "fStep: %s, index: %d, "
                          "predefinedname: %s, string processing buffer: %s)",
                          volName, crystalName, crystalNumber, crystalNameList[crystalNumber],
                          buffer);
                    cmonum = crystalNumber;
                }

                TParticleInfo *nowParticle = nullptr;
                if (AssociatedParticleColl.find(nowTID) == AssociatedParticleColl.end()) {
                    nowParticle = AddAnAssociatedParticle(AssociatedParticleColl, (*tclTrack),
                                                          (*readEndTrack), TrackIdxLookupTbl,
                                                          EndTrackIdxLookupTbl, nowTID, enableET);
                } else {
                    nowParticle = AssociatedParticleColl[nowTID];
                }
                CMOStepColl.push_back(new TCMOStepInfo(nowParticle, *readSingleStep, cmonum));
            }
        }
        vector<Int_t> CMOStepCollSortedIdx(CMOStepColl.size(), 0);
        k = 0;
        for (auto &now : CMOStepCollSortedIdx)
            now = k++;

        sort(CMOStepCollSortedIdx.begin(), CMOStepCollSortedIdx.end(),
             [&CMOStepColl](const Int_t &a, const Int_t &b) -> bool {
                 return CMOStepColl[a]->GetTime() < CMOStepColl[b]->GetTime();
             });
        stable_sort(CMOStepCollSortedIdx.begin(), CMOStepCollSortedIdx.end(),
                    [&CMOStepColl](const Int_t &a, const Int_t &b) -> bool {
                        return CMOStepColl[a]->GetCMONum() < CMOStepColl[b]->GetCMONum();
                    });

        CMOHitColl.push_back(new TCMOHitInfo(hit_evt_window, hit_evt_Eth));

        for (auto nowIdx : CMOStepCollSortedIdx) {
            TCMOStepInfo *nowStep = CMOStepColl[nowIdx];
            TCMOHitInfo *nowHit   = CMOHitColl.back();
            if (!(nowHit->AddStep(nowStep))) {
                if (nowHit->TerminateHit())
                    CMOHitColl.push_back(new TCMOHitInfo(hit_evt_window, hit_evt_Eth));
                else
                    nowHit->Clear();
                CMOHitColl.back()->AddStep(nowStep);
            }
        }

        if (!(CMOHitColl.back()->TerminateHit())) {
            CMOHitColl.back()->Clear();
            CMOHitColl.pop_back();
        }

        Int_t cmoNumOfPrevHit = (*CMOHitColl.begin())->GetCMONum();
        singleHit             = true;
        for (auto &nowHit : CMOHitColl) {
            if (nowHit->GetCMONum() != cmoNumOfPrevHit) singleHit = false;
        }

        if (AssociatedParticleColl.size() != 0) {
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

int main(int argc, char *argv[]) {
    NeutAnlys(atoi(argv[1]), argv[2], argv[3], argv[4], atoi(argv[5]));
}
