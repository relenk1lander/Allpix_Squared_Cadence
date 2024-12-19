#include <TCanvas.h>
#include <TFile.h>
#include <TH1I.h>
#include <TSystem.h>
#include <TTree.h>
#include <TCollection.h>
#include <TKey.h>

#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>

// ACTIVATIONS (x) PER EVENT (y) HISTOGRAM

// Settings -- activated pixels per event, per plane - separate graphs:
// ACTIVATION_THRESHOLD - all 3125;
// SKIP_ZEROS - false;
// EVENTS_ON_X - false;
// COUNT_ACTIVATIONS - true;
// MERGE_GRAPHS - false;
// SINGLE_CHIP - nullptr;
// if we want to focus on single chip:
// SINGLE_CHIP - "Plane405"
// if we want have a single graph:
// MERGE_GRAPHS - true
// if we want variable thresholds on single graph:
// ACTIVATION_THRESHOLD - what is the prefered threshold per plane index
// MERGE_GRAPHS - true

auto px = 130.30f;
auto py = 128.98f;

auto pitch_x = (px / 2.0f) + (py / 2.0f) * 0.5f;
auto pitch_y = 2 * (py / 2.0f) * sqrt(3.) / 2.;

std::pair<float, float> translate_coords(float x, float y)
{
    return {x * pitch_x, (y + x * 0.5) * pitch_y};
}

const Int_t NUM_PLANES = 6;
const size_t BATCH_NUM = 6 * 12; // number of chips per plane
const Int_t ACTIVATION_THRESHOLD[6] = {3125, 3125, 3125, 3125, 3125, 3125};
// if the x/y axis should be flipped putting the activations on Y, 0.5-> 3125
const bool SKIP_ZEROS = false;
const bool EVENTS_ON_X = false;
const bool COUNT_ACTIVATIONS = false;
const bool MERGE_GRAPHS = false;
const char *SINGLE_CHIP = "Detector_6_4_7";
const Int_t target_x = 109;
const Int_t target_y = 8;

int iii = 0;

