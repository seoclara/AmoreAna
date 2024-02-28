#ifndef _NeutAnlys__H_
#define _NeutAnlys__H_ 1

#include <TChain.h>
#include <TClonesArray.h>
#include <TFile.h>
#include <TTree.h>
#include <TObjString.h>

#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <vector>

#if !defined(__CLING__) // for compiler (don't care about this section. If error occurs in this //
                        // section, erase this section)
#include <MCObjs/DetectorArray_Amore.hh>
#include <MCObjs/DetectorModule_Amore.hh>
#include <MCObjs/EvtStep.hh>
#include <MCObjs/EvtTrack.hh>
#include <MCObjs/MuonSD.hh>
#include <MCObjs/Photon.hh>
#include <MCObjs/Primary.hh>
#include <MCObjs/TCell.hh>
#include <MCObjs/THit.hh>
#include <MCObjs/TStep.hh>
#include <MCObjs/TTrack.hh>
#endif

#include "MCAnlysInfo/TCMOHitInfo.hxx"
#include "MCAnlysInfo/TCMOStepInfo.hxx"
#include "MCAnlysInfo/TParticleInfo.hxx"
#include "defs/I_definition.hxx"
#endif
