///// RockgammaAnlys_v2.cxx
// 3.053 MeV gamma count at three boundaries.

#include "BackgroundAnlys.hxx"

using namespace std;

Int_t g_entryID; // Variables for debug output
Int_t g_fileID;

// Static informations
const int debug_verbose = 0;
const Double_t hit_evt_window = 100 * ms;
const Double_t hit_evt_Eth    = 0;
const Int_t number_of_border  = 3;
const Int_t flush_modular     = 1000;
const char *tree_name         = "event_tree";

void RockgammaAnlys(const int runno, const char *input_path, const char *fileformat, const char *output_path, const int numpertask) {
#ifndef __CLING__
	cout << "COMPILED VERSION" << endl;
#endif


	/// INPUT FILE VARIABLES ////////////////////////////////////////////////////////////
	TChain *input_tree = new TChain(tree_name);
	for (int i = runno * numpertask; i < (runno + 1) * numpertask; i++) {
		cout << "Adding: " << i + 1 << endl;
		input_tree->Add(Form(Form("%s/%s", input_path, fileformat), i));
	}

	EvtInfo *read_info    = new EvtInfo;
	EvtTrack *read_track  = new EvtTrack;
	EvtStep  *read_step   = new EvtStep;
	vector<TTrack *> *read_endtrack = new std::vector<TTrack *>;

	TClonesArray *tcl_step  = read_step->GetStep();
	TClonesArray *tcl_track = read_track->GetTrack();

	input_tree->SetBranchAddress("EVENTINFO", &read_info);
	input_tree->SetBranchAddress("TRACK", &read_track);
	input_tree->SetBranchAddress("STEP", &read_step);
	input_tree->SetBranchAddress("EndTrack", &read_endtrack);



	/// OUTPUT FILE VARIABLES ////////////////////////////////////////////////////////
	int fileid, entryid;
	int event_id, event_type;
	double event_time, event_deltaT;
	double edep_total;
	int ngamma, nvolume;
	int shield_incoming, coppercan_incoming, targetroom_incoming, crystal_hit;
	int passing_leadgap;
	//char volumename[100][30];
	std::vector<TDecayInfo> decay_info;

	TFile *output_file = new TFile(Form("%s/rockgamma_result_run%d.root", output_path, runno), "RECREATE");
	TTree *output_tree = new TTree("reproduction", "reproduction tree", 5);
	///..... file information
	output_tree->Branch("fileid", &fileid, "fileid/I");
	output_tree->Branch("entryid", &entryid, "entryid/I");
	///..... event information
	output_tree->Branch("event_id", &event_id, "event_id/I");
	output_tree->Branch("event_time", &event_time, "event_time/D");
	output_tree->Branch("event_deltaT", &event_deltaT, "event_deltaT/D");
	output_tree->Branch("event_type", &event_type, "event_type/I");
	///..... decay information from step
	output_tree->Branch("edep_total", &edep_total, "edep_total/D");
	output_tree->Branch("decay_info", &decay_info);
	///..... event count for rock simulation
	output_tree->Branch("ngamma", &ngamma, "ngamma/I");
	output_tree->Branch("nvolume", &nvolume, "nvolume/I");
	output_tree->Branch("shield_incoming", &shield_incoming, "shield_incoming/I");
	output_tree->Branch("coppercan_incoming", &coppercan_incoming, "coppercan_incoming/I");
	output_tree->Branch("targetroom_incoming", &targetroom_incoming, "targetroom_incoming/I");
	output_tree->Branch("crystal_hit", &crystal_hit, "crystal_hit/I");
	output_tree->Branch("passing_leadgap", &passing_leadgap, "passing_leadgap/I");
	//output_tree->Branch("volumename", &volumename, " volumename[100][30]/C");
	/*
	for(int iv = 0; iv < 100; iv++){
		output_tree->Branch(Form("volumename%d",iv), &volumename[iv], Form("volumename%d[30]/C",iv));
	}
	*/



	/// EVENT LOOP ///////////////////////////////////////////////////////////////////
	int entries = input_tree->GetEntries();
	int curec, entrycnt = 0;
	bool shieldin, coppercanin, targetroomin, crystalin, gapin;
	bool pre_alphatag[max_ncrystals];
	for( int ha = 0; ha < max_ncrystals; ha++ ) {
		pre_alphatag[ha] = false;
	}

	int ntot_gamma = 0;
	int ntot_shieldin = 0;
	int ntot_coppercanin = 0;
	int ntot_targetroomin = 0;
	int ntot_crystalhit = 0;
	int ntot_gapin = 0;

	cout << "entries : " << entries << endl;
	for (int i = 0; i < entries; i++) {
	//for( int i = 0; i < 100; i++ ) { // event loop start
		if(debug_verbose) printf("\n%dth event \n",i); 
		input_tree->GetEntry(i);

		bool rockshellevent = false;
		bool toprockevent = false;
		decay_info.clear();
		ngamma = 0;
		shield_incoming = coppercan_incoming = targetroom_incoming = crystal_hit = passing_leadgap= 0;
		//for (int i =0; i<100; i++) {volumename[i][0] = '\0';}
		nvolume = 0;

		if (entrycnt <= i) { // Job 별로 처리할 파일들 나누는 루틴
			curec = ((TTree *)input_tree->GetFile()->Get(tree_name))->GetEntriesFast();
			entrycnt += curec;
			const char *now_filedesc = input_tree->GetFile()->GetName();
			TObjArray *tokened_nfd   = TString(now_filedesc).Tokenize("/");
			const char *now_filename = static_cast<TObjString *>(tokened_nfd->At(tokened_nfd->GetEntriesFast() - 1))->String().Data();
			sscanf(now_filename, fileformat, &fileid);
			g_fileID = fileid;
			cout << "NOWFILE: " << now_filename << " FILENO: " << fileid << " NOWINDEX: " << i << " CURFILEENTRIES: " << curec << endl;
		}
		entryid   = i - entrycnt + curec;
		g_entryID = entryid;



		///// TRACK CHECK ----------------------------------
		bool skip_thisevent = false;
		int ntrack = tcl_track->GetEntriesFast();
		vector<int> track_number;
		track_number.resize(ntrack + 2);

		for( int itr = 0; itr < ntrack; itr++ ) {

			auto single_endtrack = (*read_endtrack)[itr];
			TTrack *single_track = (TTrack*)(tcl_track->At(itr));
			 
			if( itr==0 && strstr(single_track->GetVolumeName(), "physRock") ) rockshellevent = true;
			if( itr==0 && strstr(single_track->GetVolumeName(), "physTopRock") ) toprockevent = true;

			if(single_track->GetTrackID() >= ntrack + 1) {
				printf(" ===============================\n");
				printf(" WARNING! TOO MANY TRACKS! : %d %d \n", ntrack, single_track->GetTrackID());
				printf(" This event will be skipped\n");
				skip_thisevent = true;
				break;
			} 
			else { track_number[single_track->GetTrackID()] = itr; }
		}
		if( skip_thisevent ) continue;


		///// EVENT INFORMATION --------------------------------
		event_id = read_info->GetEventID();
		event_time = read_info->GetUT();
		event_deltaT = read_info->GetDeltaUT();
		event_type = read_info->GetEventType();


		///// STEP READING with checking track number ---------
		edep_total = 0;
		double edep_alpha[max_ncrystals] = {0};
		double edep_beta[max_ncrystals] = {0};
		bool alphatag[max_ncrystals] = {0};

		int nstep = tcl_step->GetEntriesFast();
		if(debug_verbose) printf("There are %d tracks in this event.\n",ntrack); 
		for( int tid = 1; tid < ntrack+1; tid++ ) {
			if(debug_verbose) printf("present track id = %d\n", tid); 
			shieldin = coppercanin = targetroomin = crystalin = gapin = false;
			bool trackfound = false;
			int nstepintrk = 0;
			double firststep_z = 0;
			char pre_vname[30] = {'\0'};
			int nsubvol = 0;

			TTrack *single_track = (TTrack*)(tcl_track->At(track_number[tid]));
			const char *trk_pname = single_track->GetParticleName();
			const double trk_ke = single_track->GetKineticEnergy();
			
			if( !strstr(trk_pname, "gamma") || !(3.052<trk_ke && trk_ke<3.053) ) continue;
			if( debug_verbose) printf("track particle name: %s, kinetic energy: %f\n",trk_pname, trk_ke);

			for( int ist = 0; ist < nstep; ist++ ) {

				TStep *single_step = (TStep*)(tcl_step->At(ist));
				const int now_trkid = single_step->GetTrackID();
				const int pdgcode = single_step->GetPDGcode();
				const char *vname = single_step->GetVolumeName();
				const char *pname = single_step->GetParticleName();
				const double stepke = single_step->GetKineticEnergy();
				const double stepx = single_step->GetX();
				const double stepy = single_step->GetY();
				const double stepz = single_step->GetZ();

				if( now_trkid == tid ) {
					if( strstr(pname, "gamma") ){

						if(!trackfound) ngamma++;
						///........ gamma counting at border 1, 2 & 3 
						/// ( border1: shield housing, border2: innermost can, border3: target room)
						//if( debug_verbose ) printf("pre_vname: %s, now_vname: %s\n", pre_vname, vname);

						if (3.052 < stepke) {
							if( debug_verbose ) printf("pre_vname: %s, now_vname: %s\n", pre_vname, vname);
							/*
							bool repeated = false;
							for (int ivol = nsubvol; ivol < nvolume; ivol++) {
								if (strstr(vname, volumename[ivol])) repeated = true;
							}
							if (!repeated) {
								sprintf(volumename[nvolume],"%s",vname);
								nsubvol++;
								nvolume++;

								if ( strstr(vname,"shieldHousingPV") || strstr(vname, "PEShield_PV") ) shieldin = true;
								else if ( strstr(vname, "physCu1") ) coppercanin = true;
								else if ( strstr(vname, "physTargetRoom") ) targetroomin = true;
								else if ( strstr(vname, "CrystalCell") ) crystalin = true;

								/////...... gamma counting which are passing down on the gap between outer lead and inner lead
								if( single_step->GetStepNo()==1 ) {
									firststep_z = stepz;
								}
								else if( (345.75-5 < stepz && stepz < 575.75+50)  && 
										(-685-20 < stepx && stepx < 685+20) && (-685-20 < stepy && stepy < 685+20) && 
										(stepx*stepx + stepy*stepy > 508.5*508.5) && 
										firststep_z < stepz ) {
									if( debug_verbose) printf("gamma is in the gap between outer and inner lead\n");
									gapin = true;
								}
							}
							*/
							 if( (strstr(vname,"shieldHousingPV") || strstr(vname,"PEShield_PV")) && strstr(pre_vname,"Rock") && !shieldin) {
							 shieldin = true;
							 if( debug_verbose ) printf("gamma is going into shield housing\n");
							 }
							 else if( strstr(vname,"physCu1") && strstr(pre_vname,"physCu2") && !coppercanin ){
							 coppercanin = true;
							 if( debug_verbose ) printf("gamma is going into innermost copper can\n");
							 }
							 else if( strstr(vname,"physTargetRoom") && !strstr(pre_vname,"physCrystalArray") && !strstr(pre_vname,"physTargetRoom") && !targetroomin) {
							 targetroomin = true;
							 if( debug_verbose ) printf("gamma is going into target room\n");
							 }
							 else if( strstr(vname,"CrystalCell") && !crystalin ) {
							 crystalin = true;
							 if( debug_verbose ) printf("gamma is hit the crystals\n");
							 }

							 /////...... gamma counting which are passing down on the gap between outer lead and inner lead
							 if( single_step->GetStepNo()==1 ) {
								 firststep_z = stepz;
							 }
							 else if( (345.75-5 < stepz && stepz < 575.75+50)  && 
									 (-685-20 < stepx && stepx < 685+20) && (-685-20 < stepy && stepy < 685+20) && 
									 (stepx*stepx + stepy*stepy > 508.5*508.5) && 
									 firststep_z < stepz ) {
								 if( debug_verbose) printf("gamma is in the gap between outer and inner lead\n");
								 gapin = true;
							 }
						}

						/*
							 if( (strstr(vname,"shieldHousingPV") || strstr(vname,"PEShield_PV")) && strstr(pre_vname,"Rock") && !shieldin) {
							 shieldin = true;
							 if( debug_verbose ) printf("gamma is going into shield housing\n");
							 }
							 else if( strstr(vname,"physCu1") && strstr(pre_vname,"physCu2") && !coppercanin ){
							 coppercanin = true;
							 if( debug_verbose ) printf("gamma is going into innermost copper can\n");
							 }
							 else if( strstr(vname,"physTargetRoom") && !strstr(pre_vname,"physCrystalArray") && !strstr(pre_vname,"physTargetRoom") && !targetroomin) {
							 targetroomin = true;
							 if( debug_verbose ) printf("gamma is going into target room\n");
							 }
							 else if( strstr(vname,"CrystalCell") && !crystalin ) {
							 crystalin = true;
							 if( debug_verbose ) printf("gamma is hit the crystals\n");
							 }

						/////...... gamma counting which are passing down on the gap between outer lead and inner lead
						if( single_step->GetStepNo()==1 ) {
						firststep_z = stepz;
						}
						else if( (345.75-5 < stepz && stepz < 575.75+50)  && 
						(-685-20 < stepx && stepx < 685+20) && (-685-20 < stepy && stepy < 685+20) && 
						(stepx*stepx + stepy*stepy > 508.5*508.5) && 
						firststep_z < stepz ) {
						if( debug_verbose) printf("gamma is in the gap between outer and inner lead\n");
						gapin = true;
						}
						*/
					}
					trackfound = true;
					nstepintrk++;
					//printf("step looking: stepno(%d), trackid(%d), vname(%s)\n",single_step->GetStepNo(), now_trkid, vname);
					//printf("step counting: %d\n",nstepintrk);
				}

				if( trackfound && nstepintrk != single_step->GetStepNo() ) {
					//printf(" this step is moved on other track.\n");
					break;
				}

				sprintf(pre_vname, "%s", vname);
			} // step reading end


			if( shieldin ) shield_incoming++;
			if( coppercanin ) coppercan_incoming++;
			if( targetroomin ) targetroom_incoming++;
			if( crystalin ) crystal_hit++;
			if( gapin ) passing_leadgap++;

		} // track checking end

		///// calculation of deposited energy of hitted event
		for( int ist = 0; ist < nstep; ist++ ) {

			TStep *single_step = (TStep*)(tcl_step->At(ist));
			const int now_trkid = single_step->GetTrackID();
			const int pdgcode = single_step->GetPDGcode();
			const char *vname = single_step->GetVolumeName();
			const char *pname = single_step->GetParticleName();
			const double stepx = single_step->GetX();
			const double stepy = single_step->GetY();
			const double stepz = single_step->GetZ();

			/////......... Finding the crystal number and adding energy deposition if there is crystal hit.
			char buffer[19] = {'\0'};
			const size_t length_vname = strlen(vname);
			strncpy(buffer, vname+15, length_vname);
			if( strstr(vname, "CrystalCell") ) {
				const char *tower_pvname  = strtok(buffer,"_");
				const char *module_pvname = strtok(NULL, "_");
				int tower_id = atoi(tower_pvname);
				int module_id = atoi(module_pvname);
				int crystal_id = tower_id * 9 + module_id;

				double now_edep = single_step->GetEnergyDeposit();
				if(now_edep <= 0.) continue;

				edep_total += now_edep;
				if( strstr(pname,"alpha") || pdgcode>20040 ){
					if( single_step->GetGlobalTime() <= 500000. ){	edep_alpha[crystal_id] += now_edep;	}
				}
				else {
					if( single_step->GetGlobalTime() <= 500000. ){	edep_beta[crystal_id] += now_edep; 	}
				}
			}
		}


		///// EVENT TAGGING ----------- alpha/beta tagging
		for(int qq = 0; qq < max_ncrystals; qq++){
			if(pre_alphatag[qq] && (edep_alpha[qq] + edep_beta[qq]) >=3.024 && (edep_alpha[qq] + edep_beta[qq])<=3.044) alphatag[qq] = true;
			else alphatag[qq] = false;
		}
		for(int dd = 0; dd < max_ncrystals; dd++){
			if(edep_alpha[dd] >= 6.00 && edep_alpha[dd] <= 6.2) pre_alphatag[dd] = true;
			else pre_alphatag[dd] = false;
		}


		///// DECAY INFORMATION SAVING
		for(int idt = 0; idt < max_ncrystals; idt++){
			if(edep_alpha[idt]>0 || edep_beta[idt]>0){
				TDecayInfo decay;
				decay.setDetID(idt);
				decay.setEdepAlpha(edep_alpha[idt]);
				decay.setEdepBeta(edep_beta[idt]);
				decay.setAlphaTag(alphatag[idt]);
				decay_info.push_back(decay);
			}
		}

		ntot_gamma += ngamma;
		ntot_shieldin += shield_incoming;
		ntot_coppercanin += coppercan_incoming;
		ntot_targetroomin += targetroom_incoming;
		ntot_crystalhit += crystal_hit;
		ntot_gapin += passing_leadgap;
		//printf("%d gammas\n", ngamma);
		//printf("passing gamma: b1(%d), b2(%d), lmo(%d), gap(%d)\n", shield_incoming, targetroom_incoming, crystal_hit, passing_leadgap);

		cout << "nvolume: " << nvolume << endl;
		//cout << "volumename: " << endl;
		/*
		for(int i = 0; i<nvolume; i ++){
			cout << "      " << volumename[i] << endl;
		}
		*/
		output_tree->Fill();
	}// event loop end

	output_file->cd();
	output_tree->Write();
	output_file->Close();

	printf("%d total gammas\n", ntot_gamma);
	printf("passing total gamma: b1(%d), b2(%d), b3(%d), lmo(%d), gap(%d)\n", ntot_shieldin, ntot_coppercanin, ntot_targetroomin, ntot_crystalhit, ntot_gapin);
	cout << "END ANALYSIS" << endl;
	}

	int main(int, char *argv[]) {
		RockgammaAnlys(std::stoi(argv[1]), argv[2], argv[3], argv[4], stoi(argv[5]));
		return 0;
	}
