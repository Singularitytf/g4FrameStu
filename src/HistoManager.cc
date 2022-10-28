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
  G4cout << "### Run " << aRun->GetRunID() << " start." << G4endl;
  Book(aRun);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::EndOfRunAction(const G4Run *aRun)
{
  G4int NbOfEvents = aRun->GetNumberOfEventToBeProcessed();
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
  fEng = 0.;
  fDCsI1 = 0.;
  fDCsI2 = 0.;
  inner_compt = 0;
  outter_compt = 0;
  fGenerator.clear();
  fParticle.clear();
  tag_tpe = 0;
  tag_de = 0;
  // brem tags
  tag_dbrem_p = 0;
  tag_dbrem_pe = 0;
  tag_dbrem_c = 0;
  tag_dbrem_ce = 0;
  // neutron filter.
  tag_filt = 0;
  fx = 0.;
  fy = 0.;
  fz = 0.;
  fdx = 0.;
  fdy = 0.;
  fdz = 0.;

  tag_multi_pe = 0;
  tag_md_compt= 0;
  tag_out_dying = 0;

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

void HistoManager::RecordStep(const G4Step *aStep) {

  // get volume of the current step
  //
  fVolume = aStep->GetPreStepPoint()->GetPhysicalVolume();
  fPostVolume = aStep->GetPostStepPoint()->GetTouchableHandle()->GetVolume();
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

  if (fParticle=="neutron"&&
      aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName()=="filter"&&
      aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName()=="counter"&&
      aStep->IsFirstStepInVolume()) {
    tag_filt = 1;
    fEng = fPostKEnergy;
    fx = aStep->GetPostStepPoint()->GetPosition().getX();
    fy = aStep->GetPostStepPoint()->GetPosition().getY();
    fz = aStep->GetPostStepPoint()->GetPosition().getZ();
    fdx = aStep->GetPostStepPoint()->GetMomentumDirection().getX();
    fdy = aStep->GetPostStepPoint()->GetMomentumDirection().getY();
    fdz = aStep->GetPostStepPoint()->GetMomentumDirection().getZ();
  }

}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::Book(const G4Run *aRun)
{
  // Creating a tree container to handle histograms and ntuples.
  // This tree is associated to an output file.
  //
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Reset();
  G4bool fRootFile = analysisManager->OpenFile();
  if (!fRootFile) {
    G4cout << " HistoManager::Book :"
           << " problem creating the ROOT TFile "
           << G4endl;
    return;
  }
  fFactoryOn = true;
  if ( !(aRun->GetRunID()) ) {
    analysisManager->CreateNtuple("eng", "");
    analysisManager->CreateNtupleDColumn(0, "ne"); // neutron energy.
    analysisManager->CreateNtupleDColumn(0, "x");
    analysisManager->CreateNtupleDColumn(0, "y");
    analysisManager->CreateNtupleDColumn(0, "z");
    analysisManager->CreateNtupleDColumn(0, "px");
    analysisManager->CreateNtupleDColumn(0, "py");
    analysisManager->CreateNtupleDColumn(0, "pz");
    analysisManager->FinishNtuple(0);
  }
  
  G4cout << "\n----> Output file is open." << G4endl;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::FillTuple()
{
  // fill ntuple
  //
  G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
  if (tag_filt) {
    analysisManager->FillNtupleDColumn(0, 0, fEng);
    analysisManager->FillNtupleDColumn(0, 1, fx);
    analysisManager->FillNtupleDColumn(0, 2, fy);
    analysisManager->FillNtupleDColumn(0, 3, fz);
    analysisManager->FillNtupleDColumn(0, 4, fdx);
    analysisManager->FillNtupleDColumn(0, 5, fdy);
    analysisManager->FillNtupleDColumn(0, 6, fdz);
    analysisManager->AddNtupleRow(0);
  }

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
