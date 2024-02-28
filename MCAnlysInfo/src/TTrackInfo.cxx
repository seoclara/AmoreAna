#include "TTrackInfo.hxx"
#include "MCObjs/TStep.hh"
#include "MCObjs/TTrack.hh"

#include <cstring>

using namespace std;

ClassImp(TTrackInfo);

TTrackInfo::TTrackInfo(const TTrack &aTrack, TParticleInfo &aParticle)
    : TObject(), fParticle(&aParticle),
      fPosition(aTrack.GetX(), aTrack.GetY(), aTrack.GetZ(), aTrack.GetGlobalTime()),
      fKineticEnergy(aTrack.GetKineticEnergy()), fVolumeName(aTrack.GetVolumeName()) {}

TTrackInfo::TTrackInfo(const TStep &aStep, TParticleInfo &aParticle)
    : TObject(), fParticle(&aParticle),
      fPosition(aStep.GetX(), aStep.GetY(), aStep.GetZ(), aStep.GetGlobalTime()),
      fKineticEnergy(aStep.GetKineticEnergy()), fVolumeName(aStep.GetVolumeName()) {}

Bool_t TTrackInfo::IsEqual(const TObject *inp) const {
    const TTrackInfo *aTarget = static_cast<const TTrackInfo *>(inp);
    return aTarget->fPosition == fPosition &&
           aTarget->GetAssociatedParticle() == GetAssociatedParticle();
}

void TTrackInfo::Clear(Option_t *opt) {
    fParticle.Clear(opt);
    fPosition.Clear(opt);
    fPosition.SetXYZT(0, 0, 0, 0);
    fKineticEnergy = 0;
    fVolumeName.Clear();
}
