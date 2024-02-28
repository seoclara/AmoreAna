#include "TCMOHitAndMakerInfo.hxx"
#include "TCMOHitInfo.hxx"
#include "TCMOStepInfo.hxx"
#include "THitMakerInfo.hxx"
#include "TParticleInfo.hxx"
#include "TStepInfo.hxx"
#include "TTrackInfo.hxx"

#include "EffiAnlys.hxx" // Deprecated!
#include "TSimpleCMOHitInfo.hxx"
#include "TEdepInfo.hxx"
#include "TPrimaryInfo.hxx"
#include "TDecayInfo.hxx"
#include <map>
#include <vector>

#ifdef __MAKECINT__
// For Trajectory Classes
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class CMOStepInfo + ;                 // Deprecated!
#pragma link C++ class CMOTrackInfo + ;                // Deprecated!
#pragma link C++ class std::vector < CMOStepInfo > +;  // Deprecated!
#pragma link C++ class std::vector < CMOTrackInfo > +; // Deprecated!
#pragma link C++ class std::vector < CMOHitInfo > +;   // Deprecated!

#pragma link C++ class TStepInfo + ;
#pragma link C++ class TTrackInfo + ;
#pragma link C++ class TParticleInfo + ;

#pragma link C++ class TCMOStepInfo + ;
#pragma link C++ class TCMOHitInfo + ;

#pragma link C++ class THitMakerInfo + ;
#pragma link C++ class TCMOHitMakerInfo + ;
#pragma link C++ class TCMOHitAndMakerInfo + ;

#pragma link C++ class TSimpleCMOHitInfo + ;

#pragma link C++ class std::vector < TCMOHitInfo > +;
#pragma link C++ class std::vector < TCMOStepInfo > +;
#pragma link C++ class std::map < int, TParticleInfo * > +;
#pragma link C++ class std::vector < TCMOHitAndMakerInfo * > +;
#pragma link C++ class std::vector < TCMOStepInfo * > +;
#pragma link C++ class std::vector < TTrackInfo * > +;
#pragma link C++ class std::vector < TSimpleCMOHitInfo > +;
#pragma link C++ class std::vector < TCMOHitInfo * > +;

#pragma link C++ class TPrimaryInfo + ;
#pragma link C++ class TEdepInfo + ;
#pragma link C++ class TSelectedTrack +;
#pragma link C++ class TDecayInfo +;

#pragma link C++ class std::vector <TPrimaryInfo> +;
#pragma link C++ class std::vector <TEdepInfo> +;
#pragma link C++ class std::vector <TSelectedTrack> +;
#pragma link C++ class std::vector <TDecayInfo> +;
#endif
