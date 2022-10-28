void plotHist(){
  auto brem_hist = new TH1F("ebremHist", "eBrem histo", 100, 0, 100);
  auto file = new TFile("Anaex02.root");
  auto tree = (TTree*)file->Get("GeDepEnergy");
  double febrem;
  tree->GetBranchAddress("eBrem", febrem);
  Long64_t nentries = tree->GetEntries();
  for (int i = 0; i<nentries; ++i){
    tree->GetEntry(i);
    brem_hist->Fill(febrem);
  }
  brem_hist->Draw();
}
