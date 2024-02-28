#include "TSimpleCMOHitInfo.hxx"

ClassImp(TSimpleCMOHitInfo)

    bool TSimpleCMOHitInfo::CompareByTime_0(const TSimpleCMOHitInfo &a,
                                            const TSimpleCMOHitInfo &b) {
    return a.fTime_0 < b.fTime_0;
}
bool TSimpleCMOHitInfo::CompareByCMONum(const TSimpleCMOHitInfo &a, const TSimpleCMOHitInfo &b) {
    return a.fCMONum < b.fCMONum;
}

TSimpleCMOHitInfo &TSimpleCMOHitInfo::operator=(const TSimpleCMOHitInfo &a) {
    fTime_0    = a.fTime_0;
    fTime_mean = a.fTime_mean;
    fX         = a.fX;
    fY         = a.fY;
    fZ         = a.fZ;
    fEdep      = a.fEdep;
    fCMONum    = a.fCMONum;
    return *this;
}
