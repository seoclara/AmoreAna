#include "BackgroundAnlys.hxx"

using namespace std;

void CalibrationAnlys(Int_t runno, const char *path, const char *filenameformat, const char *outpath, Int_t numpertask) 
{
  // Static informations =============================
  const Double_t hit_evt_window = 100 * ms;
  const char *tree_name         = "event_tree";

  // input files adding ===============================
  TChain *fInput = new TChain(tree_name);
  for(int i = runno * numpertask; i < (runno + 1) * numpertask; i++) 
	{
		cout << "Adding: " << i + 1 << endl;
    fInput->Add(Form(Form("%s/%s", path, filenameformat), i));
  }

  TGSD *readTGSD      = new TGSD;
  fInput->SetBranchAddress("TGSD", &readTGSD);
  TClonesArray *tclCell  = readTGSD->GetCell();

  // Set output tree =================================
  int fileID, entryID, nHit;
	std::vector<TEdepInfo> edepColl;

  TFile *fOutput = new TFile(Form("%s/result_run%d.root", outpath, runno), "RECREATE");
  TTree *fOTree  = new TTree("CalibrationAnlys", "Amore-II calibration analysis production", 5);
  fOTree->Branch("FileID", &fileID, "FileID/I");
  fOTree->Branch("EntryID", &entryID, "EntryID/I");
	fOTree->Branch("nHittedCell", &nHit, "nHit/I");
	fOTree->Branch("edepColl",&edepColl);

	// START EVENT LOOP ===================================
  Int_t curec, entrycnt = 0;

  Int_t entries = fInput->GetEntries();
	for(int i = 0; i < entries; i++) 
	{
    fInput->GetEntry(i);

    // Initialize output variables per events
		nHit = 0;
		edepColl.clear();

		// File and Event ID numbering------------
    if(entrycnt <= i) 
		{ 
			curec = ((TTree *)fInput->GetFile()->Get(tree_name))->GetEntriesFast();
			entrycnt += curec;
			const char *now_filedesc = fInput->GetFile()->GetName();
			TObjArray *tokened_nfd   = TString(now_filedesc).Tokenize("/");
			const char *now_filename = 
				static_cast<TObjString *>(tokened_nfd->At(tokened_nfd->GetEntriesFast() - 1))->String().Data();
			sscanf(now_filename, filenameformat, &fileID);
			cout << "NOWFILE: " << now_filename << " FILENO: " << fileID << " NOWINDEX: " << i << " CURFILEENTRIES: " << curec << endl;
		}
    entryID = i - entrycnt + curec;

		// Count number of crystal hit --------------------------
		for(int icell = 0; icell < max_ncrystals; icell++)
		{
			TCell *readSingleCell = (TCell*)(tclCell->At(icell));
			if(readSingleCell->GetEdep()>0) 
			{
				nHit++;

				TEdepInfo event;
				event.setEdep(readSingleCell->GetEdep());
				event.setID(icell);

				edepColl.push_back(event);
			}
		}

		if(nHit>0) fOTree->Fill();
	}
	fOTree->Write();
	fOutput->Close();
	cout << "END" << endl;
}

int main(int argc, char *argv[]) 
{
  CalibrationAnlys(atoi(argv[1]), argv[2], argv[3], argv[4], atoi(argv[5]));
}
