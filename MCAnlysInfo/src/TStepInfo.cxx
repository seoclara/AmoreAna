#include "TStepInfo.hxx"

#include "MCObjs/TStep.hh"

ClassImp(TStepInfo);

TStepInfo::TStepInfo(TParticleInfo *particle, const TStep &inp)
    : TObject(), fAssociatedParticle(particle),
      fPosition(inp.GetX(), inp.GetY(), inp.GetZ(), inp.GetGlobalTime()),
      fEdep(inp.GetEnergyDeposit()), fKineticEnergy(inp.GetKineticEnergy()),
      fProcessName(inp.GetProcessName()), fVolumeName(inp.GetVolumeName()){};

TStepInfo::TStepInfo()
    : TObject(), fAssociatedParticle(), fPosition(), fEdep(0), fKineticEnergy(0), fProcessName(),
      fVolumeName(){};

void TStepInfo::Clear(Option_t *opt) {
    TObject::Clear(opt);
    fAssociatedParticle.Clear(opt);
    fPosition.Clear(opt);
    fPosition.SetXYZT(0, 0, 0, 0);
    fEdep          = 0;
    fKineticEnergy = 0;
    fProcessName.Clear();
    fVolumeName.Clear();
}

bool TStepInfo::CompareByTime(const TStepInfo &a, const TStepInfo &b) {
    return a.GetTime() < b.GetTime();
}

bool TStepInfo::CompareByTrkID(const TStepInfo &a, const TStepInfo &b) {
    return a.GetTrackID() < b.GetTrackID();
}

bool TStepInfo::CompareByTime(const TStepInfo *a, const TStepInfo *b) {
    return a->GetTime() < b->GetTime();
}

bool TStepInfo::CompareByTrkID(const TStepInfo *a, const TStepInfo *b) {
    return a->GetTrackID() < b->GetTrackID();
}
