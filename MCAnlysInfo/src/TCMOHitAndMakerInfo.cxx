#include "TCMOHitAndMakerInfo.hxx"
#include "TCMOHitMakerInfo.hxx"
#include "TCMOStepInfo.hxx"
#include "TParticleInfo.hxx"

#include "MCObjs/TStep.hh"

#include <TObjArray.h>
#include <TRef.h>

#include <iostream>

using namespace std;

Int_t TCMOHitAndMakerInfo::fgDefault_number_of_border = 1;

ClassImp(TCMOHitAndMakerInfo);

TCMOHitAndMakerInfo::TCMOHitAndMakerInfo()
    : TCMOHitInfo(), fEdepOfAParticle(), fcNumberOfBorders(-1){};

TCMOHitAndMakerInfo::TCMOHitAndMakerInfo(Double_t window, Double_t e_th, Int_t borderNum,
                                         const char **names)
    : TCMOHitInfo(window, e_th), fEdepOfAParticle(), fcNumberOfBorders(borderNum) {
    for (int i = 0; i < fcNumberOfBorders; i++) {
        fBorderNames.push_back(TString());
        fHitMakerIdx.push_back(vector<Int_t>());
        fHitMakerArrays.AddLast(new TObjArray);
    }
    if (names != nullptr) SetBorderNames(names);
};

TCMOHitAndMakerInfo::TCMOHitAndMakerInfo(Double_t window, Double_t e_th, Int_t borderNum,
                                         const TString *names)
    : TCMOHitInfo(window, e_th), fEdepOfAParticle(), fcNumberOfBorders(borderNum) {
    for (int i = 0; i < fcNumberOfBorders; i++) {
        fBorderNames.push_back(TString());
        fHitMakerIdx.push_back(vector<Int_t>());
        fHitMakerArrays.AddLast(new TObjArray);
    }
    if (names != nullptr) SetBorderNames(names);
};

TCMOHitAndMakerInfo::TCMOHitAndMakerInfo(Int_t borderNum, const char **names)
    : TCMOHitInfo(), fEdepOfAParticle(), fcNumberOfBorders(borderNum) {
    for (int i = 0; i < fcNumberOfBorders; i++) {
        fBorderNames.push_back(TString());
        fHitMakerIdx.push_back(vector<Int_t>());
        fHitMakerArrays.AddLast(new TObjArray);
    }
    if (names != nullptr) SetBorderNames(names);
};

TCMOHitAndMakerInfo::TCMOHitAndMakerInfo(Int_t borderNum, const TString *names)
    : TCMOHitInfo(), fEdepOfAParticle(), fcNumberOfBorders(borderNum) {
    for (int i = 0; i < fcNumberOfBorders; i++) {
        fBorderNames.push_back(TString());
        fHitMakerIdx.push_back(vector<Int_t>());
        fHitMakerArrays.AddLast(new TObjArray);
    }
    if (names != nullptr) SetBorderNames(names);
};

void TCMOHitAndMakerInfo::Clear(Option_t *opt) {
    for (int i = 0; i < fcNumberOfBorders; i++) {
        TObjArray *nowArray = static_cast<TObjArray *>(fHitMakerArrays[i]);
        for (auto j : *nowArray) {
            TCMOHitMakerInfo *nowInfo = static_cast<TCMOHitMakerInfo *>(j);
            nowInfo->Clear(opt);
        }
        nowArray->Clear(opt);
        fBorderNames[i].Clear();
        fHitMakerIdx[i].clear();
    }
    fEdepOfAParticle.clear();
    TCMOHitInfo::Clear(opt);
}

Int_t TCMOHitAndMakerInfo::FindHitMakerInList(TTIListIter_type iter_begin,
                                              TTIListIter_type iter_end,
                                              TParticleInfo *&targetParticle) {
    TParticleInfo *nowParticle = targetParticle;
    while (nowParticle != nullptr) {
        Int_t i = 0;
        for (auto now_iter = iter_begin; now_iter != iter_end; ++now_iter) {
            const TParticleInfo &nowInci = (*now_iter)->GetAssociatedParticle();
            if (nowInci == (*nowParticle)) {
                targetParticle = nowParticle;
                return i;
            }
            i++;
        }
        nowParticle = nowParticle->GetParent();
    }
    return -1;
}

