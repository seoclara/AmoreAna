#ifndef _H_TStepInfo
#define _H_TStepInfo

#include "TParticleInfo.hxx"

#include <TLorentzVector.h>
#include <TObject.h>
#include <TRef.h>
#include <TString.h>

class TStep;

class TStepInfo : public TObject {
  protected:
    TRef fAssociatedParticle;

  private:
    TLorentzVector fPosition;
    Double_t fEdep;
    Double_t fKineticEnergy;
    TString fProcessName;
    TString fVolumeName;

  public:
    TStepInfo();
    TStepInfo(TParticleInfo *, const TStep &);
    virtual ~TStepInfo() { Clear(); };

    virtual void Clear(Option_t *opt = "");

    inline void SetX(Double_t a) { fPosition.SetX(a); }
    inline void SetY(Double_t a) { fPosition.SetY(a); }
    inline void SetZ(Double_t a) { fPosition.SetZ(a); }
    inline void SetTime(Double_t a) { fPosition.SetT(a); }
    inline void SetFourPosition(const TLorentzVector &a) { fPosition = a; }
    inline void SetXYZT(Double_t x, Double_t y, Double_t z, Double_t t) {
        fPosition.SetXYZT(x, y, z, t);
    }

    inline void SetEdep(Double_t a) { fEdep = a; }
    inline void SetEnergyDeposit(Double_t a) { fEdep = a; }

    inline void SetKineticEnergy(Double_t a) { fKineticEnergy = a; }
    inline void SetKE(Double_t a) { fKineticEnergy = a; }

    inline void SetProcessName(const TString &a) { fProcessName = a; }

    inline void SetVolumeName(const TString &a) { fVolumeName = a; }

    inline void SetParticle(TParticleInfo *a) { fAssociatedParticle = a; }

    inline Int_t GetTrackID() const {
        return static_cast<TParticleInfo *>(fAssociatedParticle.GetObject())->GetTrackID();
    }

    inline Double_t GetX() const { return fPosition.X(); }
    inline Double_t GetY() const { return fPosition.Y(); }
    inline Double_t GetZ() const { return fPosition.Z(); }
    inline Double_t GetTime() const { return fPosition.T(); }
    inline const TLorentzVector &GetFourPosition() const { return fPosition; }

    inline Double_t GetEdep() const { return fEdep; }
    inline Double_t GetEnergyDeposit() const { return fEdep; }

    inline Double_t GetKineticEnergy() const { return fKineticEnergy; }
    inline Double_t GetKE() const { return fKineticEnergy; }

    inline const TString &GetProcessName() const { return fProcessName; }

    inline const TString &GetVolumeName() const { return fVolumeName; }

    inline TParticleInfo &GetParticle() const {
        return static_cast<TParticleInfo &>(*fAssociatedParticle.GetObject());
    }
    inline const TString &GetParticleName() const {
        return static_cast<TParticleInfo *>(fAssociatedParticle.GetObject())->GetParticleName();
    }

    inline Bool_t IsSame3Position(const TStepInfo &a) const {
        return fPosition.X() == a.fPosition.X() && fPosition.Y() == a.fPosition.Y() &&
               fPosition.Z() == a.fPosition.Z();
    }
    inline Bool_t IsSame4Position(const TStepInfo &a) const { return fPosition == a.fPosition; }

    // Comparing functions for std::sort (for ascending order)
    static bool CompareByTime(const TStepInfo &a, const TStepInfo &b);
    static bool CompareByTrkID(const TStepInfo &a, const TStepInfo &b);

    static bool CompareByTime(const TStepInfo *a, const TStepInfo *b);
    static bool CompareByTrkID(const TStepInfo *a, const TStepInfo *b);

    // operator
    virtual Bool_t operator==(const TStepInfo &b) const {
        return fPosition == b.fPosition && fAssociatedParticle == b.fAssociatedParticle;
    };

    virtual Bool_t operator==(const TStepInfo *b) const {
        return fPosition == b->fPosition && fAssociatedParticle == b->fAssociatedParticle;
    };

    ClassDef(TStepInfo, 1)
};
#endif
