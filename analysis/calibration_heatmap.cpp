#include <TCanvas.h>
#include <TFile.h>
#include <TH1I.h>
#include <TH2I.h>
#include <TSystem.h>
#include <TTree.h>
#include <TGraph.h>
#include <TCollection.h>
#include <TKey.h>

#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>

// ACTIVATIONS (y) PER EVENT (x) SCATTER

const size_t BATCH_NUM = 6 * 12;
//const Int_t ACTIVATION_THRESHOLD = 3125;

void Hitmap_det_eff(){
// 	int argc=1;
// 	char *argv[1];
//   TApplication App("app",&argc,argv);
    //-------------------------------------------------------------------
    //open File with TTree
    TFile file("/home/rkotitsa/Desktop/allpix-squared/workspace/output/CalibrationTreeWriter/test2ph_1.root");

    auto event_number = ((TTree*) file.Get("Event"))->GetEntries();

    std::vector<Double_t> values;
    std::vector<Double_t> referenceValues;
    std::vector<Int_t> x_coords;
    std::vector<Int_t> y_coords;

    auto dir = (TDirectory*) file.Get("Detector_6_4_7");

    if (!dir) {
        return;
    }

    auto tree = (TTree*) dir->Get("Hits");

    if (!tree) {
        return;
    }

    Int_t length;
    tree->SetBranchAddress("NHits", &length);

    values.resize(1 << 20);
    tree->SetBranchAddress("Value", &values[0]);

    referenceValues.resize(1 << 20);
    tree->SetBranchAddress("ReferenceValue", &referenceValues[0]);

    x_coords.resize(1 << 20);
    tree->SetBranchAddress("PixX", &x_coords[0]);

    y_coords.resize(1 << 20);
    tree->SetBranchAddress("PixY", &y_coords[0]);

    auto c1 = new TCanvas();
    c1->cd();
    auto c2 = new TCanvas();
    c2->cd();
    TH2F *graph = new TH2F("hcol1", "Title", 60, 8000, 14000, 53, 4000, 10000);
    TH2F *graphReference = new TH2F("hcol2", "Title2", 60, 8000, 14000, 53, 4000, 10000);
    char command;
	uint event;    

    while(true) {

    	std::cout << "plase insert the event number" << std::endl;
    	std::cin >> command;
        std::cout << command << std::endl;
        if (command == 'e') {
            std::cout << "exit" << std::endl;
            break;
        }
        std::cin >> event;
        std::cout << event << std::endl;

	    graph->Reset();
        graphReference->Reset();
        tree->GetEntry(event);
	    values.resize(length);
        referenceValues.resize(length);
	    x_coords.resize(length);
	    y_coords.resize(length);
	    // tree->GetEntry(event);

        if (length != 0) {
            // std::cout << "NHits: " << length << "\n[";

            // values.resize(length);

            for (int j = 0; j < length; ++j) {
                auto px = 130.30f;
                auto py = 128.98f;

                auto pitch_x = (px / 2.0f) + (py / 2.0f) * 0.5f;
                auto pitch_y = 2* (py / 2.0f) * sqrt(3.)/2.;

                //std::cout << "\n\n [[ (py / 2) = " << py / 2.0f << ", * sin(30) = " << std::sin(30.0f) << ", * pixY = " << y_coords[i] << " ]]\n\n";

                auto x = x_coords[j]*pitch_x; //* pitch_x;
                auto y = y_coords[j]*pitch_y + x_coords[j]*0.5*pitch_y;//* pitch_y;
                if(values[j]>0.01){
                    graph->Fill(x, y, values[j]);
                }
                graphReference->Fill(x, y, referenceValues[j]);
               // std::cout << '(' << x << ", " << y << ", " << values[j] << "), ";
            }
            // std::cout << "]\n\n";
        }
        c1->cd();
        graph->SetBit(TH1::kNoStats);
        graph->SetMaximum(60);
	    graph->Draw("zcol");
        c1->Update();
        c2->cd();
        graphReference->SetBit(TH1::kNoStats);
        graphReference->SetMaximum(60);
        graphReference->Draw("zcol");
        c2->Update();
    }

    tree->ResetBranchAddresses();

    return;
}

