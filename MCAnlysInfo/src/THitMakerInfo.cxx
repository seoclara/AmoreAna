#include "THitMakerInfo.hxx"
#include "TCMOStepInfo.hxx"

#include <iostream>

using namespace std;

ClassImp(THitMakerInfo);

THitMakerInfo::THitMakerInfo() : TObject(), fAssociatedTrack(), fTotalEdep(-1){};

THitMakerInfo::THitMakerInfo(TTrackInfo &aHitMaker)
    : TObject(), fAssociatedTrack(&aHitMaker), fTotalEdep(0){};

Int_t THitMakerInfo::FindHitMakerInTCollection(const TCollection &target_coll,
                                               const THitMakerInfo *target_THMI) {
    Int_t retval = 0;
    for (auto nowObject : target_coll) {
        auto nowHM = static_cast<THitMakerInfo *>(nowObject);
        if (nowHM->IsEqual(target_THMI)) return retval;
        retval++;
    }
    return -1;
}

Int_t THitMakerInfo::FindHitMakerInTCollection(const TCollection &target_coll,
                                               const TParticleInfo *target_TPI,
                                               THitMakerInfo *&result) {
    Int_t retval = 0;
    result       = nullptr;
    for (auto nowObject : target_coll) {
        auto nowHM = static_cast<THitMakerInfo *>(nowObject);
        if (nowHM->IsEqual(target_TPI)) {
            result = nowHM;
            return retval;
        }
        retval++;
    }
    return -1;
}

Bool_t THitMakerInfo::IsEqual(const TObject *inp) const {
    return IsEqual(static_cast<const THitMakerInfo *>(inp));
}

Bool_t THitMakerInfo::IsEqual(const THitMakerInfo *inp) const {
    return this->GetAssociatedParticle() == inp->GetAssociatedParticle();
}

Bool_t THitMakerInfo::IsEqual(const TParticleInfo *inp) const {
    return this->GetAssociatedParticle() == (*inp);
}

void THitMakerInfo::Clear(Option_t *opt) {
    fAssociatedTrack.Clear(opt);
    fTotalEdep = 0;
}
