// Neutron Analysis script for AMoRE--Pilot
// Written by basehw(a.k.a. BaseHardware)
#include "NeutAnlys.hxx"

using namespace std;

bool IsOutsideOfInnerDetector(const char *inp) {
    return strcmp("physHall", inp) == 0 || strcmp("physRock", inp) == 0 ||
           strcmp("physRockCavity", inp) == 0 || strcmp("physB4CRubber", inp) == 0 ||
           strcmp("physWorkArea", inp) == 0 || strstr(inp, "TopPb") != nullptr ||
           strstr(inp, "PolyEt") != nullptr || strstr(inp, "Vert") != nullptr ||
           strstr(inp, "PhyH") != nullptr || strstr(inp, "Hori") != nullptr ||
           strstr(inp, "Mu") != nullptr || strstr(inp, "Scint") != nullptr ||
           strstr(inp, "pmt") != nullptr;
}

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
    TGSD *readTGSD;
    EvtTrack *readTrack;
    EvtStep *readStep;
    std::vector<TTrack *> *readEndTrack;

    TClonesArray *tclCell;
    TClonesArray *tclTrack;
    TClonesArray *tclStep;

    TStep *readSingleStep;
    TTrack *readSingleTrack;

    // output tree variables
    Int_t fileid, entryid;
    Bool_t singleHit, primEnteredIntoWorkArea;
    TString primStatus;
    map<Int_t, TParticleInfo *> AssociatedParticleColl;
    vector<TCMOStepInfo *> CMOStepColl;
    vector<TCMOHitInfo *> CMOHitColl;

    // control variables
    Int_t entries;
    Int_t curec, entrycnt = 0;
    Int_t i, j, k;

    // input files adding
    fInput = new TChain(tree_name);
    for (i = runno * numpertask; i < (runno + 1) * numpertask; i++) {
        cout << "Adding: " << i + 1 << endl;
        fInput->Add(Form(Form("%s/%s", path, filenameformat), i));
    }
    entries = fInput->GetEntries();

    // Set output tree
    fOutput = new TFile(Form("%s/result_run%d.root", outpath, runno), "RECREATE");
    fOTree  = new TTree("NeutAnlys", "AmorePilot Neutron analysis production", 5);
    fOTree->Branch("file_ID", &fileid, "file_ID/I");
    fOTree->Branch("entry_ID", &entryid, "entry_ID/I");
    fOTree->Branch("singleHit", &singleHit, "singleHit/O");
    fOTree->Branch("primEnteredWorkArea", &primEnteredIntoWorkArea, "primEnteredWorkArea/O");
    fOTree->Branch("primStatus", &primStatus);
    fOTree->Branch("ParticleColl", &AssociatedParticleColl, 1000000);
    fOTree->Branch("CMOStepColl", &CMOStepColl, 1000000);
    fOTree->Branch("CMOHitColl", &CMOHitColl, 1000000);

    // Set reader variables
    readTGSD     = new TGSD;
    readTrack    = new EvtTrack;
    readStep     = new EvtStep;
    readEndTrack = new std::vector<TTrack *>;

    tclCell  = readTGSD->GetCell();
    tclTrack = readTrack->GetTrack();
    tclStep  = readStep->GetStep();

    fInput->SetBranchAddress("TGSD", &readTGSD);
    fInput->SetBranchAddress("TRACK", &readTrack);
    fInput->SetBranchAddress("STEP", &readStep);
    fInput->SetBranchAddress("EndTrack", &readEndTrack);

    for (i = 0; i < entries; i++) {
        fInput->GetEntry(i);

        Bool_t skipThisEvt = false;
        Int_t TotTrackN    = tclTrack->GetEntriesFast();
        Int_t TotStepN     = tclStep->GetEntriesFast();

        // Initialize output variables per events
        CMOStepColl.clear();
        CMOHitColl.clear();
        AssociatedParticleColl.clear();
        singleHit               = true;
        primEnteredIntoWorkArea = false;
        primStatus              = "NotEvaluated";

        // Track lookup table
        vector<Int_t> TrackIdxLookupTbl;
        vector<Int_t> EndTrackIdxLookupTbl;
        vector<Int_t> TrackStartsAt;
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

        TrackStartsAt.resize(TotTrackN + 2);
        childList.resize(TotTrackN + 2);
        fill(TrackStartsAt.begin(), TrackStartsAt.end(), -1);
        TrackIdxLookupTbl.resize(TotTrackN + 2);
        EndTrackIdxLookupTbl.resize(TotTrackN + 2);

        for (j = 0; j < TotTrackN; j++) {
            auto readSingleEndTrack = (*readEndTrack)[j];
            readSingleTrack         = static_cast<TTrack *>(tclTrack->At(j));

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
                TrackIdxLookupTbl[readSingleTrack->GetTrackID()]       = j;
                EndTrackIdxLookupTbl[readSingleEndTrack->GetTrackID()] = j;
                childList[readSingleTrack->GetParentID()].push_back(readSingleTrack->GetTrackID());
            }
        }
        if (skipThisEvt) continue;

        // ============================== Primary position

        Int_t nowPrimStatus = -1;
        readSingleTrack     = (*readEndTrack)[EndTrackIdxLookupTbl[1]];
        auto deadVolName    = readSingleTrack->GetVolumeName();

        for (j = 0; j < TotStepN; j++) { // For TID=1 particle (Primary)
            readSingleStep     = static_cast<TStep *>(tclStep->At(j));
            const Int_t nowTID = readSingleStep->GetTrackID();
            auto nowVolName    = readSingleStep->GetVolumeName();
            if (strcmp(nowVolName, "physWorkArea") == 0) primEnteredIntoWorkArea = true;
            if (nowTID != 1) break;
        }

        if (IsOutsideOfInnerDetector(deadVolName)) {
            primStatus       = "Outside,";
            Bool_t nowInside = false;

            for (; j > 0; j--) {
                readSingleStep  = static_cast<TStep *>(tclStep->At(j - 1));
                auto nowVolName = readSingleStep->GetVolumeName();
                if (!nowInside && !IsOutsideOfInnerDetector(nowVolName)) {
                    primStatus += "EXT:";
                    primStatus += nowVolName;
                    nowInside = true;
                } else if (nowInside && IsOutsideOfInnerDetector(nowVolName)) {
                    readSingleStep = static_cast<TStep *>(tclStep->At(j));
                    nowVolName     = readSingleStep->GetVolumeName();
                    primStatus += ",ENT:";
                    primStatus += nowVolName;
                    break;
                }
            }

            if (j == 0) {
                if (nowInside == true)
                    primStatus += "Error";
                else
                    primStatus += "NeverEntered";
            }
        } else {
            for (; j > 0; j--) {
                readSingleStep  = static_cast<TStep *>(tclStep->At(j - 1));
                auto nowVolName = readSingleStep->GetVolumeName();
                if (IsOutsideOfInnerDetector(nowVolName)) {
                    readSingleStep = static_cast<TStep *>(tclStep->At(j));
                    nowVolName     = readSingleStep->GetVolumeName();
                    primStatus     = "ENT:";
                    primStatus += nowVolName;
                    nowPrimStatus = 0;
                    break;
                }
            }
            if (nowPrimStatus == -1) primStatus += "Unknown";
        }

        // ============================== Hit generation

        Int_t prevTrkID = -1, prevCMONum = -1;

        for (j = 0; j < TotStepN; j++) {
            readSingleStep     = static_cast<TStep *>(tclStep->At(j));
            const Int_t nowTID = readSingleStep->GetTrackID();

            if (TrackStartsAt[nowTID] == -1) TrackStartsAt[nowTID] = j;

            if (strstr(readSingleStep->GetVolumeName(), "physCMOCell") != NULL) {
                // Stepping 정보 정리하는 루틴
                Int_t cmonum;
                sscanf(readSingleStep->GetVolumeName(), "physCMOCell%d", &cmonum);
                if ((nowTID != prevTrkID) || (cmonum != prevCMONum)) {
                    AddAnAssociatedParticle(AssociatedParticleColl, (*tclTrack), (*readEndTrack),
                                            TrackIdxLookupTbl, EndTrackIdxLookupTbl, nowTID);
                }
                TParticleInfo *nowParticle = AssociatedParticleColl[nowTID];
                CMOStepColl.push_back(new TCMOStepInfo(nowParticle, *readSingleStep, cmonum));
                prevTrkID  = nowTID;
                prevCMONum = cmonum;
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
