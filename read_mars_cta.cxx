#include <map>
#include <iostream>
#include <fstream>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "mbase/MParContainer.h"
#include "mimage/MHillas.h"
#include "mimage/MHillasExt.h"
#include "mimage/MNewImagePar.h"
#include "mmc/MMcEvt.hxx"

#include "json.hpp"

using json = nlohmann::json;



int main(int argc, char** args) {

	if (argc < 3) {
		std::cout << "Usage:" << std::endl;
		std::cout << "read_mars_cta INPUTFILE OUTPUTFILE" << std::endl;
		return 1;
	}

	std::string inputfile = args[1];
	std::string outputfile = args[2];

	std::ofstream output(outputfile);

	const int n_telescopes = 567;

	// haenge die files aneinander
	TFile* f = new TFile(inputfile.c_str(), "READ"); 

	// pointer definieren
	MHillas** hillas = new MHillas*[n_telescopes];
	MHillasExt** hillas_ext = new MHillasExt*[n_telescopes];
	MNewImagePar** new_image_par = new MNewImagePar*[n_telescopes];
	MMcEvt** mc_evt = new MMcEvt*[n_telescopes];
	
	// aktiviere die Teile der Datei, die uns interessieren
	TTree* tree = (TTree*) f->Get("Events");

	tree->SetBranchStatus("*", 0);
	tree->SetBranchStatus("MHillas_*", 1);
	tree->SetBranchStatus("MHillasExt_*", 1);
	tree->SetBranchStatus("MNewImagePar_*", 1);
	tree->SetBranchStatus("MMcEvt_*", 1);
	tree->GetEntry(0);
	
	std::cout << "Setting branch addresses" << std::endl;
	for (int tel_id = 1;  tel_id < n_telescopes + 1; tel_id++) {
		hillas[tel_id - 1] = NULL;
		hillas_ext[tel_id - 1] = NULL;
		new_image_par[tel_id - 1] = NULL;
		mc_evt[tel_id - 1] = NULL;

		tree->SetBranchAddress(("MHillas_" + std::to_string(tel_id) + ".").c_str(), &hillas[tel_id - 1]);
		tree->SetBranchAddress(("MHillasExt_" + std::to_string(tel_id) + ".").c_str(), &hillas_ext[tel_id - 1]);
		tree->SetBranchAddress(("MNewImagePar_" + std::to_string(tel_id) + ".").c_str(), &new_image_par[tel_id - 1]);
		tree->SetBranchAddress(("MMcEvt_" + std::to_string(tel_id) + ".").c_str(), &mc_evt[tel_id - 1]);
	}

	std::cout << "Start Reading" << std::endl;

	json j;

	for (int i = 0; i < tree->GetEntries(); ++i) {
		tree->GetEntry(i);
		for (int tel_id = 1;  tel_id < n_telescopes + 1; tel_id++) {
			int idx = tel_id - 1;

			if (hillas[idx]->GetLength() == -1){
				continue;
			}

			j.clear();
			j["event_id"] = i;
			j["telescope_id"] = tel_id;

			j["array_event_id"] = mc_evt[idx]->GetEvtNumber();
			j["energy"] = mc_evt[idx]->GetEnergy();
			j["phi"] = mc_evt[idx]->GetPhi();
			j["theta"] = mc_evt[idx]->GetTheta();
			j["telescope_phi"] = mc_evt[idx]->GetTelescopePhi();
			j["telescope_theta"] = mc_evt[idx]->GetTelescopeTheta();

			j["size"] = hillas[idx]->GetSize();
			j["x"] = hillas[idx]->GetMeanX();
			j["y"] = hillas[idx]->GetMeanY();
			j["width"] = hillas[idx]->GetWidth();
			j["length"] = hillas[idx]->GetLength();
			j["delta"] = hillas[idx]->GetDelta();

			j["m3_long"] = hillas_ext[idx]->GetM3Long();
			j["m3_trans"] = hillas_ext[idx]->GetM3Trans();

			j["leakage1"] = new_image_par[idx]->GetLeakage1();
			j["leakage2"] = new_image_par[idx]->GetLeakage2();
			j["conc"] = new_image_par[idx]->GetConc();

			output << j.dump() << std::endl;
		}
	}	

	output.close();

	return 0;
}
