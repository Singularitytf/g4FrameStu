//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file analysis/AnaEx02/src/HistoManager.cc
/// \brief Implementation of the HistoManager class
//
// $Id: HistoManager.cc 98060 2016-07-01 16:24:08Z gcosmo $
// GEANT4 tag $Name: geant4-09-04 $
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include <CLHEP/Units/SystemOfUnits.h>
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "HistoManager.hh"
#include "G4UnitsTable.hh"
#include "EventAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "DetectorConstruction.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ProcessManager.hh"
#include "SteppingVerbose.hh"
#include "G4tgbVolumeMgr.hh"
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HistoManager::HistoManager(DetectorConstruction *det)
    : fFactoryOn(false),
      fPrintModulo(1000000),
      fDetector(det)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HistoManager::~HistoManager()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::BeginOfRunAction(const G4Run *aRun)
{
  G4AnalysisManager ::Instance()->Reset();
  G4cout << "### Run " << aRun->GetRunID() << " start." << G4endl;

  Book();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::EndOfRunAction(const G4Run *aRun)
{
  NbOfEvents = aRun->GetNumberOfEventToBeProcessed();
  if (NbOfEvents == 0)
    return;
  Save();
  // for (auto it = fLevProcName.begin(); it != fLevProcName.end(); ++it)
  //     std::cout << *it<< std::endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::BeginOfEventAction(const G4Event *evt)
{
  // initialisation per event
  fTCsI = 0.;
  fDCsI1 = 0.;
  fDCsI2 = 0.;
  NumOfScat = 0;
  label = 0;
  feBrem = 0.;
  feLeak = 0.;
  fGenerator.clear();
  fParticle.clear();
  trak.Angle = 0.;
  trak.GeEnergy = 0.;
  trak.NaIEnergy = 0.;
  trak.v.clear();
  p_exist.clear(); // clear existed trackid for this event;
  p_stack.clear(); // clear store particle for this event;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::EndOfEventAction(const G4Event *)
{
  FillTuple();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::RecordStep(const G4Step *aStep)
{

  // get volume of the current step
  //
  fVolume = aStep->GetPreStepPoint()->GetPhysicalVolume();
  fPostVolume = aStep->GetPostStepPoint()->GetTouchableHandle()->GetVolume();
  fpostMomentum = aStep->GetPostStepPoint()->GetMomentumDirection();
  fPostKEnergy = aStep->GetPostStepPoint()->GetKineticEnergy() / keV;
  //在这一步的过程的反应
  fProcess = aStep->GetPostStepPoint()
                 ->GetProcessDefinedStep()
                 ->GetProcessName();
  //获得沉积下来的能量
  ftmpEnergy =
      aStep->GetTotalEnergyDeposit() / keV;

  // Get Track
  //
  aTrack = aStep->GetTrack();
  fTrackID = aTrack->GetTrackID();
  fParentID = aTrack->GetParentID();
  fParticle = aTrack->GetDefinition()->GetParticleName();
  fevtid = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
  // G4cout << "work?" << G4endl;

  // check if a new particle
  if (p_exist.find(fTrackID) == p_exist.end())
  {
    // store info of last particle.
    // G4cout << aStep->GetPreStepPoint()->GetKineticEnergy()/keV << G4endl;

    p_exist[fTrackID].reset(fTrackID, fParentID,
                            fevtid, // evt id
                            fParticle,                                                      // particle type.
                            aStep->GetPreStepPoint()->GetKineticEnergy() / keV,             // init energy.
                            fVolume->GetName(),                                             // birth volume
                            // check if primary particle? if so, fill null to crt process,
                            // if not, fill creator process.
                            (fTrackID == 1) ? "NULL" : aTrack->GetCreatorProcess()->GetProcessName());
  }
  (p_exist[fTrackID]).filter[fVolume->GetName()][fProcess] += ftmpEnergy;
  // G4cout << "TrackId: " << fTrackID << "\t" << fVolume->GetName() << "\t" << fProcess << "\t"<< p_exist[fTrackID].filter[fVolume->GetName()][fProcess] << G4endl;
  // G4cout << p_exist[fTrackID].filter["tcsi"]["compt"] << G4endl;

  if (fVolume->GetName() == "tcsi")
  {
    fTCsI += ftmpEnergy;
    // 如果粒子出现在target中，记录trackid。
    if (fTrackID == 1 && fProcess == "compt")
    {
      NumOfScat++;
    }
  }
  if (fVolume->GetName() == "d1csi")
  {
    // G4cout << "123" << G4endl;
    fDCsI1 += ftmpEnergy;
    if (fTrackID == 1)
    {
      if (NumOfScat > 1)
        label = 1;
      // mulitple scattering.
      label += 1;
    }
  }
  if (fVolume->GetName() == "d2csi")
  {
    // G4cout << "123" << G4endl;
    fDCsI2 += ftmpEnergy;
    // if (aStep->IsFirstStepInVolume()&&fTrackID!=1&&(f_tatget_track_id.find(fParentID)!=f_tatget_track_id.end())) {
    //   // G4cout << "AAParntAA" << G4endl;
    //   fGenerator = aTrack->GetCreatorProcess()->GetProcessName();
    //   fParticled = fParticle;
    // }
  }

  //   if (fTrackID == 1 && fParentID == 0) {
  //   auto A = aTrack->GetCurrentStepNumber();
  //   auto B = aTrack->GetPosition().x();
  //   auto C = aTrack->GetPosition().y();
  //   auto D = aTrack->GetPosition().z();
  //   auto E = aTrack->GetKineticEnergy();
  //   auto F = aStep->GetTotalEnergyDeposit();
  //   auto G = aStep->GetStepLength();
  //   auto H = aTrack->GetTrackLength();

  //   G4String I = " ";
  //   if (aTrack->GetNextVolume() != 0)
  //   {
  //     I = aTrack->GetNextVolume()->GetName();
  //   }
  //   else
  //   {
  //     I = "OutWorld";
  //   }

  //   auto J = fProcess;

  //   STEP S(A, B, C, D, E, F, G, H, I, J);
  //   trak.v.push_back(S);
  // }
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::Book()
{
  // Creating a tree container to handle histograms and ntuples.
  // This tree is associated to an output file.
  //
  auto analysisManager = G4AnalysisManager::Instance();
  G4bool fRootFile = analysisManager->OpenFile();
  if (!fRootFile)
  {
    G4cout << " HistoManager::Book :"
           << " problem creating the ROOT TFile "
           << G4endl;
    return;
  }
  fFactoryOn = true;

  analysisManager->CreateNtuple("eng", "");
  analysisManager->CreateNtupleDColumn(0, "tc");
  analysisManager->CreateNtupleDColumn(0, "dc1");
  analysisManager->CreateNtupleDColumn(0, "dc2");
  analysisManager->CreateNtupleIColumn(0, "n");
  analysisManager->CreateNtupleIColumn(0, "evtid"); // particle.
  // analysisManager->CreateNtupleDColumn(0, "x");
  // analysisManager->CreateNtupleDColumn(0, "y");
  // analysisManager->CreateNtupleDColumn(0, "z");
  // analysisManager->CreateNtupleDColumn(0, "e");
  analysisManager->FinishNtuple(0);

  analysisManager->CreateNtuple("e", "");
  analysisManager->CreateNtupleIColumn(1, "tid");
  analysisManager->CreateNtupleIColumn(1, "pid");
  analysisManager->CreateNtupleIColumn(1, "evtid");
  analysisManager->CreateNtupleSColumn(1, "ctrp");  // creator process
  analysisManager->CreateNtupleSColumn(1, "bvm");   // birth place
  analysisManager->CreateNtupleDColumn(1, "ie");    // init energy
  analysisManager->CreateNtupleDColumn(1, "0brem"); // 0 for target CsI.
  analysisManager->CreateNtupleDColumn(1, "0msc");  // 0 for target CsI.
  analysisManager->CreateNtupleDColumn(1, "0eion"); // 0 for target CsI.
  analysisManager->CreateNtupleDColumn(1, "1brem"); // 1 for detector CsI1.
  analysisManager->CreateNtupleDColumn(1, "1msc");  // 1 for detector CsI1.
  analysisManager->CreateNtupleDColumn(1, "1eion"); // 1 for detector CsI1.
  analysisManager->CreateNtupleDColumn(1, "2brem"); // 2 for detector CsI2.
  analysisManager->CreateNtupleDColumn(1, "2msc");  // 2 for detector CsI2.
  analysisManager->CreateNtupleDColumn(1, "2eion"); // 2 for detector CsI2.
  analysisManager->FinishNtuple(1);

  analysisManager->CreateNtuple("gamma", "");
  analysisManager->CreateNtupleIColumn(2, "tid");
  analysisManager->CreateNtupleIColumn(2, "pid");
  analysisManager->CreateNtupleIColumn(2, "evtid");
  analysisManager->CreateNtupleSColumn(2, "ctrp");   // creator process
  analysisManager->CreateNtupleSColumn(2, "bvm");    // birth place
  analysisManager->CreateNtupleDColumn(2, "ie");     // init energy
  analysisManager->CreateNtupleDColumn(2, "0pe");    // 0 for target CsI.
  analysisManager->CreateNtupleDColumn(2, "0compt"); // 0 for target CsI.
  analysisManager->CreateNtupleDColumn(2, "1pe");    // 1 for detector CsI1.
  analysisManager->CreateNtupleDColumn(2, "1compt"); // 1 for detector CsI1.
  analysisManager->CreateNtupleDColumn(2, "2pe");    // 2 for detector CsI2.
  analysisManager->CreateNtupleDColumn(2, "2compt"); // 2 for detector CsI2.
  analysisManager->FinishNtuple(2);
  G4cout << "\n----> Output file is open." << G4endl;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::FillTuple()
{
  // fill ntuple
  //
  G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();

  if (fTCsI*fDCsI1*fDCsI2!=0&&((fDCsI2 < 20 && fDCsI1 > 30 && fDCsI1 < 37) || (fDCsI1 < 20 && fDCsI2 > 30 && fDCsI2 < 37)) && fTCsI + fDCsI1 + fDCsI2 > 660) {
    for (auto i : p_exist)
    {
      if (i.second.type == "e-")
      {
        analysisManager->FillNtupleIColumn(1, 0, i.second.tid);    // tid
        analysisManager->FillNtupleIColumn(1, 1, i.second.pid);    // pid
        analysisManager->FillNtupleIColumn(1, 2, i.second.evt_id); // evtid
        analysisManager->FillNtupleSColumn(1, 3, i.second.crt_proc);
        analysisManager->FillNtupleSColumn(1, 4, i.second.bth_vlm);
        analysisManager->FillNtupleDColumn(1, 5, i.second.init_eng);
        analysisManager->FillNtupleDColumn(1, 6, i.second.filter["tcsi"]["eBrem"]); // 0 pe
        analysisManager->FillNtupleDColumn(1, 7, i.second.filter["tcsi"]["msc"]);
        analysisManager->FillNtupleDColumn(1, 8, i.second.filter["tcsi"]["eIoni"]); // 0 compt
        analysisManager->FillNtupleDColumn(1, 9, i.second.filter["d1csi"]["eBrem"]);
        analysisManager->FillNtupleDColumn(1, 10, i.second.filter["d1csi"]["msc"]);
        analysisManager->FillNtupleDColumn(1, 11, i.second.filter["d1csi"]["eIoni"]);
        analysisManager->FillNtupleDColumn(1, 12, i.second.filter["d2csi"]["eBrem"]);
        analysisManager->FillNtupleDColumn(1, 13, i.second.filter["d2csi"]["msc"]);
        analysisManager->FillNtupleDColumn(1, 14, i.second.filter["d2csi"]["eIoni"]);
        analysisManager->AddNtupleRow(1);
      }
      if (i.second.type == "gamma")
      {
        // G4cout << "work?" << G4endl;
        //     G4cout << G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID() << G4endl;
        // G4cout << "..." << fTrackID << G4endl;
        analysisManager->FillNtupleIColumn(2, 0, i.second.tid);    // tid
        analysisManager->FillNtupleIColumn(2, 1, i.second.pid);    // pid
        analysisManager->FillNtupleIColumn(2, 2, i.second.evt_id); // evtid
        analysisManager->FillNtupleSColumn(2, 3, i.second.crt_proc);
        analysisManager->FillNtupleSColumn(2, 4, i.second.bth_vlm);
        analysisManager->FillNtupleDColumn(2, 5, i.second.init_eng);
        analysisManager->FillNtupleDColumn(2, 6, i.second.filter["tcsi"]["phot"]);  // 0 pe
        analysisManager->FillNtupleDColumn(2, 7, i.second.filter["tcsi"]["compt"]); // 0 compt
        analysisManager->FillNtupleDColumn(2, 8, i.second.filter["d1csi"]["phot"]);
        analysisManager->FillNtupleDColumn(2, 9, i.second.filter["d1csi"]["compt"]);
        analysisManager->FillNtupleDColumn(2, 10, i.second.filter["d2csi"]["phot"]);
        analysisManager->FillNtupleDColumn(2, 11, i.second.filter["d2csi"]["compt"]);
        analysisManager->AddNtupleRow(2);
      }
    }
    analysisManager->FillNtupleDColumn(0, 0, fTCsI);
    analysisManager->FillNtupleDColumn(0, 1, fDCsI1);
    analysisManager->FillNtupleDColumn(0, 2, fDCsI2);
    analysisManager->FillNtupleIColumn(0, 3, NumOfScat);
    analysisManager->FillNtupleIColumn(0, 4, fevtid);
    analysisManager->AddNtupleRow(0);
  }

  // if (fTCsI!=0&&(fDCsI1!=0&&fDCsI2!=0)){
  //   //and fDCsI1 != 0
  // analysisManager->FillNtupleDColumn(0, 0, fTCsI);
  // analysisManager->FillNtupleDColumn(0, 1, fDCsI1);
  // analysisManager->FillNtupleDColumn(0, 2, fDCsI2);
  // analysisManager->FillNtupleIColumn(0, 3, NumOfScat);
  // analysisManager->FillNtupleDColumn(0, 4, feBrem);
  // analysisManager->FillNtupleDColumn(0, 5, feLeak);

  //   // analysisManager->FillNtupleDColumn(0, 5, fPtce);
  //   analysisManager->AddNtupleRow(0);
  // }

  // if (fTCsI!=0){
  //   // analysisManager->FillNtupleDColumn(1, 1, fEeBremO);
  //   analysisManager->FillNtupleDColumn(1, 0, fTCsI);
  //   // analysisManager->FillNtupleIColumn(0, 1, NumOfScat);
  //   analysisManager->AddNtupleRow(1);
  // }
  //

  // trak.Angle = fAngle;
  // trak.GeEnergy = fGeEnergy;
  // trak.NaIEnergy = NaIDepEnergy;
  // SV.push_back(trak);

  // analysisManager->AddNtupleRow(0);
  return;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::Save()
{
  if (!fFactoryOn)
    return;
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile(); // and closing the tree (and the file)

  G4cout << "\n----> Histograms and ntuples are saved\n"
         << G4endl;
  // ST.open("EVent.txt", std::ios::app);
  // if (ST.is_open())
  // {
  //   //将SV容器中的内容打印出来
  //   ST << "统计到的事例数有" << SV.size() << G4endl;
  //   int EventID = 0;
  //   for (std::vector<TRAK>::iterator it = SV.begin(); it != SV.end(); it++,EventID++)
  //   {
  //     ST << std::string(50, '*') << G4endl;
  //     ST << "  EventID： " << EventID;
  //     ST << "  离开Ge的角度： " << it->Angle;
  //     ST << "  Ge中的沉积能量： " << it->GeEnergy;
  //     ST << "  NaI中的沉积能量： " << it->NaIEnergy << G4endl;
  //     ST << std::setw(5) << "Step#"
  //        << std::setw(11) << "X"
  //        << std::setw(13) << "Y"
  //        << std::setw(16) << "Z"
  //        << std::setw(18) << "KineE"
  //        << std::setw(15) << "dEStep"
  //        << std::setw(15) << "StepLeng"
  //        << std::setw(15) << "TrakLeng"
  //        << std::setw(15) << "Volume"
  //        << std::setw(15) << "Process" << G4endl;
  //     for (std::vector<STEP>::iterator io = it->v.begin(); io != it->v.end(); io++)
  //     {
  //       ST << std::setw(5) << io->StepNumber << " "
  //          << std::setw(11) << G4BestUnit(io->X, "Length")
  //          << std::setw(11) << G4BestUnit(io->Y, "Length")
  //          << std::setw(11) << G4BestUnit(io->Z, "Length")
  //          << std::setw(11) << G4BestUnit(io->KE, "Energy")
  //          << std::setw(11) << G4BestUnit(io->DepE, "Energy")
  //          << std::setw(11) << G4BestUnit(io->StepL, "Length")
  //          << std::setw(11) << G4BestUnit(io->TrackL, "Length")
  //          << std::setw(11) << io->VOLUNAME
  //          << std::setw(20) << io->Process << G4endl;
  //     }
  //   }
  // }
  // ST.close();
}
