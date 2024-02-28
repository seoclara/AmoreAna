// TCMOHitMakerInfo: HitMaker의 정보를 저장하는 클래스
// 정보 목록: HitMaker의 Track정보(Reference), 총 Edep 정보, 이놈에 속하는 Particle들 정보
#ifndef _THitMakerInfo__H_
#define _THitMakerInfo__H_ 1

#include <cstring>
#include <vector>

#include <Rtypes.h>

#include <TCollection.h>
#include <TObject.h>

#include "TParticleInfo.hxx"
#include "TTrackInfo.hxx"

class TCMOStepInfo;

using namespace std;

class THitMakerInfo : public TObject {
  protected:
    TRef fAssociatedTrack;

  private:
    Double_t fTotalEdep;

  public:
    THitMakerInfo();
    THitMakerInfo(TTrackInfo &aHitMaker);
    virtual ~THitMakerInfo() { Clear(); };

    virtual void Clear(Option_t * = "");

    inline const TParticleInfo &GetAssociatedParticle() const {
        return static_cast<TTrackInfo *>(fAssociatedTrack.GetObject())->GetAssociatedParticle();
    }

    inline TTrackInfo *GetHitMaker() const {
        return static_cast<TTrackInfo *>(fAssociatedTrack.GetObject());
    }

    static Int_t FindHitMakerInTCollection(const TCollection &target_coll,
                                           const THitMakerInfo *target_THMI);

    static Int_t FindHitMakerInTCollection(const TCollection &target_coll,
                                           const TParticleInfo *target_TPI, THitMakerInfo *&result);

    inline void AddEdep(Double_t a) { fTotalEdep += a; }
    inline Double_t GetEdep() const { return fTotalEdep; }

    virtual Bool_t IsEqual(const TObject *inp) const;
    virtual Bool_t IsEqual(const THitMakerInfo *inp) const;
    virtual Bool_t IsEqual(const TParticleInfo *inp) const;

    ClassDef(THitMakerInfo, 1)
};

#endif
