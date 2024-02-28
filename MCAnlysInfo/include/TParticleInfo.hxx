// TParticleInfo: 입자의 정보를 저장하는 클래스
// 정보 목록: TrackID, PDG코드, 생성될 때의 정보, 소멸될 때의 정보, 조상들의 정보
#ifndef _TParticleInfo__H_
#define _TParticleInfo__H_ 1
#include <Rtypes.h>
#include <TLorentzVector.h>
#include <TObjArray.h>
#include <TObject.h>
#include <TRef.h>

class TTrack;

class TParticleInfo : public TObject {
  private:
    Int_t fTrackID;
    Int_t fParentID;
    Int_t fPDGCode;
    TLorentzVector fStart4Pos;
    TLorentzVector fEnd4Pos;
    Double_t fStartKineticEnergy;
    TString fStartProcName;
    TString fEndProcName;
    TString fStartVolName;
    TString fEndVolName;
    TString fParticleName;

  protected:
    TObjArray fAncestorRefArray;

  public:
    TParticleInfo();
    TParticleInfo(const TTrack &, const TTrack &);
    virtual ~TParticleInfo() { Clear(); };

    virtual void Clear(Option_t *opt = "");

    bool AddAncestor(TParticleInfo *);
    void AddAncestors(TParticleInfo *);
    inline TParticleInfo *GetParent() const {
        if (fAncestorRefArray.GetEntries()) {
            TRef *parentRef = static_cast<TRef *>(fAncestorRefArray.First());
            return static_cast<TParticleInfo *>(parentRef->GetObject());
        } else
            return nullptr;
    }
    inline const TObjArray &GetAncestorRefArray() const { return fAncestorRefArray; };
    inline void ClearAncestorRefArray() { fAncestorRefArray.Clear(); }

    inline void SetTrackID(Int_t a) { fTrackID = a; }
    inline void SetParentID(Int_t a) { fParentID = a; }
    inline void SetPDGCode(Int_t a) { fPDGCode = a; }

    inline void SetStartPos(const TLorentzVector &a) { fStart4Pos = a; }
    inline void SetStartX(Double_t a) { fStart4Pos.SetX(a); }
    inline void SetStartY(Double_t a) { fStart4Pos.SetY(a); }
    inline void SetStartZ(Double_t a) { fStart4Pos.SetZ(a); }
    inline void SetStartTime(Double_t a) { fStart4Pos.SetT(a); }

    inline void SetEndPos(const TLorentzVector &a) { fEnd4Pos = a; }
    inline void SetEndX(Double_t a) { fEnd4Pos.SetX(a); }
    inline void SetEndY(Double_t a) { fEnd4Pos.SetY(a); }
    inline void SetEndZ(Double_t a) { fEnd4Pos.SetZ(a); }
    inline void SetEndTime(Double_t a) { fEnd4Pos.SetT(a); }

    inline void SetKineticEnergy(Double_t a) { fStartKineticEnergy = a; }
    inline void SetStartProcessName(const TString &a) { fStartProcName = a; }
    inline void SetEndProcessName(const TString &a) { fEndProcName = a; }
    inline void SetStartVolumeName(const TString &a) { fStartVolName = a; }
    inline void SetEndVolumeName(const TString &a) { fEndVolName = a; }
    inline void SetParticleName(const TString &a) { fParticleName = a; }

    inline void SetStart(const TTrack &);
    inline void SetEnd(const TTrack &);

    inline Int_t GetTrackID() const { return fTrackID; }
    inline Int_t GetParentID() const { return fParentID; }
    inline Int_t GetPDGCode() const { return fPDGCode; }

    inline const TLorentzVector &GetStartPos() const { return fStart4Pos; }
    inline Double_t GetStartX() const { return fStart4Pos.X(); }
    inline Double_t GetStartY() const { return fStart4Pos.Y(); }
    inline Double_t GetStartZ() const { return fStart4Pos.Z(); }
    inline Double_t GetStartTime() const { return fStart4Pos.T(); }

    inline const TLorentzVector &GetEndPos() const { return fEnd4Pos; }
    inline Double_t GetEndX() const { return fEnd4Pos.X(); }
    inline Double_t GetEndY() const { return fEnd4Pos.Y(); }
    inline Double_t GetEndZ() const { return fEnd4Pos.Z(); }
    inline Double_t GetEndTime() const { return fEnd4Pos.T(); }

    inline Double_t GetKineticEnergy() const { return fStartKineticEnergy; }
    inline const TString &GetStartProcessName() const { return fStartProcName; }
    inline const TString &GetEndProcessName() const { return fEndProcName; }
    inline const TString &GetStartVolumeName() const { return fStartVolName; }
    inline const TString &GetEndVolumeName() const { return fEndVolName; }
    inline const TString &GetParticleName() const { return fParticleName; }

    virtual Int_t FindAncestor(const TParticleInfo &) const;

    virtual Bool_t IsEqual(const TObject *) const;

    static Int_t FindParticleInTCollectionOfTRef(const TCollection &target_coll,
                                                 const TParticleInfo *target_TPI,
                                                 TParticleInfo *&result);

    virtual bool operator==(const TParticleInfo &inp) const { return IsEqual(&inp); }
    bool operator==(const TVector3 &inp) const {
        return inp.X() == fStart4Pos.X() && inp.Y() == fStart4Pos.Y() && inp.Z() == fStart4Pos.Z();
    }

    ClassDef(TParticleInfo, 1)
};
#endif
