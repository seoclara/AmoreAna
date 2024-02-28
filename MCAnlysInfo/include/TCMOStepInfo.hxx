#ifndef _H_TCMOStepInfo
#define _H_TCMOStepInfo

#include "TStepInfo.hxx"

class TStep;

class TCMOStepInfo : public TStepInfo {
  private:
    Int_t fCMONum;
    Bool_t fNeutAssociated;

  public:
    TCMOStepInfo();
    TCMOStepInfo(TParticleInfo *, const TStep &, Int_t);
    virtual ~TCMOStepInfo() { Clear(); };

    virtual void Clear(Option_t *opt = "");

    inline void SetCMONumber(Int_t a) { fCMONum = a; }
    inline void SetCMONum(Int_t a) { fCMONum = a; }

    inline void SetNeutronAssociated(Bool_t a) { fNeutAssociated = a; };

    inline Int_t GetCMONumber() const { return fCMONum; }
    inline Int_t GetCMONum() const { return fCMONum; }

    inline Bool_t GetNeutronAssociated() const { return fNeutAssociated; };

    inline Bool_t IsSameCMO(const TCMOStepInfo &a) const { return fCMONum == a.fCMONum; }

    // Comparing functions for std::sort (for ascending order)
    static bool CompareByCMONum(const TCMOStepInfo &a, const TCMOStepInfo &b);
    static bool CompareByCMONum(const TCMOStepInfo *a, const TCMOStepInfo *b);

    // operator
    virtual Bool_t operator==(const TCMOStepInfo &b) const {
        return fCMONum == b.fCMONum && TStepInfo::operator==(b);
    };

    // operator
    virtual Bool_t operator==(const TCMOStepInfo *b) const {
        return fCMONum == b->fCMONum && TStepInfo::operator==(b);
    };

    ClassDef(TCMOStepInfo, 1)
};
#endif
