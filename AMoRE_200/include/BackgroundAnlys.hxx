#ifndef _H_BackgroundAnlys
#define _H_BackgroundAnlys

#include <Rtypes.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <vector>

#include <TROOT.h>
#include <TSystem.h>
#include <TObjString.h>

#include <TRef.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TClonesArray.h>
#include <TRandom3.h>
#include <TBranch.h>
#include <TChain.h>
#include <TFile.h>
#include <TLeaf.h>
#include <TMath.h>
#include <TString.h>
#include <TTree.h>
#include <TVector3.h>

#if !defined(__CLING__) // for compiler (don't care about this section. If error occurs in this //
                        // section, erase this section)
#include "MCObjs/EvtInfo.hh"
#include "MCObjs/EvtStep.hh"
#include "MCObjs/EvtTrack.hh"
#include "MCObjs/TGSD.hh"
#include "MCObjs/TStep.hh"
#include "MCObjs/TTrack.hh"
#include "MCObjs/Primary.hh"
#include "MCObjs/TCell.hh"
#include "MCObjs/MuonSD.hh"
#include "MCObjs/Photon.hh"
#include "MCObjs/THit.hh"
#include "MCObjs/Vertex.hh"
#endif

#include "MCAnlysInfo/TCMOHitInfo.hxx"
#include "MCAnlysInfo/TCMOStepInfo.hxx"
#include "MCAnlysInfo/TTrackInfo.hxx"
#include "MCAnlysInfo/TParticleInfo.hxx"
#include "MCAnlysInfo/TEdepInfo.hxx"
#include "MCAnlysInfo/TPrimaryInfo.hxx"
#include "MCAnlysInfo/TDecayInfo.hxx"
#include "defs/200_definition.hxx"

#endif
