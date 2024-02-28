// WARNING: This script uses units of Geant4
// so, don't use a name of units as a variable name (i.e. ms, ns, mm, etc...)
#include "MuonAnlys.hxx"

using namespace std;
using namespace MCAnlysDef::AMoRE_Pilot;

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

/*! \function GetPanelNumber
 *
 *   GetPanelNumber: The function for determinating a panel number by using position
 *      coordinates.
 */
Int_t GetPanelNumber(const char *volN, Double_t x, Double_t y, Double_t z) {
    if (strstr(volN, "Top") != NULL) {
        if (z > 3640) { // kTop side
            if (x < 0.)
                return kTop1;
            else if (x > 0.)
                return kTop2;
        }
    } else if (strstr(volN, "FB") != NULL) {
        if (x >= 525.) { // kFront side
            if (y < 0.)
                return kFront1;
            else if (y > 0.)
                return kFront2;
        } else if (x <= -525) { // kBack side
            if (y < 0.)
                return kBack1;
            else if (y > 0.)
                return kBack2;
        }
    } else if (strstr(volN, "LR") != NULL) {
        if (y <= -550) { // kLeft side
            if (x > 0.)
                return kLeft1;
            else if (x < 0.)
                return kLeft2;
        } else if (y >= 550) { // kRight side
            if (x > 0.)
                return kRight1;
            else if (x < 0.)
                return kRight2;
        }
    }
    cout << "WARNING: Error scintillation number!! MOMMY! Position and name is suspicious! ("
         << volN << ": (" << x << ", " << y << ", " << z << ") )" << endl;
    return kErrorType;
}

Int_t DeterminateEvtTag(Int_t num1, Int_t num2) {
    if (num1 == -1 && num2 == -1)
        return kNoIncident;
    else if (num2 == -1) {
        if (num1 == kTop1 || num1 == kTop2)
            return kTOnly;
        else if (num1 == kLeft1 || num1 == kLeft2)
            return kLOnly;
        else if (num1 == kRight1 || num1 == kRight2)
            return kROnly;
        else if (num1 == kFront1 || num1 == kFront2)
            return kFOnly;
        else if (num1 == kBack1 || num1 == kBack2)
            return kBOnly;
    } else {
        if (num1 <= 2 && num2 >= 3)
            return kT2S;
        else if (num1 >= 3 && num2 <= 2)
            return kS2T;
        else if (num1 >= 3 && num2 >= 3)
            return kS2S;
    }
    cout << "WARNING: Error event occured!! MOMMY! Incident panel end exit panel number is "
            "suspicious! ("
         << num1 << "," << num2 << ")" << endl;
    return kErrorEvt;
}

