#include "TCMOHitMakerInfo.hxx"

ClassImp(TCMOHitMakerInfo);

TCMOHitMakerInfo::TCMOHitMakerInfo()
    : THitMakerInfo(), fDaughterRefArray(), fDistToDaughter(), fHMEnteredCMO(false) {}

TCMOHitMakerInfo::TCMOHitMakerInfo(TTrackInfo &aHitMaker)
    : THitMakerInfo(aHitMaker), fDaughterRefArray(), fDistToDaughter(), fHMEnteredCMO(false) {}

void TCMOHitMakerInfo::Clear(Option_t *opt) {
    fDaughterRefArray.Clear(opt);
    fDistToDaughter.clear();
    fHMEnteredCMO = false;
    THitMakerInfo::Clear(opt);
}

Int_t TCMOHitMakerInfo::AppendDaughter(TParticleInfo *aDaug) {
    if (aDaug->GetTrackID() == this->GetAssociatedParticle().GetTrackID()) {
        fHMEnteredCMO = true;
        return -1;
    }

    Int_t i = 0;
    for (auto nowObj : fDaughterRefArray) {
        TRef *nowDaugRef       = static_cast<TRef *>(nowObj);
        TParticleInfo *nowDaug = static_cast<TParticleInfo *>(nowDaugRef->GetObject());
        if (nowDaug->GetTrackID() == aDaug->GetTrackID()) return i;
        i++;
    }
    Int_t j = 0;

    TRef *newDaugRef = new TRef(aDaug);
    fDaughterRefArray.AddLast(newDaugRef);

    const TObjArray &lAncestorRefArray = aDaug->GetAncestorRefArray();

    for (auto nowObj : lAncestorRefArray) {
        auto nowAncRef = static_cast<TRef *>(nowObj);
        auto nowAnc    = static_cast<TParticleInfo *>(nowAncRef->GetObject());
        if (nowAnc->GetTrackID() == this->GetAssociatedParticle().GetTrackID()) break;
        j++;
    }
    if (j == lAncestorRefArray.GetEntriesFast()) return -2;

    fDistToDaughter.push_back(j + 1);
    return i;
}

Int_t TCMOHitMakerInfo::FindHitMakerInTCollection(const TCollection &target_coll,
                                                  const TCMOHitMakerInfo *target_THMI) {
    Int_t retval = 0;
    for (auto nowObject : target_coll) {
        auto nowHM = static_cast<THitMakerInfo *>(nowObject);
        if (nowHM->IsEqual(target_THMI)) return retval;
        retval++;
    }
    return -1;
}

Int_t TCMOHitMakerInfo::FindHitMakerInTCollection(const TCollection &target_coll,
                                                  const TParticleInfo *target_TPI,
                                                  TCMOHitMakerInfo *&result) {
    Int_t retval = 0;
    result       = nullptr;
    for (auto nowObject : target_coll) {
        auto nowHM = static_cast<TCMOHitMakerInfo *>(nowObject);
        if (nowHM->IsEqual(target_TPI)) {
            result = nowHM;
            return retval;
        }
        retval++;
    }
    return -1;
}
