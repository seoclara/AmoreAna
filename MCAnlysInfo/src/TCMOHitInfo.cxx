#include "TCMOHitInfo.hxx"
#include "TCMOStepInfo.hxx"

#include "MCObjs/TStep.hh"

#include <TRef.h>
#include <iostream>

using namespace std;

Double_t TCMOHitInfo::fgDefault_event_window     = 1;
Double_t TCMOHitInfo::fgDefault_energy_threshold = 0;

ClassImp(TCMOHitInfo);

TCMOHitInfo::TCMOHitInfo()
    : TObject(), fcEvent_window(fgDefault_event_window),
      fcEnergy_threshold(fgDefault_energy_threshold), fFourPos_0(0, 0, 0, 0),
      fFourPos_Mean(0, 0, 0, 0), fEdep(0), fCMONum(-1), fNeutAssociated(false), fTerminated(false),
      fParticleRefList(), fStepRefList(){};

TCMOHitInfo::TCMOHitInfo(Double_t window, Double_t e_th)
    : TObject(), fcEvent_window(window), fcEnergy_threshold(e_th), fFourPos_0(0, 0, 0, 0),
      fFourPos_Mean(0, 0, 0, 0), fEdep(0), fCMONum(-1), fNeutAssociated(false), fTerminated(false),
      fParticleRefList(), fStepRefList(){};

void TCMOHitInfo::Clear(Option_t *opt) {
    TObject::Clear(opt);
    fFourPos_0.Clear(opt);
    fFourPos_Mean.Clear(opt);
    fFourPos_0.SetXYZT(0, 0, 0, 0);
    fFourPos_Mean.SetXYZT(0, 0, 0, 0);
    fEdep           = 0;
    fCMONum         = -1;
    fNeutAssociated = false;
    fTerminated     = false;

    fStepRefList.Clear(opt);
    fParticleRefList.Clear(opt);
}

Bool_t TCMOHitInfo::AddStep(TCMOStepInfo *nowStep) {
    const Double_t startedTime = fFourPos_0.T();
    TParticleInfo *nowParticle = (&nowStep->GetParticle());

    if (fStepRefList.GetEntries() == 0) {
        fFourPos_0 = fFourPos_Mean = nowStep->GetFourPosition();
        fEdep                      = nowStep->GetEdep();
        fCMONum                    = nowStep->GetCMONum();
        fNeutAssociated            = nowStep->GetNeutronAssociated();

        TRef *newParticleRef = new TRef(nowParticle);
        fParticleRefList.AddLast(newParticleRef);

        TRef *newStepRef = new TRef(nowStep);
        fStepRefList.AddLast(newStepRef);
        return true;
    } else {
        if (nowStep->GetTime() <= startedTime + fcEvent_window && nowStep->GetCMONum() == fCMONum) {
            TParticleInfo *findResult;
            TParticleInfo::FindParticleInTCollectionOfTRef(fParticleRefList, nowParticle,
                                                           findResult);
            if (findResult == nullptr) {
                TRef *newParticleRef = new TRef(nowParticle);
                fParticleRefList.AddLast(newParticleRef);
            }
            fFourPos_Mean += nowStep->GetFourPosition();
            fEdep += nowStep->GetEnergyDeposit();
            fNeutAssociated |= nowStep->GetNeutronAssociated();

            TRef *newStepRef = new TRef(nowStep);
            fStepRefList.AddLast(newStepRef);
            return true;
        } else
            return false;
    }
}

Bool_t TCMOHitInfo::TerminateHit() {
    if (!IsTerminated()) {
        SetTerminate(true);

        const Int_t numOfSteps = fStepRefList.GetEntries();

        fFourPos_Mean *= 1. / numOfSteps;
    }
    return IsOverEnergyThreshold();
}

bool TCMOHitInfo::CompareByTime_0(const TCMOHitInfo &a, const TCMOHitInfo &b) {
    return a.GetTime_0() < b.GetTime_0();
}

bool TCMOHitInfo::CompareByCMONum(const TCMOHitInfo &a, const TCMOHitInfo &b) {
    return a.GetCMONum() < b.GetCMONum();
}
