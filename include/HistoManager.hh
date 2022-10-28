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
/// \file analysis/AnaEx02/include/HistoManager.hh
/// \brief Definition of the HistoManager class
//
// $Id: HistoManager.hh 98060 2016-07-01 16:24:08Z gcosmo $
// GEANT4 tag $Name: geant4-09-04 $
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef HistoManager_h
#define HistoManager_h 1

#include "G4AnalysisManager.hh"
#include "globals.hh"
#include "TMath.h"
#include "DetectorConstruction.hh"
#include "G4Track.hh"
#include "G4EventManager.hh"
#include <fstream>
#include <vector>
#include <set>
#include <deque>
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorConstruction;
class G4Run;
class G4Step;
class G4Event;

class STEP
{
public:
  STEP(G4int A, G4double B, G4double C, G4double D, G4double E, G4double F, G4double G, G4double H, G4String I, G4String J)
  {
    this->StepNumber = A;
    this->X = B;
    this->Y = C;
    this->Z = D;
    this->KE = E;
    this->DepE = F;
    this->StepL = G;
    this->TrackL = H;
    this->VOLUNAME = I;
    this->Process = J;
  }
  ~STEP(){};
  G4int StepNumber;
  G4double X;
  G4double Y;
  G4double Z;
  G4double KE;
  G4double DepE;
  G4double StepL;
  G4double TrackL;
  G4String VOLUNAME;
  G4String Process;
};
class TRAK
{
public:
  std::vector<STEP> v;
  double Angle;
  double GeEnergy;
  double NaIEnergy;
};

class collector { // a particle information collector.
  public:
    G4String type;
    G4int tid; // track id;
    G4int pid; // parent id;
    G4int evt_id; // event id;
    G4double init_eng;
    G4String crt_proc; // ceator process
    G4String bth_vlm; // birth place
    std::map<G4String/*Physics Volume*/, std::map<G4String/*Physics Process*/, G4double/*Energy Deposit*/> > filter;
    void reset(G4int ftid, G4int fpid, G4int fevtid, G4String p_type, G4double eng, G4String bth_plc,G4String proc) {
      init_eng=eng; 
      tid=ftid; 
      pid=fpid; 
      evt_id=fevtid; 
      crt_proc = proc;
      type = p_type;
      bth_vlm = bth_plc;
      // G4cout << "Type is: "<<type << G4endl; 
      filter.clear();
    };
    collector() : tid(0), pid(0), evt_id(0) {};
  };

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class HistoManager
{
public:
  HistoManager(DetectorConstruction *);
  ~HistoManager();

  void Book(const G4Run *aRun);
  void Save();
  void FillTuple();
  void recordDepEng();
  // Following Func show primary photon generate electron.
  void ShowProcessGenerateEle(const G4Step *aStep);
  // Print the TrackID stroed in vector.
  void ShowCompteTrackStatus(std::vector<G4int> aVec);



  void BeginOfRunAction(const G4Run *);
  void EndOfRunAction(const G4Run *);

  void BeginOfEventAction(const G4Event *);
  void EndOfEventAction(const G4Event *);

  void RecordStep(const G4Step *);
private:
  G4double preX, preY, preZ;


  G4bool fFactoryOn;
  G4int fPrintModulo;
  DetectorConstruction *fDetector;

  // Recordstep
  //G4bool fHPGeFire, fFoilFire;
  G4int fNumOfComptAtGe, fTrackID, fParentID, inner_compt, outter_compt, fevtid;
  G4double  fAngle, fTCsI, fGammaFinalEnergy,
    fDCsI1, fDCsI2, ftmpEnergy, fPostKEnergy, fEng, fx, fy, fz, fdx, fdy, fdz;
  G4String fProcess, fGenerator, fParticle, fParticled;
  G4VPhysicalVolume *fVolume, *fPostVolume;
  G4Track *aTrack;
  collector p_colle;
  std::map<G4int, class collector> p_exist; // collect track id existed;
  std::deque<class collector> p_stack;
  // std::map<G4String, G4String> det_map = {{"tcsi", "0"}, {"dcsi1", "1"},{"dcsi2", "2"}}; // map detector name to its alias.
  G4int tag_tpe, tag_de, tag_multi_pe, tag_md_compt, tag_out_dying, tag_filt;
  G4double tag_dbrem_p, tag_dbrem_pe, tag_dbrem_c, tag_dbrem_ce;
  G4EventManager *evtMgr = G4EventManager::GetEventManager();
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
  
  std::vector<TRAK> SV;
  TRAK trak;
  std::ofstream ST;
};
  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