std::pair<std::vector<TH1I>, std::vector<TH1I>> Final_analysis25()
{

    //-------------------------------------------------------------------
    //open File with TTree
    TFile file("/home/client/allpix-squared/workspace/output/CalibrationTreeWriter/test.root");

    // Create an iterator on the contents of the file
    TIter iter(file.GetListOfKeys());

    // We expect an "Event" tree to exist, we use it to aquire the number of events easily, otherwise is not needed
    auto event_number = ((TTree *)file.Get("Event"))->GetEntries();
    //cout << event_number << " events found" << endl;
    
    std::vector<Double_t> ref_values;
    std::vector<Double_t> values;
    std::vector<Int_t> x_coords;
    std::vector<Int_t> y_coords;

    std::unordered_map<Int_t, std::unordered_map<Int_t, Double_t>> pix;

    // Create a 2D array, NUM_PLANES arrays of event_number size.
    // Each nested array stores the total activations, of a detector on a plane.
    std::vector<std::vector<Int_t>> activations(NUM_PLANES, std::vector<Int_t>(event_number));
    std::vector<std::vector<Int_t>> reference_activations(NUM_PLANES, std::vector<Int_t>(event_number));

    auto idx = 0;
    int runner = 0;
    // We iterate through the subdirectories of the file, tracking the current detector index.
    for (TKey *key = nullptr; key = (TKey *)iter();)
    {
        auto name = key->GetName();
        // We skip the "Event" tree if it's out of position
        if (!strcmp(name, "Event") || !strcmp(name, "Detector"))
        {
            continue;
        }
        if (SINGLE_CHIP != nullptr && strcmp(name, SINGLE_CHIP))
        {
            continue;
        }
        // std::cout << key->GetName() << '\n';

        // We assume all other keys, apart from "Event", are directories ...
        auto dir = (TDirectory *)key->ReadObj();
        // ... and contain a "Hits" tree.
        auto tree = (TTree *)dir->Get("Hits");

        // std::cout << "Tree has " << tree->GetEntries() << " entries\n";

        Int_t length; // length of the value array
        tree->SetBranchAddress("NHits", &length);

        values.resize(1 << 22); // the value array, set to max possible size of event_number
        tree->SetBranchAddress("Value", &values[0]);

        ref_values.resize(1 << 22);
        tree->SetBranchAddress("ReferenceValue", &ref_values[0]);

        x_coords.resize(1 << 20);
        tree->SetBranchAddress("PixX", &x_coords[0]);

        y_coords.resize(1 << 20);
        tree->SetBranchAddress("PixY", &y_coords[0]);

        // We calculate the plane index by batching entries on groups of BATCH_NUM
        // This is valid as the file directories were created by RCWriter by sorting the detector names
        // which follow the format of Layer_{plane number}_{x pos}_{y_pos}
        size_t current_plane = idx / BATCH_NUM;

        // Iterate through the events (entries) on the current tree
        for (int i = 0; i < tree->GetEntries(); ++i)
        {
            tree->GetEntry(i);
            values.resize(length);
            ref_values.resize(length);
            tree->GetEntry(i);
            // cout << endl
            //      << "Event " << i << endl;
            // std::cout << "First check NHits: " << length << "\n[";
            if (length != 0)
            { // Ignore entries with zero-length value arrays

                // std::cout << "NHits: " << length << "\n[";

                // We now the exact length now, so we can resize the vector, bringing it in a valid state
                //values.resize(length);

                for (size_t value_index = 0; value_index < values.size(); ++value_index)
                {
                    // Current detector value: activations[current_plane][i]
                    // plane: current_plane
                    // i: index in this plane

                    auto x = x_coords[value_index];
                    auto y = y_coords[value_index];

                    if (target_x > 0 && target_y > 0 && (x != target_x || y != target_y)) {
                        continue;
                    }

                    auto value = values[value_index];
                    auto reference_value = ref_values[value_index];

                    // activation count
                    if (COUNT_ACTIVATIONS)
                    {
                        activations[current_plane][i] += (value >= ACTIVATION_THRESHOLD[current_plane]);
                    }
                    // total charge
                    else
                    {
                        // std::cout << "sum [" << iii++ << "] :: " << activations[current_plane][i] << " + " << value << '\n';
                        activations[current_plane][i] += value;
                        reference_activations[current_plane][i] += reference_value;

                        pix[x][y] += value;
                    }

                    // if (value >= ACTIVATION_THRESHOLD[current_plane])
                    // {
                    //     auto [x_pos, y_pos] = translate_coords(x, y);

                    //     std::cout << current_plane << ", " << i << ", " << (name + 5) << ", " << x_pos << ", " << y_pos << '\n';
                    // }

                    // std::cout << x << ", ";
                }

                // std::cout << "]\n\n";
                // cout << "Activations: " << activations[current_plane][i] << endl;
            }
        }

        tree->ResetBranchAddresses();
        idx++;
    }

    std::map<Double_t, std::pair<Int_t, Int_t>> ord_pix;

    for (auto& [x, bucket] : pix) {
        for (auto& [y, value] : bucket) {
            // std::cout << x << ' ' << y << ' ' << value << '\n';
            ord_pix[value] = { x, y };
        }
    }

    for (auto& [value, pos] : ord_pix) {
        std::cout << pos.first << ' ' << pos.second << ' ' << value << '\n';
    }

    // If activations are plotted on the X-axis of a histogram,
    // we have to calculate in advance the max X value
    // hence we do the following to aquire the global (from all planes) maximum activation number
    Int_t max_activations = 0;

    for (auto plane : activations)
    {
        auto plane_max = *std::max_element(plane.begin(), plane.end());
        max_activations = std::max(max_activations, plane_max);
    }

    // std::cout << max_activations << std::endl;

    // gStyle->SetOptTitle(kFALSE);
    // gStyle->SetOptStat(0);
    // gStyle->SetErrorX(0.);

    std::vector<TH1I> histograms;
    std::vector<TH1I> reference_histograms;

    for (auto i = 0; i < activations.size(); ++i)
    {
        auto name = "Layer " + std::to_string(i + 1);
        auto reference_name = "Reference Layer " + std::to_string(i + 1);
        auto max_x = EVENTS_ON_X ? event_number : max_activations;
        std::cout << max_activations << '\n';
        histograms.emplace_back(name.c_str(), name.c_str(), (max_activations + 1), 0, max_x);
        histograms[i].SetMarkerStyle(kFullCircle);
        reference_histograms.emplace_back(reference_name.c_str(), reference_name.c_str(), (max_activations + 1), 0, max_x);
        reference_histograms[i].SetMarkerStyle(kFullCircle);
        reference_histograms[i].SetFillColor(kRed);

        // std::cout << "\n\n[";
        for (auto j = 0; j < activations[i].size(); ++j)
        {
            // std::cout << activations[i][j] << ", ";
            if (activations[i][j] || !SKIP_ZEROS)
            {
                // activatons (on x-axis) against events (on y-axis) WHAT WAS ASKED
                if (!EVENTS_ON_X)
                {
                    // std::cout << "val " << activations[i][j] << '\n';
                    histograms[i].Fill(activations[i][j]);
                    // std::cout << "ref " << reference_activations[i][j] << '\n';
                    reference_histograms[i].Fill(reference_activations[i][j]);
                }
                // std::cout << "{ " << activations[i][j] << ", " << j << " }\n";
                // activatons (on y-axis) against events (on x-axis) WHAT WAS ASKED
                else
                {
                    histograms[i].Fill(j, activations[i][j]);
                }
            }
        }
        // std::cout << "]\n";
    }

    if (SINGLE_CHIP)
    {
        histograms[0].Draw("HIST");
        reference_histograms[0].Draw("HIST same PLC PMC");
        return {std::move(histograms), std::move(reference_histograms)};
    }

    if (MERGE_GRAPHS)
    {
        histograms[0].Draw("HIST");
        histograms[1].Draw("HIST same PLC PMC");
        histograms[2].Draw("HIST same PLC PMC");
        histograms[3].Draw("HIST same PLC PMC");
        histograms[4].Draw("HIST same PLC PMC");
        histograms[5].Draw("HIST same PLC PMC");

        return {std::move(histograms), {}};
    }

    // NOTE: The for here does not work. Bug on root or wrong usage?
    // for (auto histogram : histograms) {
    histograms[0].Draw("HIST");
    reference_histograms[0].Draw("HIST same PLC PMC");
    new TCanvas();
    histograms[1].Draw("HIST");
    reference_histograms[1].Draw("HIST same PLC PMC");
    new TCanvas();
    histograms[2].Draw("HIST");
    reference_histograms[2].Draw("HIST same PLC PMC");
    new TCanvas();
    histograms[3].Draw("HIST");
    reference_histograms[3].Draw("HIST same PLC PMC");
    new TCanvas();
    histograms[4].Draw("HIST");
    reference_histograms[4].Draw("HIST same PLC PMC");
    new TCanvas();
    histograms[5].Draw("HIST");
    reference_histograms[5].Draw("HIST same PLC PMC");
    // }

    // gPad->BuildLegend();

    return {std::move(histograms), std::move(reference_histograms)};
}

