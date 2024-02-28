#ifndef _H_TCMOHitAndMakerInfo_
#define _H_TCMOHitAndMakerInfo_

#include "TCMOHitInfo.hxx"
#include "TCMOHitMakerInfo.hxx"
#include "TTrackInfo.hxx"

class TParticleInfo;
class TObjArray;

#include <vector>
using namespace std;

class TCMOHitAndMakerInfo : public TCMOHitInfo {
  public:
    typedef vector<TTrackInfo *> TTIList_type;
    typedef vector<TTrackInfo *>::iterator TTIListIter_type;

  protected:
    vector<Double_t> fEdepOfAParticle;
    vector<vector<Int_t>> fHitMakerIdx;
    vector<TString> fBorderNames;
    TObjArray fHitMakerArrays;

    void CalculateEdepArray();
    virtual Int_t GenerateHitMaker(TTIList_type *inciList);
    virtual Int_t FindHitMakerInList(TTIListIter_type, TTIListIter_type,
                                     TParticleInfo *&targetParticle);
    virtual void CheckAndAddHitmakerInfo(Int_t arrayIdx, TTrackInfo *&targetHM, Double_t edepOfDaug,
                                         TParticleInfo *aDaughter);

  private:
    static Int_t fgDefault_number_of_border;
    const Int_t fcNumberOfBorders;

  public:
    TCMOHitAndMakerInfo();

    TCMOHitAndMakerInfo(Double_t window, Double_t e_th, Int_t borderNum,
                        const char **names = nullptr);
    TCMOHitAndMakerInfo(Double_t window, Double_t e_th, Int_t borderNum,
                        const TString names[] = nullptr);

    TCMOHitAndMakerInfo(Int_t borderNum, const char **names = nullptr);
    TCMOHitAndMakerInfo(Int_t borderNum, const TString names[] = nullptr);

    virtual ~TCMOHitAndMakerInfo() { Clear(); };

    virtual void Clear(Option_t * = "");

    virtual Int_t TerminateHit(TTIList_type *inciList, Int_t &failIdx);

    static inline Int_t GetDefaultNumberOfBorder() { return fgDefault_number_of_border; }
    static void SetDefaultNumberOfBorder(Int_t a) { fgDefault_number_of_border = a; }

    inline const vector<Double_t> &GetEdepArray() const { return fEdepOfAParticle; }

    inline const vector<vector<Int_t>> &GetIdxArrays() const { return fHitMakerIdx; }
    inline const vector<Int_t> *GetIdxArray(Int_t i) const {
        return (i >= 0 && i < fcNumberOfBorders) ? &fHitMakerIdx[i] : nullptr;
    }

    inline const TObjArray *GetHitMakerArray(Int_t i) const {
        return static_cast<TObjArray *>(fHitMakerArrays.At(i));
    }
    inline const TObjArray &GetHitMakerArrays() const { return fHitMakerArrays; }

    inline void SetBorderName(Int_t i, const TString &name) {
        if (i >= fcNumberOfBorders) {
            return;
        } else {
            fBorderNames[i] = name;
        }
    }
    inline void SetBorderNames(const char **names) {
        for (int i = 0; i < fcNumberOfBorders; i++)
            fBorderNames[i] = names[i];
    }
    inline void SetBorderNames(const TString *names) {
        for (int i = 0; i < fcNumberOfBorders; i++)
            fBorderNames[i] = names[i];
    }

    inline Int_t GetNumberOfBorders() const { return fcNumberOfBorders; }

    inline TString GetBorderName(Int_t i) const {
        if (i >= fcNumberOfBorders)
            return "";
        else
            return fBorderNames[i];
    }

    ClassDef(TCMOHitAndMakerInfo, 1);
};

#endif