void MuonAnlys(Int_t runno, const char *path, const char *filenameformat, const char *outpath,
               Int_t numpertask) {
#ifndef __CLING__
    cout << "COMPILED VERSION" << endl;
#endif
    // Static informations
    constexpr size_t maxcmonum        = 6;
    constexpr size_t maxscintnum      = 10;
    constexpr Double_t hit_evt_window = 200 * ms;
    constexpr Double_t hit_evt_Eth    = 0;
    constexpr Int_t flush_modular     = 10000;
    constexpr const char *tree_name   = "event_tree";
    TCMOHitInfo::SetDefaultEventWindow(hit_evt_window);
    TCMOHitInfo::SetDefaultEnergyThreshold(hit_evt_Eth);

    // File input/output variables
    TChain *fInput;
    TFile *fOutput;
    TTree *fOTree;
    Int_t fill_count = 0;

    // event_tree reader variables
    vector<TTrack *> *readEndTrack;
    EvtTrack *readTrack;
    EvtStep *readStep;
    TGSD *readTGSD;
    MuonSD *readMuon;
    TClonesArray *tclStep, *tclMuon, *tclTGSD, *tclTrack;

    TStep *readSingleStep;
    TTrack *readSingleTrack;
    TCell *readTGCell, *readMuCell;

    TBranch *branchStep, *branchMuon, *branchTGSD, *branchTrack;
    Double_t Pos[3] = {-1};

    // output tree variables
    Int_t entryID, evttag, fileID;
    Int_t maxCMOnum = maxcmonum;
    Bool_t singleHit;
    Double_t CMOTotEdep[maxcmonum]  = {0};
    Double_t ScintEdep[maxscintnum] = {0};
    Int_t panel_no[2];
    Int_t tid1_CMO_incnum;
    Double_t tid1_CMO_inctime;
    Double_t sL[2];
    Double_t Edep[2];
    Double_t I_time[2] = {0}, O_time[2] = {0};
    map<Int_t, TParticleInfo *> AssociatedParticleColl;
    std::vector<TCMOStepInfo *> CMOStepColl;
    std::vector<TCMOHitInfo *> CMOHitColl;

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
    fOTree  = new TTree("MuonAnlys", "MuonMC Production");
    fOTree->Branch("file_ID", &fileID, "file_ID/I");
    fOTree->Branch("entry_ID", &entryID, "entry_ID/I");
    fOTree->Branch("evt_type", &evttag, "evt_type/I");
    fOTree->Branch("singleHit", &singleHit, "singleHit/O");
    fOTree->Branch("I_Edep", &Edep[0], "I_Edep/D");
    fOTree->Branch("I_StepL", &sL[0], "I_StepL/D");
    fOTree->Branch("I_inTime", &I_time[0], "I_inTime/D");
    fOTree->Branch("I_outTime", &O_time[0], "I_outTime/D");
    fOTree->Branch("I_PanelNum", &panel_no[0], "I_PanelNum/I");
    fOTree->Branch("O_Edep", &Edep[1], "O_Edep/D");
    fOTree->Branch("O_StepL", &sL[1], "O_StepL/D");
    fOTree->Branch("O_inTime", &I_time[1], "O_inTime/D");
    fOTree->Branch("O_outTime", &O_time[1], "O_outTime/D");
    fOTree->Branch("O_PanelNum", &panel_no[1], "O_PanelNum/I");
    fOTree->Branch("CMOTotEdep", CMOTotEdep, Form("CMOTotEdep[%lu]/D", maxcmonum));
    fOTree->Branch("ScintEdep", ScintEdep, Form("ScintEdep[%lu]/D", maxscintnum));
    fOTree->Branch("MuonCMOIncNum", &tid1_CMO_incnum, "MuonCMOIncNum/I");
    fOTree->Branch("MuonCMOIncTime", &tid1_CMO_inctime, "MuonCMOIncTime/D");
    fOTree->Branch("ParticleColl", &AssociatedParticleColl, 1000000);
    fOTree->Branch("CMOStepColl", &CMOStepColl, 1000000);
    fOTree->Branch("CMOHitColl", &CMOHitColl, 1000000);

    // Set reader variables
    readTrack    = new EvtTrack;
    readEndTrack = new std::vector<TTrack *>;
    readStep     = new EvtStep;
    readTGSD     = new TGSD;
    readMuon     = new MuonSD;

    tclStep  = readStep->GetStep();
    tclTGSD  = readTGSD->GetCell();
    tclMuon  = readMuon->GetCell();
    tclTrack = readTrack->GetTrack();

    fInput->SetBranchAddress("STEP", &readStep, &branchStep);
    fInput->SetBranchAddress("MuonSD", &readMuon, &branchMuon);
    fInput->SetBranchAddress("TGSD", &readTGSD, &branchTGSD);
    fInput->SetBranchAddress("TRACK", &readTrack, &branchTrack);
    fInput->SetBranchAddress("EndTrack", &readEndTrack);
    enableET = (fInput->GetBranch("EndTrack") != nullptr);

    for (i = 0; i < entries; i++) { // Event loop
        fInput->GetEntry(i);

        if (entrycnt <= i) { // Job 별로 처리할 파일들 나누는 루틴
            curec = ((TTree *)fInput->GetFile()->Get(tree_name))->GetEntriesFast();
            entrycnt += curec;
            const char *now_filedesc = fInput->GetFile()->GetName();
            TObjArray *tokened_nfd   = TString(now_filedesc).Tokenize("/");
            const char *now_filename =
                static_cast<TObjString *>(tokened_nfd->At(tokened_nfd->GetEntriesFast() - 1))
                    ->String()
                    .Data();
            sscanf(now_filename, filenameformat, &fileID);
            cout << "NOWFILE: " << now_filename << " FILENO: " << fileID << " NOWINDEX: " << i
                 << " CURFILEENTRIES: " << curec << endl;
        }
        entryID = i - entrycnt + curec;

        vector<Int_t> TrackIdxLookupTbl;
        vector<Int_t> EndTrackIdxLookupTbl;

        Bool_t isInPanel   = false;
        Bool_t skipThisEvt = false;
        Int_t stepnum, tracknum;
        Double_t stepL;
        std::vector<Int_t> Track1_stepentry;

        panel_no[0] = panel_no[1] = -1;
        sL[0] = sL[1] = 0;
        I_time[0] = I_time[1] = O_time[0] = O_time[1] = 0;
        Edep[0] = Edep[1] = 0;
        tid1_CMO_incnum = tid1_CMO_inctime = -1;

        stepnum  = tclStep->GetEntries();
        tracknum = tclTrack->GetEntries();

        TrackIdxLookupTbl.resize(tracknum + 2);
        EndTrackIdxLookupTbl.resize(tracknum + 2);

        for (j = 0; j < tracknum; j++) {
            TTrack *readSingleEndTrack;
            if (enableET) readSingleEndTrack = (*readEndTrack)[j];
            readSingleTrack = static_cast<TTrack *>(tclTrack->At(j));

            if (readSingleTrack->GetTrackID() >= tracknum + 1) {
                cout << "===================" << endl;
                cout << "WARNING! TOO MANY TRACKS! : " << tracknum << " "
                     << readSingleTrack->GetTrackID() << endl;
                cout << "This event will be skipped" << endl;
                cout << "FileID: " << fileID << " | EntryID: " << entryID << endl;
                cout << "===================" << endl;
                skipThisEvt = true;
                break;
            } else {
                TrackIdxLookupTbl[readSingleTrack->GetTrackID()] = j;
                if (enableET) EndTrackIdxLookupTbl[readSingleEndTrack->GetTrackID()] = j;
            }
        }
        if (skipThisEvt) continue;

        Int_t prevTrkID = -1, prevCMONum;
        for (j = 0; j < stepnum; j++) { // 매 Step 별로 필요한 Process 들에 대한 루틴
            readSingleStep = static_cast<TStep *>(tclStep->At(j));
            Int_t nowTID   = readSingleStep->GetTrackID();
            if (nowTID == 1) Track1_stepentry.push_back(j);
            // Muon의 Step만 모음(Muon의 스텝 중간 중간에 다른 입자들의 트랙이 끼여있어서 따로처리)
            const char *volName = readSingleStep->GetVolumeName();
            // Muon까지 포함하는 모든 Step이 CMO 내에서 하는 개지랄을 보는 루틴
            if (strstr(volName, "physCMOCell") != NULL) {
                // Stepping 정보 정리하는 루틴
                Int_t cmonum;
                sscanf(volName, "physCMOCell%d", &cmonum);

                if ((nowTID != prevTrkID) || (cmonum != prevCMONum)) {
                    AddAnAssociatedParticle(AssociatedParticleColl, (*tclTrack), (*readEndTrack),
                                            TrackIdxLookupTbl, EndTrackIdxLookupTbl, nowTID,
                                            enableET);
                }
                TParticleInfo *nowParticle = AssociatedParticleColl[nowTID];
                CMOStepColl.push_back(new TCMOStepInfo(nowParticle, *readSingleStep, cmonum));
                prevTrkID  = nowTID;
                prevCMONum = cmonum;
            }
        }

        for (auto k : Track1_stepentry) {
            const char *volName;

            readSingleStep = static_cast<TStep *>(tclStep->At(k));

            volName = readSingleStep->GetVolumeName();

            if (!readSingleStep) {
                cout << "ABORTED: INVALID FILE STRUCTURE!" << endl;
                return;
            } else {
                if (!isInPanel && (strstr(volName, "Muon") != NULL &&
                                   strstr(volName, "Envelope") == NULL)) { // 안에 도달했을때 처리
                    Pos[0]    = readSingleStep->GetX();
                    Pos[1]    = readSingleStep->GetY();
                    Pos[2]    = readSingleStep->GetZ();
                    isInPanel = true;
                    Int_t num = GetPanelNumber(volName, Pos[0], Pos[1], Pos[2]);

                    // STATMENT FOR DEBUGGING (Do not care about this lines)
                    if (num == kErrorType) {
                        cout << "ERROR OCCURED: " << entryID << ", " << Pos[0] << ", " << Pos[1]
                             << ", " << Pos[2] << endl;
                        break;
                    }

                    if (panel_no[0] == -1) {
                        panel_no[0] = num;
                        I_time[0]   = readSingleStep->GetGlobalTime();
                    } else if (panel_no[1] == -1) {
                        panel_no[1] = num;
                        I_time[1]   = readSingleStep->GetGlobalTime();
                    }
                } else if (isInPanel) { // 안에 있을 때의 처리
                    if (strstr(volName, "Muon") == NULL ||
                        strstr(volName, "Envelope") != NULL) { // 밖으로 나왔을 때의 처리
                        Pos[0] -= readSingleStep->GetX();
                        Pos[1] -= readSingleStep->GetY();
                        Pos[2] -= readSingleStep->GetZ();
                        stepL = TMath::Sqrt(Pos[0] * Pos[0] + Pos[1] * Pos[1] + Pos[2] * Pos[2]);
                        if (sL[0] == 0) {
                            readMuCell =
                                static_cast<TCell *>(tclMuon->At(fcPanelMapper[panel_no[0]]));
                            sL[0]     = stepL;
                            Edep[0]   = readMuCell->GetEdep();
                            O_time[0] = readSingleStep->GetGlobalTime();
                        } else if (sL[1] == 0) {
                            readMuCell =
                                static_cast<TCell *>(tclMuon->At(fcPanelMapper[panel_no[1]]));
                            sL[1]     = stepL;
                            Edep[1]   = readMuCell->GetEdep();
                            O_time[1] = readSingleStep->GetGlobalTime();
                        }
                        isInPanel = false;
                    }
                } else if (tid1_CMO_incnum == -1 && strstr(volName, "CMO") != NULL) {
                    sscanf(volName, "physCMOCell%d", &tid1_CMO_incnum);
                    tid1_CMO_inctime = readSingleStep->GetGlobalTime();
                }
            }
            if (sL[1] != 0)
                break; // 해당 트랙이 두 번째로 만난 Scintillator panel 밖으로 나왔다면 더 이상
                       // 루프를 돌 필요 없으므로 탈출
        } // Step loop end

        // Event tag 정리, 만일 Scintillator 안에서 제거되었거나 한 경우의 처리
        evttag = DeterminateEvtTag(panel_no[0], panel_no[1]);
        if (isInPanel) {
            readSingleStep = static_cast<TStep *>(tclStep->At(Track1_stepentry.back()));
            if (strstr(readSingleStep->GetVolumeName(), "Muon") == NULL ||
                strstr(readSingleStep->GetVolumeName(), "Envelope") != NULL) {
                sL[0] = sL[1] = -2;
                Edep[0] = Edep[1] = -2;
                O_time[0] = O_time[1] = -2;
                evttag                = kErrorEvt;
                cout << "WARNING: Error event occured! MOMMY! I can't analyze steps of muon in "
                        "this event!"
                     << endl;
                continue;
            }
            Pos[0] -= readSingleStep->GetX();
            Pos[1] -= readSingleStep->GetY();
            Pos[2] -= readSingleStep->GetZ();
            stepL = TMath::Sqrt(Pos[0] * Pos[0] + Pos[1] * Pos[1] + Pos[2] * Pos[2]);
            if (sL[0] == 0) {
                readMuCell = static_cast<TCell *>(tclMuon->At(fcPanelMapper[panel_no[0]]));
                sL[0]      = stepL;
                Edep[0]    = readMuCell->GetEdep();
                O_time[0]  = readSingleStep->GetGlobalTime();
            } else {
                readMuCell = static_cast<TCell *>(tclMuon->At(fcPanelMapper[panel_no[1]]));
                sL[1]      = stepL;
                Edep[1]    = readMuCell->GetEdep();
                O_time[1]  = readSingleStep->GetGlobalTime();
            }
            isInPanel = false;
        }

        if (evttag == kErrorEvt || evttag == kNoIncident) CMOHitColl.clear();

        // CMO Edep 저장
        for (j = 0; j < maxCMOnum; j++) {
            readTGCell    = static_cast<TCell *>(tclTGSD->At(j));
            CMOTotEdep[j] = readTGCell->GetEdep();
        }

        // 들어온 경우 안내 메시지 출력부(디버깅용)
        if (evttag != kNoIncident) {
            cout << "EVENT: " << entryID << " panel_no[0]: " << panel_no[0]
                 << " panel_no[1]: " << panel_no[1] << " EVENT TYPE: " << evttag
                 << " EDEP_INC: " << Edep[0] << endl;
        } else if (CMOTotEdep[0] != 0 || CMOTotEdep[1] != 0 || CMOTotEdep[2] != 0 ||
                   CMOTotEdep[3] != 0 || CMOTotEdep[4] != 0) {
            cout << "CMOEDEP IS NOT ZERO! EVTID: " << entryID << " FILENO: " << fileID << endl;
        }

        for (k = 0; k < maxscintnum; k++) {
            readMuCell   = static_cast<TCell *>(tclMuon->At(k));
            ScintEdep[k] = readMuCell->GetEdep();
        }

        // 여기서 CMO Step이 정렬됨
        vector<Int_t> CMOStepCollSortedIdx(CMOStepColl.size(), 0);
        k = 0;
        for (auto &now : CMOStepCollSortedIdx)
            now = k++;
        std::sort(CMOStepCollSortedIdx.begin(), CMOStepCollSortedIdx.end(),
                  [&CMOStepColl](int a, int b) -> bool {
                      return CMOStepColl[a]->GetTime() < CMOStepColl[b]->GetTime();
                  });
        std::stable_sort(CMOStepCollSortedIdx.begin(), CMOStepCollSortedIdx.end(),
                         [&CMOStepColl](int a, int b) -> bool {
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
            if (nowHit->GetCMONum() != cmoNumOfPrevHit) {
                singleHit = false;
                break;
            }
        }

        if (evttag != kNoIncident || readMuon->GetTotEdep() != 0) {
            fOTree->Fill();
            if (flush_modular != 0 && ((++fill_count) % flush_modular == 0)) {
                fOTree->FlushBaskets();
                fOutput->Flush();
            }
        }
        CMOHitColl.clear();
        CMOStepColl.clear();
        AssociatedParticleColl.clear();
    } // Event loop end

    fOutput->cd();
    fOTree->Write();
    fOutput->Close();
}

int main(int, char *argv[]) {
    MuonAnlys(stoi(argv[1]), argv[2], argv[3], argv[4], stoi(argv[5]));
    return 0;
}
