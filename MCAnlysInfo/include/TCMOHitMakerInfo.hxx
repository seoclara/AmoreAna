#ifndef _TCMOHitMakerInfo__H_
#define _TCMOHitMakerInfo__H_ 1

#include <Rtypes.h>
#include <TObjArray.h>
#include <TRef.h>

#include "THitMakerInfo.hxx"

class TCMOHitMakerInfo : public THitMakerInfo {
  private:
    vector<Int_t> fDistToDaughter;
    Bool_t fHMEnteredCMO;
    TObjArray fDaughterRefArray;

  public:
    TCMOHitMakerInfo();
    TCMOHitMakerInfo(TTrackInfo &aHitMaker);
    virtual ~TCMOHitMakerInfo() { Clear(); };

    virtual void Clear(Option_t * = "");

    inline Int_t GetNumberOfDaughterInCMO() const { return fDaughterRefArray.GetEntries(); }

    inline Int_t IsCMOInternalDaug(Int_t tid) const {
        for (auto nowObj : fDaughterRefArray) {
            TRef *nowDaugRef       = static_cast<TRef *>(nowObj);
            TParticleInfo *nowDaug = static_cast<TParticleInfo *>(nowDaugRef->GetObject());
            if (tid == nowDaug->GetTrackID())
                return nowDaug->GetStartVolumeName().Contains("physCMOCell");
        }
        return -1;
    }

    inline Int_t IsCMOInternalDaug(const TParticleInfo *aParticle) const {
        return IsCMOInternalDaug(aParticle->GetTrackID());
    }

    const TObjArray &GetDaughterRefArray() const { return fDaughterRefArray; }

    inline Bool_t IsHitMakerReachedCMO() const { return fHMEnteredCMO; }

    Int_t AppendDaughter(TParticleInfo *);

    static Int_t FindHitMakerInTCollection(const TCollection &target_coll,
                                           const TCMOHitMakerInfo *target_THMI);

    static Int_t FindHitMakerInTCollection(const TCollection &target_coll,
                                           const TParticleInfo *target_TPI,
                                           TCMOHitMakerInfo *&result);

    ClassDef(TCMOHitMakerInfo, 1);
};

#endif
