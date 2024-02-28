// The following include statements is for my VIM's auto-completer engine.
// Don't care about this line. If error occured, omit following include lines.(Especially
// /home/basehw/*)
//
// Written by Base Hardware
#ifndef _H_EffiAnlysInfo
#define _H_EffiAnlysInfo

#include <Rtypes.h>

// R__LOAD_LIBRARY(/home/basehw/AMoRE_work/shlib/Linux3.10-GCC_4_8/libMCObjs.so)

#include <TBranch.h>
#include <TChain.h>
#include <TClonesArray.h>
#include <TFile.h>
#include <TMath.h>
#include <TROOT.h>

#include <iostream>

#if !defined(__CLING__) // for compiler (don't care about this section. If error occurs in this
                        // section, erase this section)
#include "MCObjs/EvtStep.hh"
#include "MCObjs/EvtTrack.hh"
#include "MCObjs/MuonSD.hh"
#include "MCObjs/Photon.hh"
#include "MCObjs/Primary.hh"
#include "MCObjs/TCell.hh"
#include "MCObjs/TGSD.hh"
#include "MCObjs/THit.hh"
#include "MCObjs/TStep.hh"
#include "MCObjs/TTrack.hh"
#endif

struct CMOTrackInfo {
    Int_t PDGcode;
    Int_t TrackID;
    Int_t ParentID;
    Float_t x, y, z;
    Double_t KineticEnergy;
    Double_t Globaltime;
    char ProcessName[50];
    char ParticleName[50];
    char VolumeName[50];
};

struct CMOStepInfo {
    Double_t fTime_0;
    Double_t fKE;
    Double_t fX;
    Double_t fY;
    Double_t fZ;
    Double_t fEdep;
    Int_t fCMONum;
    Int_t fStepNo;
    Int_t fTrackID;
    char ParticleName[50];
    char CreatorProc[50];
    Int_t fAnceStart;
    Int_t fAnceEnd;
    Bool_t fNeutAssociated;

    // Comparing functions for std::sort (for ascending order)
    static bool CompareByTime_0(const CMOStepInfo &a, const CMOStepInfo &b) {
        return a.fTime_0 < b.fTime_0;
    }
    static bool CompareByCMONum(const CMOStepInfo &a, const CMOStepInfo &b) {
        return a.fCMONum < b.fCMONum;
    }
    static bool CompareByTrkID(const CMOStepInfo &a, const CMOStepInfo &b) {
        return a.fCMONum < b.fTrackID;
    }
};

struct CMOHitInfo {
    Double_t fTime_0;
    Double_t fTime_mean;
    Double_t fEdep;
    Int_t fStepNum;
    Int_t fCMONum;
    Bool_t fNeutAssociated;

    // Comparing functions for std::sort (for ascending order)
    static bool CompareByTime_0(const CMOHitInfo &a, const CMOHitInfo &b) {
        return a.fTime_0 < b.fTime_0;
    }
    static bool CompareByCMONum(const CMOHitInfo &a, const CMOHitInfo &b) {
        return a.fCMONum < b.fCMONum;
    }
};

struct CMOHitInfo2 {
    Double_t fTime_0;
    Double_t fTime_mean;
    Double_t fEdep;
    Int_t fStepNum;
    Int_t fCMONum;
    Int_t fAncestorsAtAirBufferBorder[50] = {0};
    Bool_t fNeutAssociated;

    // Comparing functions for std::sort (for ascending order)
    static bool CompareByTime_0(const CMOHitInfo &a, const CMOHitInfo &b) {
        return a.fTime_0 < b.fTime_0;
    }
    static bool CompareByCMONum(const CMOHitInfo &a, const CMOHitInfo &b) {
        return a.fCMONum < b.fCMONum;
    }
};
#endif