Int_t TCMOHitAndMakerInfo::TerminateHit(TTIList_type *inciList, Int_t &failIdx) {
    Bool_t overETh = IsOverEnergyThreshold();
    if (!IsTerminated()) {
        overETh = TCMOHitInfo::TerminateHit();

        CalculateEdepArray();
        failIdx = GenerateHitMaker(inciList);
        if (failIdx == fcNumberOfBorders)
            failIdx = -1;
        else
            return -1;
    }
    return (overETh) ? 1 : 0;
}

void TCMOHitAndMakerInfo::CalculateEdepArray() {
    const Int_t numberOfParticles = fParticleRefList.GetEntries();
    fEdepOfAParticle              = vector<Double_t>(numberOfParticles, 0.);

    for (auto nowObject : fStepRefList) {
        TRef *nowStepRef      = static_cast<TRef *>(nowObject);
        TCMOStepInfo *nowStep = static_cast<TCMOStepInfo *>(nowStepRef->GetObject());
        TParticleInfo *findRes;
        Int_t findIdx = TParticleInfo::FindParticleInTCollectionOfTRef(
            fParticleRefList, &nowStep->GetParticle(), findRes);
        fEdepOfAParticle[findIdx] += nowStep->GetEdep();
    }
}

Int_t TCMOHitAndMakerInfo::GenerateHitMaker(TTIList_type *inciList) {
    Int_t i = 0;
    for (auto nowObject : fParticleRefList) {
        TRef *nowParticleRef       = static_cast<TRef *>(nowObject);
        TParticleInfo *nowParticle = static_cast<TParticleInfo *>(nowParticleRef->GetObject());
        TParticleInfo *moving_ParticlePointer = nowParticle;
        for (Int_t nowBorderIdx = fcNumberOfBorders - 1; nowBorderIdx >= 0; nowBorderIdx--) {
            TTIList_type nowList = inciList[nowBorderIdx];
            Int_t findIdx =
                FindHitMakerInList(nowList.begin(), nowList.end(), moving_ParticlePointer);
            if (findIdx == -1)
                return nowBorderIdx;
            else
                CheckAndAddHitmakerInfo(nowBorderIdx, nowList[findIdx], fEdepOfAParticle[i],
                                        nowParticle);
        }
        i++;
    }
    return fcNumberOfBorders;
}

void TCMOHitAndMakerInfo::CheckAndAddHitmakerInfo(Int_t arrayIdx, TTrackInfo *&targetHM,
                                                  Double_t edepOfDaug, TParticleInfo *aDaughter) {
    TCMOHitMakerInfo *findRes;
    TObjArray &nowArray = *static_cast<TObjArray *>(fHitMakerArrays[arrayIdx]);

    TCMOHitMakerInfo::FindHitMakerInTCollection(nowArray, &targetHM->GetAssociatedParticle(),
                                                findRes);
    if (findRes == nullptr) {
        TCMOHitMakerInfo *nowHM = new TCMOHitMakerInfo(*targetHM);
        nowHM->AddEdep(edepOfDaug);
        fHitMakerIdx[arrayIdx].push_back(nowArray.GetEntries());
        if (nowHM->AppendDaughter(aDaughter) == -2)
            cerr << "TCMOHitAndMakerInfo::CheckAndAddHitmakerInfo says: AppendDaughter has been "
                    "failed"
                 << endl;
        nowArray.AddLast(nowHM);
    } else {
        findRes->AddEdep(edepOfDaug);
        if (findRes->AppendDaughter(aDaughter) == -2)
            cerr << "TCMOHitAndMakerInfo::CheckAndAddHitmakerInfo says: AppendDaughter has been "
                    "failed"
                 << endl;
    }
}
