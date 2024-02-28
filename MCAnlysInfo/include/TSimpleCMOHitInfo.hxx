#ifndef _TSimpleCMOHitInfo__H_
#define _TSimpleCMOHitInfo__H_ 1
#include <Rtypes.h>
#include <TObject.h>
#include <limits>

class TSimpleCMOHitInfo : public TObject {
  private:
    const Double_t _M_NaN = std::numeric_limits<double>::quiet_NaN();

  public:
    TSimpleCMOHitInfo()
        : TObject(), fTime_0(0), fTime_mean(0), fX(0), fY(0), fZ(0), fEdep(0), fCMONum(0) {}
    TSimpleCMOHitInfo(Int_t num)
        : TObject(), fTime_0(_M_NaN), fTime_mean(_M_NaN), fX(_M_NaN), fY(_M_NaN), fZ(_M_NaN),
          fEdep(_M_NaN), fCMONum(num) {}
    Double_t fTime_0;
    Double_t fTime_mean;
    Double_t fX;
    Double_t fY;
    Double_t fZ;
    Double_t fEdep;
    Int_t fCMONum;

    // Comparing functions for std::sort (for ascending order)
    static bool CompareByTime_0(const TSimpleCMOHitInfo &a, const TSimpleCMOHitInfo &b);
    static bool CompareByCMONum(const TSimpleCMOHitInfo &a, const TSimpleCMOHitInfo &b);

    TSimpleCMOHitInfo &operator=(const TSimpleCMOHitInfo &a);

    ClassDef(TSimpleCMOHitInfo, 1)
};
#endif
