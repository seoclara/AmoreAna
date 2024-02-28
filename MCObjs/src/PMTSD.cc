#include "MCObjs/PMTSD.hh"

ClassImp(PMTSD);

//______________________________________________________________________________
//JW (2024.12.14) : NtotPmts added
PMTSD::PMTSD() : TObject(), Nhits(0), NhitPmts(0), NtotPmts(0) {}

//______________________________________________________________________________
PMTSD::PMTSD(const PMTSD &cell) : TObject(cell), Nhits(cell.Nhits), NhitPmts(cell.NhitPmts), NtotPmts(cell.NtotPmts) {}
// Copy a track object

//______________________________________________________________________________
PMTSD &PMTSD::operator=(const PMTSD &cell) {
    // Copy a track

    TObject::operator=(cell);
    Nhits            = cell.GetNhits();
    NhitPmts         = cell.GetNhitPmts();
    NtotPmts         = cell.GetNtotPmts(); // JW (2024.12.14)

    return *this;
}

//______________________________________________________________________________
void PMTSD::Clear(Option_t * /*option*/) { TObject::Clear(); }

