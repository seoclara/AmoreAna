#ifndef _H_InternalAnlys
#define _H_InternalAnlys

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <vector>

#include <TROOT.h>
#include <TSystem.h>
#include <TObjString.h>

#include <TRandom3.h>
#include <TBranch.h>
#include <TChain.h>
#include <TClonesArray.h>
#include <TFile.h>
#include <TLeaf.h>
#include <TMath.h>
#include <TString.h>
#include <TTree.h>

#if !defined(__CLING__) // for compiler (don't care about this section. If error occurs in this //
                        // section, erase this section)
#include "MCObjs/DetectorArray_Amore.hh"
#include "MCObjs/DetectorModule_Amore.hh"
#include "MCObjs/EvtStep.hh"
#include "MCObjs/EvtTrack.hh"
#include "MCObjs/MuonSD.hh"
#include "MCObjs/Photon.hh"
#include "MCObjs/Primary.hh"
#include "MCObjs/TCell.hh"
#include "MCObjs/THit.hh"
#include "MCObjs/TStep.hh"
#include "MCObjs/TTrack.hh"
#include "MCObjs/EvtInfo.hh"
#endif

#include "MCAnlysInfo/TCMOHitInfo.hxx"
#include "MCAnlysInfo/TCMOStepInfo.hxx"
#include "MCAnlysInfo/TParticleInfo.hxx"
#include "defs/I_definition.hxx"

#endif
/*
// Header file for the classes stored in the TTree if any.
#include <TStopwatch.h>

//EJ:
#include <iostream>
#include <TH2.h>
#include <TNtupleD.h>

// Header file for the classes stored in the TTree if any.
#include <TStopwatch.h>
#include <TClonesArray.h>
//#include <TObject.h>
//#include <TObjString.h>

#include "MCObjs/EvtInfo.hh"
#include "MCObjs/Primary.hh"
#include "MCObjs/Vertex.hh"
#include "MCObjs/EvtTrack.hh"
#include "MCObjs/TTrack.hh"
#include "MCObjs/EvtStep.hh"
#include "MCObjs/TStep.hh"
#include "MCObjs/Photon.hh"
#include "MCObjs/THit.hh"
#include "MCObjs/TGSD.hh"
#include "MCObjs/TCell.hh"
#include "MCObjs/PMTSD.hh"
#include "MCObjs/Scint.hh"

using namespace std;


double Eres(double E, double para0, double para1);
int GetGroupNo(int PDGcode);

// Fixed size dimensions of array or collections stored in the TTree if any.
class Ana {
protected:
   void AmorePilot();

public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   static const int max_n_photons=100000;
   static const int max_n_tgsd=1000;
   static const int max_n_primaries=16;

   Primary*	tclprimary;
   TClonesArray*	tclvrtx;
   EvtTrack*	tcltrack; 
   TClonesArray*	tcltr;
   EvtStep*	tclstep; 
   TClonesArray*	tclst;
   Photon*	tclphoton; 
   TClonesArray*	tclhit;
   TGSD*		tcltgsd; 
   TClonesArray*	tclcell;
   PMTSD*	pmtsd_inner; 
   PMTSD*	pmtsd_outer; 
   PMTSD*	pmtsd_chimney; 
   Scint*	scint; 
   EvtInfo * 	evtinfo;

   //EJ:
   //Ana(TTree *tree=0);
   Ana(TString srcname, TString fname, TTree *tree=0);
   virtual ~Ana();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(TString whichDetector);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
   virtual void     Booking();
   virtual void     Writing();
   virtual int      readInputParameters();
   virtual void     RemoveState(char *pname);

   //EJ: user definition
   TFile * RootOutputFile;
   TString _srcname;
   TString _fname;

   TTree *mctree;

   enum {max_ndetpart=3, max_npmt=100, max_ndet=50, max_nsource=4, max_nprimary=100};

   //Ntuple common variables
   float actIntl[max_ndet];
   float actPmt[max_npmt];
   float actMat;
   float massMat;
   float massCrystal[max_ndet];

   //Variables for input parameters
   const int ncrystal=max_ndet;
   int nDet=max_ndet;
   int nPmt=max_npmt;
   int isrc;
   int setup;
   double fadcEresPars[2];
   double sadcEresPars[2];
   float crystalMass[max_ndet];
   float crystalActivity[max_ndet][max_nsource];
   float pmtActivity[max_npmt][max_nsource];
   float materialActivity[max_nsource];
   char matName[30];
   float matMass;

   int detIdx;
   int pdgcode;
   int pmtid;
   int groupno;
   char  pname[30];
   char  vname[30];
   char  dtype[30];
   int nhit[max_npmt];
   int nhitall;


   ClassDef(Ana, 1)
};

#endif

#ifdef Ana_cxx
//Ana::Ana(TTree *tree) : fChain(0) 
Ana::Ana(TString srcname, TString fname, TTree *tree) : fChain(0)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   _srcname = srcname;
   _fname = fname;
   if (tree == 0) {    cout<<"something wrong in the input file??......"<<endl;
   }

   Init(tree);
}

Ana::~Ana()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t Ana::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t Ana::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void Ana::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;

   evtinfo	= new EvtInfo(); 
   tclprimary	= new Primary();
   tclvrtx	=  tclprimary->GetVertex();
   tcltrack	= new EvtTrack(); 
   tcltr	=  tcltrack->GetTrack();
   tclstep	= new EvtStep(); 
   tclst	=  tclstep->GetStep();
   tclphoton	= new Photon(); 
   tclhit	=  tclphoton->GetHit();
   tcltgsd	= new TGSD(); 
   tclcell	=  tcltgsd->GetCell();
   scint	= new Scint(); 


   fChain->SetBranchAddress("PRIMARY",	&tclprimary);
   fChain->SetBranchAddress("TRACK",	&tcltrack);
   fChain->SetBranchAddress("STEP",	&tclstep);
   fChain->SetBranchAddress("PHOTONHIT",&tclphoton);
   fChain->SetBranchAddress("SCINT",	&scint);
   fChain->SetBranchAddress("TGSD",	&tcltgsd);
   fChain->SetBranchAddress("EVENTINFO",&evtinfo);
   fChain->SetBranchAddress("PMTSD",	&pmtsd_inner);
   fChain->SetBranchAddress("PMTSD",	&pmtsd_outer);
   fChain->SetBranchAddress("PMTSD",	&pmtsd_chimney);

   Notify();
}

Bool_t Ana::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void Ana::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t Ana::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef Ana_cxx
*/
