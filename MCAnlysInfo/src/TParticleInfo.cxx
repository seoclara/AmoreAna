#include "TParticleInfo.hxx"

#include "MCObjs/TStep.hh"
#include "MCObjs/TTrack.hh"

#include <TRef.h>
#include <iostream>

using namespace std;

ClassImp(TParticleInfo);

TParticleInfo::TParticleInfo()
    : TObject(), fTrackID(-1), fParentID(-1), fPDGCode(-1), fStart4Pos(), fEnd4Pos(),
      fStartKineticEnergy(0.), fStartProcName(), fEndProcName(), fStartVolName(), fEndVolName(),
      fParticleName(), fAncestorRefArray() {}

TParticleInfo::TParticleInfo(const TTrack &startInfo, const TTrack &endInfo)
    : TObject(), fTrackID(startInfo.GetTrackID()), fParentID(startInfo.GetParentID()),
      fPDGCode(startInfo.GetPDGcode()),
      fStart4Pos(startInfo.GetX(), startInfo.GetY(), startInfo.GetZ(), startInfo.GetGlobalTime()),
      fEnd4Pos(endInfo.GetX(), endInfo.GetY(), endInfo.GetZ(), endInfo.GetGlobalTime()),
      fStartKineticEnergy(startInfo.GetKineticEnergy()), fStartProcName(startInfo.GetProcessName()),
      fEndProcName(endInfo.GetProcessName()), fStartVolName(startInfo.GetVolumeName()),
      fEndVolName(endInfo.GetVolumeName()), fParticleName(startInfo.GetParticleName()),
      fAncestorRefArray() {}

bool TParticleInfo::AddAncestor(TParticleInfo *ancestor) {
    if (this != ancestor) {
        if (fAncestorRefArray.GetEntries() == 0) {
            if (ancestor->fTrackID == fParentID) {
                TRef *newAncestorRef = new TRef(ancestor);
                fAncestorRefArray.AddLast(newAncestorRef);
                return true;
            } else {
                cout << "Wrong ancestor! It will not be added!" << endl;
                return false;
            }
        } else {
            TRef *readLastRef = static_cast<TRef *>(fAncestorRefArray[fAncestorRefArray.GetLast()]);
            TParticleInfo *lastAnc = static_cast<TParticleInfo *>(readLastRef->GetObject());
            if (ancestor->fTrackID == lastAnc->fParentID) {
                TRef *newAncestorRef = new TRef(ancestor);
                fAncestorRefArray.AddLast(newAncestorRef);
                return true;
            } else {
                cout << "Wrong ancestor! It will not be added!" << endl;
                return false;
            }
        }
    } else {
        cout << "A TParticleInfo can not be added to it's own ancestor list!" << endl;
        return false;
    }
}

void TParticleInfo::AddAncestors(TParticleInfo *parent) {
    if (this != parent && AddAncestor(parent)) {
        Int_t target_num = parent->fAncestorRefArray.GetEntries();
        for (Int_t i = 0; i < target_num; i++) {
            TRef *readRef = static_cast<TRef *>(parent->fAncestorRefArray[i]);
            if (!AddAncestor(static_cast<TParticleInfo *>(readRef->GetObject()))) {
                cout << "Wrong parent! Adding has been aborted!" << endl;
                return;
            }
        }
    } else {
        cout << "Wrong parent! It will not be added!" << endl;
        return;
    }
}

void TParticleInfo::SetStart(const TTrack &inp) {
    fStart4Pos.SetX(inp.GetX());
    fStart4Pos.SetY(inp.GetY());
    fStart4Pos.SetZ(inp.GetZ());
    fStart4Pos.SetT(inp.GetGlobalTime());
    fStartKineticEnergy = inp.GetKineticEnergy();
    fStartProcName      = inp.GetProcessName();
    fStartVolName       = inp.GetVolumeName();
}

void TParticleInfo::SetEnd(const TTrack &inp) {
    fEnd4Pos.SetX(inp.GetX());
    fEnd4Pos.SetY(inp.GetY());
    fEnd4Pos.SetZ(inp.GetZ());
    fEnd4Pos.SetT(inp.GetGlobalTime());
    fEndProcName = inp.GetProcessName();
    fEndVolName  = inp.GetVolumeName();
}

Bool_t TParticleInfo::IsEqual(const TObject *inp) const {
    const TParticleInfo *target = static_cast<const TParticleInfo *>(inp);
    return fPDGCode == target->fPDGCode && fTrackID == target->fTrackID &&
           fParentID == target->fParentID && fStart4Pos == target->fStart4Pos &&
           fEnd4Pos == target->fEnd4Pos;
}

Int_t TParticleInfo::FindAncestor(const TParticleInfo &target) const {
    Int_t i;
    const Int_t entries = fAncestorRefArray.GetEntries();
    for (i = 0; i < entries; i++) {
        TRef *readRef         = static_cast<TRef *>(fAncestorRefArray[i]);
        TParticleInfo *nowAnc = static_cast<TParticleInfo *>(readRef->GetObject());
        if (nowAnc->fTrackID == target.fTrackID) return i;
    }
    return -1;
}

Int_t TParticleInfo::FindParticleInTCollectionOfTRef(const TCollection &target_coll,
                                                     const TParticleInfo *target_TPI,
                                                     TParticleInfo *&result) {
    result         = nullptr;
    Int_t nowindex = 0;

    for (auto nowObject : target_coll) {
        TRef *nowParticleRef       = static_cast<TRef *>(nowObject);
        TParticleInfo *nowParticle = static_cast<TParticleInfo *>(nowParticleRef->GetObject());
        if (nowParticle->IsEqual(target_TPI)) {
            result = nowParticle;
            break;
        }
        nowindex++;
    }

    return nowindex;
}

void TParticleInfo::Clear(Option_t *opt) {
    TObject::Clear(opt);
    fTrackID  = -1;
    fParentID = -1;
    fPDGCode  = -1;
    fStart4Pos.Clear(opt);
    fStart4Pos.SetXYZT(0, 0, 0, 0);
    fEnd4Pos.Clear(opt);
    fEnd4Pos.SetXYZT(0, 0, 0, 0);
    fStartKineticEnergy = 0;
    fStartProcName.Clear();
    fEndProcName.Clear();
    fStartVolName.Clear();
    fEndVolName.Clear();
    fParticleName.Clear();
    fAncestorRefArray.Clear(opt);
}
