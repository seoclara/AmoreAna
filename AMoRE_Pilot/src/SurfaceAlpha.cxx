// Surface alpha background simulation analysis script for AMoRE-Pilot
#include "SurfaceAlpha.hxx"

using namespace std;

int GetGroupNo(int PDGcode);

void SurfaceAlpha(Int_t runno, const char *path, const char *filenameformat, const char *outpath, Int_t numpertask, const char *sname) {

#ifndef __CLING__
    cout << "COMPILED VERSION" << endl;
#endif

    // Static informations
    const Double_t hit_evt_window = 100 * ms;
    const Double_t hit_evt_Eth    = 0;
    const Int_t flush_modular     = 1000;
    const char *tree_name         = "event_tree";
    TCMOHitInfo::SetDefaultEventWindow(hit_evt_window);
    TCMOHitInfo::SetDefaultEnergyThreshold(hit_evt_Eth);

    // INPUT FILE READING -------------------------------------------
    TChain *fInput = new TChain(tree_name);
    for( int i = runno * numpertask; i < (runno + 1) * numpertask; i++ ) {
        cout << "Adding: " << i + 1 << endl;
        fInput->Add(Form(Form("%s/%s", path, filenameformat), i));
    }

		EvtInfo  *readInfo  = new EvtInfo;
		Primary  *readPrim  = new Primary;
    EvtTrack *readTrack = new EvtTrack;
		EvtStep  *readStep  = new EvtStep;
		TGSD     *readTGSD  = new TGSD;
		TClonesArray *tclVertex = readPrim->GetVertex();
    TClonesArray *tclTrack  = readTrack->GetTrack();
		TClonesArray *tclStep   = readStep->GetStep();
		TClonesArray *tclCell   = readTGSD->GetCell();
    std::vector<TTrack *> *readEndTrack = new std::vector<TTrack *>;

		fInput->SetBranchAddress("EVENTINFO",&readInfo);
		fInput->SetBranchAddress("PRIMARY", &readPrim);
    fInput->SetBranchAddress("TRACK", &readTrack);
		fInput->SetBranchAddress("STEP", &readStep);
		fInput->SetBranchAddress("TGSD",&readTGSD);
    fInput->SetBranchAddress("EndTrack", &readEndTrack);
		Bool_t enableET = (fInput->GetBranch("EndTrack") != nullptr);

		// OUTPUT FILE SET-UP -----------------------------------------
    int fileid, entryid, nPrimVrtx;
		int evtid, evttype, groupno, decaytype;
		double UT, dUT; // time in nano second
		double TotEdep;

		int primPDGcode;
		double primX0, primY0, primZ0;
		TVector3 startPos;
		TVector3 endPos;
		
		//double X_start,Y_start,Z_start;
		//double X_end,Y_end,Z_end;

		double edep; // edepAlpha+edepBeta
		double edepAlpha, edepBeta, hitTimeAlpha, hitTimeBeta;
		double EdepCrystal[6] = {0};

		bool singleHitTag, multipleHitTag;
		//bool AlphaTag = false;
		//bool reAlphaTag = false;

		//int nGen = 0;
		double GTime_start = -1;
		double GTime_end   = -1;
		//double edepAlpha[6] = {0};
		//double edepBeta[6] = {0};
		//double edep[6] = {0};
		char Par[30];
		char Vol_start[30];
		char Vol_end[30];

    //TFile *fOutput = new TFile(Form("%s/result_run%d.root", outpath, runno), "RECREATE");
    TFile *fOutput = new TFile(Form("%s/TEST_result_run%d.root", outpath, runno), "RECREATE");
    TTree *fOTree  = new TTree("SurfaceAnal","Internal Production for surface alpha analysis");
		// ..... file information
    fOTree->Branch("file_ID", &fileid, "file_ID/I");
    fOTree->Branch("entry_ID", &entryid, "entry_ID/I");
		// ..... event information
		fOTree->Branch("evt_ID",&evtid,"evt_ID/I");
		fOTree->Branch("evt_Type",&evttype,"evt_Type/I");
		//fOTree->Branch("nGen",&nGen,"nGen/I");
		fOTree->Branch("UT",&UT,"UT/D");
		fOTree->Branch("dUT",&dUT,"dUT/D");
		// ..... primary vertex information
		fOTree->Branch("nPrim",&nPrimVrtx,"nPrim/I");
		fOTree->Branch("PDGcode",&primPDGcode, "PDGcode/I");
		fOTree->Branch("groupNo",&groupno, "groupNo/I");
		fOTree->Branch("decayType",&decaytype, "decayType/I");
		fOTree->Branch("primX0",&primX0,"primX0/D");
		fOTree->Branch("primY0",&primY0,"primY0/D");
		fOTree->Branch("primZ0",&primZ0,"primZ0/D");
		fOTree->Branch("Par",&Par,"Par[30]/C");
		fOTree->Branch("Vol_start",&Vol_start,"Vol_start[30]/C");
		fOTree->Branch("Vol_end",&Vol_end,"Vol_end[30]/C");
		fOTree->Branch("startPos",&startPos);
		fOTree->Branch("endPos",&endPos);
		/*
		fOTree->Branch("X_start",&X_start,"X_start/D");
		fOTree->Branch("Y_start",&Y_start,"Y_start/D");
		fOTree->Branch("Z_start",&Z_start,"Z_start/D");
		fOTree->Branch("X_end",&X_end,"X_end/D");
		fOTree->Branch("Y_end",&Y_end,"Y_end/D");
		fOTree->Branch("Z_end",&Z_end,"Z_end/D");
		*/
		// ..... step information
		fOTree->Branch("GTime_start",&GTime_start,"GTime_start/D");
		fOTree->Branch("GTime_end",&GTime_end,"GTime_end/D");
		fOTree->Branch("edep",&edep,"edep/D");
		fOTree->Branch("edepAlpha", &edepAlpha,"edepAlpha/D");
		fOTree->Branch("edepBeta",&edepBeta,"edepBeta/D");
		fOTree->Branch("hitTimeAlpha",&hitTimeAlpha,"hitTimeAlpha/D");
		fOTree->Branch("hitTimeBeta",&hitTimeBeta,"hitTimeBeta/D");
		// ..... TGSD information
		fOTree->Branch("TotEdep",&TotEdep,"TotEdep/D");
		fOTree->Branch("EdepCrystal",EdepCrystal,"edepCrystal[6]/D");
		fOTree->Branch("singleHitTag",&singleHitTag,"singleHitTag/O");
		fOTree->Branch("multipleHitTag",&multipleHitTag,"multipleHitTag/O");
		//fOTree->Branch("AlphaTag",&AlphaTag,"AlphaTag/O");
		//fOTree->Branch("reAlphaTag",&reAlphaTag,"reAlphaTag/O");
		//fOTree->Branch("edep",edep,"edep[6]/D");
		//fOTree->Branch("edepBeta",edepBeta,"edepBeta[6]/D");

    // VARIABLES FOR EVENTS READING
    Int_t curec, entrycnt = 0;
    int entries = fInput->GetEntries();

		// EVENT LOOP -------------------------------------------------
    for( int i = 0; i < entries; i++ ) {
    //for( int i = 0; i < 500; i++ ) {
			fInput->GetEntry(i);

      if (entrycnt <= i) { // Job 별로 처리할 파일들 나누는 루틴
        curec = ((TTree *)fInput->GetFile()->Get(tree_name))->GetEntriesFast();
        entrycnt += curec;
        const char *now_filedesc = fInput->GetFile()->GetName();
        TObjArray *tokened_nfd   = TString(now_filedesc).Tokenize("/");
        const char *now_filename =
            static_cast<TObjString *>(tokened_nfd->At(tokened_nfd->GetEntriesFast() - 1))
                ->String()
                .Data();
        sscanf(now_filename, filenameformat, &fileid);
        cout << "NOWFILE: " << now_filename << " FILENO: " << fileid << " NOWINDEX: " << i
              << " CURFILEENTRIES: " << curec << endl;
      }
      entryid = i - entrycnt + curec;

			// TGSD reading
			singleHitTag   = false;
			multipleHitTag = false;
			TotEdep = readTGSD->GetTotEdep();
			Int_t TotCellN = tclCell->GetEntriesFast();
			if( TotCellN != 6 ) cout << " WORNING !!! : The number of crystal is not matched 6. " << endl;
			else {
				for( int icell = 0; icell < TotCellN; icell++ ) {
					TCell *readSingleCell = static_cast<TCell *>(tclCell->At(icell));
					EdepCrystal[icell] = readSingleCell->GetEdep();
					if(TotEdep>0 && EdepCrystal[icell]>0) (TotEdep == EdepCrystal[icell]) ? singleHitTag = true : multipleHitTag = true;
				}
			}

			// EVENT INFORMATION ------------------------
			evtid   = readInfo->GetEventID();
			UT      = readInfo->GetUT();
			dUT     = readInfo->GetDeltaUT();
			evttype = readInfo->GetEventType();

			// PRIMARY READING --------------------------
			Par[0] = '\0'; 
			groupno = 0;
			decaytype = 0;
			primPDGcode = 0;
			primX0 = 0.;
			primY0 = 0.;
			primZ0 = 0.;
			int nprim = 0;

			nPrimVrtx = tclVertex->GetEntriesFast();
			for( int iv = 0; iv < nPrimVrtx; iv++ ){
				Vertex *readSingleVertex = (Vertex*)(tclVertex->At(iv));
				const char *pname = readSingleVertex->GetParticleName();
				const int pdgcode = readSingleVertex->GetPDGcode();

				if( GetGroupNo(pdgcode) != 0  && GetGroupNo(pdgcode) != 31){ 
					nprim++;
					primPDGcode = pdgcode;
					groupno = GetGroupNo(pdgcode);
					primX0 = readSingleVertex->GetX0();
					primY0 = readSingleVertex->GetY0();
					primZ0 = readSingleVertex->GetZ0();
					sprintf(Par,"%s",pname);
					//sprintf(Vol,"%s",readSingleTrack->GetVolumeName());
				}

				if( strstr(pname,"alpha")) decaytype = 1;
				else if( decaytype != 1  &&  (strstr(pname,"e-") || strstr(pname,"anti_nu_e")) ) decaytype = 2;
				else if( decaytype != 1 && decaytype != 2) decaytype = 0;

				if( nprim > 1 ) cout << " WORNING !!! : counted primary isotope is bigger than 1." << endl;
			}

			// STEP READING ----------------------------------
			edep      = 0;
			edepAlpha = 0;
			edepBeta  = 0;
			hitTimeAlpha= 0;
			hitTimeBeta = 0;

			int TotStepN = tclStep->GetEntriesFast();
			for( int ist = 0; ist < TotStepN; ist++ ){
				TStep *readSingleStep = (TStep*)(tclStep->At(ist));
				const int nowTID  = readSingleStep->GetTrackID();
			  const int pdgcode = readSingleStep->GetPDGcode();
				const char *vname = readSingleStep->GetVolumeName();
				const char *pname = readSingleStep->GetParticleName();
				
				if( strstr(vname,"CMO") && readSingleStep->GetEnergyDeposit()>0 ){
					if( strstr(pname,"alpha") || pdgcode>20040 ){
						if( readSingleStep->GetGlobalTime() <= 500000. ){
							edepAlpha += readSingleStep->GetEnergyDeposit();
							edep      += readSingleStep->GetEnergyDeposit();
							if( hitTimeAlpha == 0  ||  (hitTimeAlpha != 0  &&  hitTimeAlpha > readSingleStep->GetGlobalTime()) ){
								hitTimeAlpha = readSingleStep->GetGlobalTime();
							}
						}
					}
					else {
						if( readSingleStep->GetGlobalTime() <= 500000. ){
							edepBeta += readSingleStep->GetEnergyDeposit();
							edep     += readSingleStep->GetEnergyDeposit();
							if( hitTimeBeta == 0  ||  (hitTimeBeta != 0  &&  hitTimeBeta > readSingleStep->GetGlobalTime()) ){
								hitTimeBeta = readSingleStep->GetGlobalTime();
							}
						}
					}
				}
			}


			// TRACK CHECK
      Bool_t skipThisEvt = false;
      Int_t TotTrackN    = tclTrack->GetEntriesFast();
			Vol_start[0] = '\0';
			Vol_end[0] = '\0';
      for(int j = 0; j < TotTrackN; j++) {
				//Par[0] = '\0'; 
        auto readSingleEndTrack = (*readEndTrack)[j];
        TTrack *readSingleTrack = static_cast<TTrack *>(tclTrack->At(j));

        if(readSingleTrack->GetTrackID() >= TotTrackN + 1) {
					cout << "===================" << endl;
					cout << "WARNING! TOO MANY TRACKS! : " << TotTrackN << " "
                    << readSingleTrack->GetTrackID() << endl;
					cout << "This event will be skipped" << endl;
					cout << "FileID: " << fileid << " | EntryID: " << entryid << endl;
					cout << "===================" << endl;
					skipThisEvt = true;
					break;
				} else if ( readSingleTrack->GetPDGcode() + 1000000000 == primPDGcode ){

					GTime_start = readSingleTrack->GetGlobalTime();
					GTime_end   = readSingleEndTrack->GetGlobalTime();
					sprintf(Vol_start,"%s",readSingleTrack->GetVolumeName());
					sprintf(Vol_end,"%s",readSingleEndTrack->GetVolumeName());
					startPos.SetXYZ(readSingleTrack->GetX(),readSingleTrack->GetY(),readSingleTrack->GetZ());
					endPos.SetXYZ(readSingleEndTrack->GetX(), readSingleEndTrack->GetY(), readSingleEndTrack->GetZ());
					/*
					X_start = readSingleTrack->GetX();
					Y_start = readSingleTrack->GetY();
					Z_start = readSingleTrack->GetZ();
					X_end = readSingleEndTrack->GetX();
					Y_end = readSingleEndTrack->GetY();
					Z_end = readSingleEndTrack->GetZ();
					*/
					/*
					if( (strstr(sname,"U238") && strstr(readSingleTrack->GetParticleName(),"U238") && readSingleTrack->GetParentID() == 0) || 
							(strstr(sname,"Th232") && strstr(readSingleTrack->GetParticleName(),"Th232") && readSingleTrack->GetParentID() == 0) ) nGen++;
							*/
        }
      }
      if( skipThisEvt ) continue;
			if( nprim == 1 ) fOTree->Fill();
		}
		fOTree->Write();
    fOutput->Close();
    cout << "END" << endl;
}

