#ifndef _H_TTrackInfo
#define _H_TTrackInfo

#include "TParticleInfo.hxx"

#include <TLorentzVector.h>
#include <TObject.h>
#include <TROOT.h>
#include <TRef.h>
#include <TVector3.h>

class TTrack;
class TStep;

class TTrackInfo : public TObject {
  protected:
    TRef fParticle;

  private:
    TLorentzVector fPosition;
    Double_t fKineticEnergy;
    TString fVolumeName;

  public:
    TTrackInfo() : TObject(), fParticle(), fPosition(), fKineticEnergy(), fVolumeName(){};
    TTrackInfo(const TTrack &, TParticleInfo &);
    TTrackInfo(const TStep &, TParticleInfo &);
    virtual ~TTrackInfo() { Clear(); };

    virtual void Clear(Option_t *opt = "");

    inline Int_t GetTrackID() const {
        return static_cast<TParticleInfo *>(fParticle.GetObject())->GetTrackID();
    }
    inline Int_t GetParentID() const {
        return static_cast<TParticleInfo *>(fParticle.GetObject())->GetParentID();
    }
    inline Int_t GetPDGCode() const {
        return static_cast<TParticleInfo *>(fParticle.GetObject())->GetPDGCode();
    }

    inline Double_t GetX() const { return fPosition.X(); }
    inline Double_t GetY() const { return fPosition.Y(); }
    inline Double_t GetZ() const { return fPosition.Z(); }
    inline Double_t GetTime() const { return fPosition.T(); }
    inline const TLorentzVector &GetFourPosition() const { return fPosition; }

    inline Double_t GetKineticEnergy() const { return fKineticEnergy; }
    inline Double_t GetKE() const { return fKineticEnergy; }

    inline const TString &GetVolumeName() const { return fVolumeName; }

    inline const TParticleInfo &GetAssociatedParticle() const {
        return static_cast<TParticleInfo &>(*(fParticle.GetObject()));
    }

    inline void SetX(Double_t a) { fPosition.SetX(a); }
    inline void SetY(Double_t a) { fPosition.SetY(a); }
    inline void SetZ(Double_t a) { fPosition.SetZ(a); }
    inline void SetTime(Double_t a) { fPosition.SetT(a); }
    inline void SetFourPosition(const TLorentzVector &a) { fPosition = a; }
    inline void SetXYZT(Double_t x, Double_t y, Double_t z, Double_t t) {
        fPosition.SetXYZT(x, y, z, t);
    }

    inline void SetKineticEnergy(Double_t a) { fKineticEnergy = a; }
    inline void SetKE(Double_t a) { fKineticEnergy = a; }

    inline void SetVolumeName(const TString &a) { fVolumeName = a; }

    inline Bool_t IsSame3Position(const TVector3 &inp) const {
        return inp.X() == fPosition.X() && inp.Y() == fPosition.Y() && inp.Z() == fPosition.Z();
    }

    virtual bool operator==(const TTrackInfo &inp) const { return IsEqual(&inp); }

    bool operator==(const TVector3 &inp) const { return IsSame3Position(inp); }

    Bool_t IsEqual(const TObject *inp) const;

    ClassDef(TTrackInfo, 1)
};
#endif
