#ifndef _H_TCMOHitInfo_
#define _H_TCMOHitInfo_

#include <TLorentzVector.h>
#include <TObjArray.h>
#include <TObject.h>
#include <TROOT.h>

class TCMOStepInfo;
class TParticleInfo;

class TCMOHitInfo : public TObject {
  private:
    const Double_t fcEvent_window;
    const Double_t fcEnergy_threshold;
    static Double_t fgDefault_event_window;
    static Double_t fgDefault_energy_threshold;
    TLorentzVector fFourPos_0;
    TLorentzVector fFourPos_Mean;
    Double_t fEdep;
    Int_t fCMONum;
    Bool_t fNeutAssociated;

  protected:
    Bool_t fTerminated;
    TObjArray fParticleRefList;
    TObjArray fStepRefList;
    inline void SetTerminate(Bool_t a) { fTerminated = a; }

  public:
    TCMOHitInfo();

    TCMOHitInfo(Double_t window, Double_t e_th);

    virtual ~TCMOHitInfo() { Clear(); };

    virtual void Clear(Option_t * = "");

    static inline Double_t GetDefaultEventWindow() { return fgDefault_event_window; }
    static inline Double_t GetDefaultEnergyThreshold() { return fgDefault_energy_threshold; }

    static void SetDefaultEventWindow(Double_t a) { fgDefault_event_window = a; }
    static void SetDefaultEnergyThreshold(Double_t a) { fgDefault_energy_threshold = a; }

    inline Double_t GetEventWindow() const { return fcEvent_window; }
    inline Double_t GetEnergyThreshold() const { return fcEnergy_threshold; }
    inline Bool_t IsOverEnergyThreshold() const { return fEdep > fcEnergy_threshold; }

    inline Double_t GetX_0() const { return fFourPos_0.X(); }
    inline Double_t GetY_0() const { return fFourPos_0.Y(); }
    inline Double_t GetZ_0() const { return fFourPos_0.Z(); }
    inline Double_t GetTime_0() const { return fFourPos_0.T(); }
    inline const TLorentzVector &GetFourPosition_0() const { return fFourPos_0; }

    inline Double_t GetX_Mean() const { return fFourPos_Mean.X(); }
    inline Double_t GetY_Mean() const { return fFourPos_Mean.Y(); }
    inline Double_t GetZ_Mean() const { return fFourPos_Mean.Z(); }
    inline Double_t GetTime_Mean() const { return fFourPos_Mean.T(); }
    inline const TLorentzVector &GetFourPosition_Mean() const { return fFourPos_Mean; }

    inline void SetX_0(Double_t a) { fFourPos_0.SetX(a); }
    inline void SetY_0(Double_t a) { fFourPos_0.SetY(a); }
    inline void SetZ_0(Double_t a) { fFourPos_0.SetZ(a); }
    inline void SetTime_0(Double_t a) { fFourPos_0.SetT(a); }
    inline void SetXYZT_0(const TLorentzVector &a) { fFourPos_0 = a; }
    inline void SetXYZT_0(Double_t x, Double_t y, Double_t z, Double_t t) {
        fFourPos_0.SetXYZT(x, y, z, t);
    }

    inline void SetX_Mean(Double_t a) { fFourPos_Mean.SetX(a); }
    inline void SetY_Mean(Double_t a) { fFourPos_Mean.SetY(a); }
    inline void SetZ_Mean(Double_t a) { fFourPos_Mean.SetZ(a); }
    inline void SetTime_Mean(Double_t a) { fFourPos_Mean.SetT(a); }
    inline void SetXYZT_Mean(const TLorentzVector &a) { fFourPos_Mean = a; }
    inline void SetXYZT_Mean(Double_t x, Double_t y, Double_t z, Double_t t) {
        fFourPos_Mean.SetXYZT(x, y, z, t);
    }

    inline Double_t GetEdep() const { return fEdep; }
    inline void SetEdep(Double_t a) { fEdep = a; }

    inline Double_t GetEnergyDeposit() const { return fEdep; }
    inline void SetEnergyDeposit(Double_t a) { fEdep = a; }

    inline void SetCMONumber(Int_t a) { fCMONum = a; }
    inline Int_t GetCMONumber() const { return fCMONum; }

    inline void SetCMONum(Int_t a) { fCMONum = a; }
    inline Int_t GetCMONum() const { return fCMONum; }

    inline void SetNeutAssociated(Bool_t a) { fNeutAssociated = a; }
    inline Bool_t GetNeutAssociated() { return fNeutAssociated; }

    inline Bool_t IsTerminated() const { return fTerminated; }

    virtual Bool_t AddStep(TCMOStepInfo *nowStep);
    virtual Bool_t TerminateHit();

    const TObjArray &GetParticleRefList() const { return fParticleRefList; }
    const TObjArray &GetStepRefList() const { return fStepRefList; }

    // Comparing functions for std::sort (for ascending order)
    static bool CompareByTime_0(const TCMOHitInfo &a, const TCMOHitInfo &b);
    static bool CompareByCMONum(const TCMOHitInfo &a, const TCMOHitInfo &b);

    ClassDef(TCMOHitInfo, 1);
};
#endif