int GetGroupNo(int PDGcode)
{
	int grpno = 0;
	char szBuf[128];
	sprintf(szBuf,"%i", PDGcode);

	if (((!strncmp(szBuf,"1000922380",9))) ||  //U238: 92238
			((!strncmp(szBuf,"1000902340",9))) ||  //Th234: 90234
			((!strncmp(szBuf,"1000912340",9))) ||  //Pa234: 91234
			((!strncmp(szBuf,"1000922340",9))))    //U234: 92234
	{		grpno = 11;  }
	else if (((!strncmp(szBuf,"1000902300",9)))) //Th230: 90230
	{		grpno = 12;  }
	else if ( ((!strncmp(szBuf,"1000882260",9))) ) //Ra226: 88226
	{		grpno = 13;	}
  else if (((!strncmp(szBuf,"1000862220",9))) ||  //Rn222: 86222
			     ((!strncmp(szBuf,"1000842180",9))) ||  //Po218: 84218
					 ((!strncmp(szBuf,"1000852180",9))) ||  //At218: 85218
					 ((!strncmp(szBuf,"1000822140",9))) ||  //Pb214: 82214
					 ((!strncmp(szBuf,"1000832140",9))) ||  //Bi214: 83214
					 ((!strncmp(szBuf,"1000842140",9))) ||  //Po214: 84214
					 ((!strncmp(szBuf,"1000822100",9))) ||  //Pb210: 82210
					 ((!strncmp(szBuf,"1000812100",9))) )   //Tl210: 81210
	{		grpno = 14;  }
	else if (((!strncmp(szBuf,"1000832100",9))) ||  //Bi210: 83210
					 ((!strncmp(szBuf,"1000842100",9))) ||  //Po210: 84210
					 ((!strncmp(szBuf,"1000802060",9))) ||  //Hg206: 80206
					 ((!strncmp(szBuf,"1000812060",9))) ||  //Tl206: 81206
					 ((!strncmp(szBuf,"1000822060",9))) )   //Pb206: 82206
	{		grpno = 15;	}
	else if ( ((!strncmp(szBuf,"1000902320",9))) || //Th232: 90232
            ((!strncmp(szBuf,"1000882280",9))) )  //Ra228: 88228
	{		grpno = 21;  }
	else if ( ((!strncmp(szBuf,"1000892280",9))) ||  //Ac228: 89228
            ((!strncmp(szBuf,"1000902280",9))) )   //Th228: 90228
	{		grpno = 22;	}
  else if ( ((!strncmp(szBuf,"1000882240",9))) ||  //Ra224: 88224
					  ((!strncmp(szBuf,"1000862200",9))) ||  //Rn220: 86220
					  ((!strncmp(szBuf,"1000842160",9))) ||  //Po216: 84216
					  ((!strncmp(szBuf,"1000822120",9))) ||  //Pb212: 82212
					  ((!strncmp(szBuf,"1000832120",9))) ||  //Bi212: 83212
					  ((!strncmp(szBuf,"1000842120",9))) ||  //Po212: 84212
					  ((!strncmp(szBuf,"1000812080",9))) ||  //Tl208: 81208
					  ((!strncmp(szBuf,"1000822080",9))) )   //Pb208: 82208
	{		grpno = 23;	}
  else if ( ((!strncmp(szBuf,"1000190400",9))) ||  //K40: 19040 
            ((!strncmp(szBuf,"1000180400",9))) ||  //Ar40: 18040
            ((!strncmp(szBuf,"1000200400",9))) )   //Ca40: 20040
	{		grpno = 31;	}
	else if ( ((!strncmp(szBuf,"1000922350",9))) ||  //U235: 92235
					  ((!strncmp(szBuf,"1000902310",9))) ||  //Th231: 90231
					  ((!strncmp(szBuf,"1000912310",9))) )   //Pa231: 91231
	{		grpno = 41;	}
	else if ( ((!strncmp(szBuf,"1000892270",9))) ||  //Ac227: 89227
					  ((!strncmp(szBuf,"1000902270",9))) ||  //Th227: 90227
					  ((!strncmp(szBuf,"1000872230",9))) ||  //Fr223: 87223
					  ((!strncmp(szBuf,"1000882230",9))) ||  //Ra223: 88223
					  ((!strncmp(szBuf,"1000852190",9))) ||  //At219: 85219
					  ((!strncmp(szBuf,"1000862190",9))) ||  //Rn219: 86219
					  ((!strncmp(szBuf,"1000832150",9))) ||  //Bi215: 83215
					  ((!strncmp(szBuf,"1000842150",9))) ||  //Po215: 84215
					  ((!strncmp(szBuf,"1000852150",9))) ||  //At215: 85215
					  ((!strncmp(szBuf,"1000822110",9))) ||  //Pb211: 82211
					  ((!strncmp(szBuf,"1000832110",9))) ||  //Bi211: 83211
					  ((!strncmp(szBuf,"1000842110",9))) ||  //Po211: 84211
					  ((!strncmp(szBuf,"1000812070",9))) ||  //Tl207: 81207
					  ((!strncmp(szBuf,"1000822070",9))) )   //Pb207: 82207
	{		grpno =42;	}
	else  { grpno = 0;	}

	return grpno;
}

int main(int argc, char *argv[]) {
    SurfaceAlpha(atoi(argv[1]), argv[2], argv[3], argv[4], atoi(argv[5]), argv[6]);
}
