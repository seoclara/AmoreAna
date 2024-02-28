#include "TCMOStepInfo.hxx"

#include "MCObjs/TStep.hh"

ClassImp(TCMOStepInfo);

TCMOStepInfo::TCMOStepInfo(TParticleInfo *particle, const TStep &inp, Int_t aCMONum)
    : TStepInfo(particle, inp), fCMONum(aCMONum), fNeutAssociated(false) {
    if (abs(particle->GetPDGCode()) == 2112) fNeutAssociated = true;
}

TCMOStepInfo::TCMOStepInfo() : TStepInfo(), fCMONum(-1), fNeutAssociated(false) {}

bool TCMOStepInfo::CompareByCMONum(const TCMOStepInfo &a, const TCMOStepInfo &b) {
    return a.GetCMONum() < b.GetCMONum();
}

bool TCMOStepInfo::CompareByCMONum(const TCMOStepInfo *a, const TCMOStepInfo *b) {
    return a->GetCMONum() < b->GetCMONum();
}

void TCMOStepInfo::Clear(Option_t *opt) {
    TStepInfo::Clear(opt);
    fCMONum         = -1;
    fNeutAssociated = false;
}
