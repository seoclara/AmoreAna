#ifndef _H_TCMOTrackInfo
#define _H_TCMOTrackInfo

#include <TObject.h>
#include <TROOT.h>
#include <vector>

using namespace std;

class TCMOTrackInfo : public TObject {
  private:
    Int_t fTrackID;
    Int_t fIdxInTrackInfo;
    vector<int> fAncestorIdxesInTI;

  public:
    TCMOTrackInfo() : TObject(){};
    TCMOTrackInfo(Int_t aTID, Int_t aIdx)
        : fTrackID(aTID), fIdxInTrackInfo(aIdx), fAncestorIdxesInTI() {}
    virtual ~TCMOTrackInfo(){};

    inline void AddAncestorIdx(Int_t i) { fAncestorIdxesInTI.push_back(i); }
    inline void ClearAncestorIdx() { fAncestorIdxesInTI.clear(); }
    inline Int_t GetNumberOfAncestors() { return fAncestorIdxesInTI.size(); }
    inline Int_t GetIdxOfAncestors(Int_t i) { return fAncestorIdxesInTI.at(i); }
    ClassDef(TCMOTrackInfo, 1)
};
#endif
