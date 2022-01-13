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

#include "g4root.hh"
#include "globals.hh"
#include "TMath.h"
#include "DetectorConstruction.hh"
#include "G4Track.hh"
#include <fstream>
#include <vector>
#include <set>
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorConstruction;
class G4Run;
class G4Step;
class G4Event;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class HistoManager
{
public:
  HistoManager(DetectorConstruction *);
  ~HistoManager();

  void Book();
  void Save();
  void FillTuple();
  void recordDepEng();
  void RecordGammaFinalAngle();
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


  G4String fParticle;
  G4bool fFactoryOn;
  G4int fPrintModulo;
  DetectorConstruction *fDetector;

  // Recordstep
  //G4bool fHPGeFire, fFoilFire;
  G4int fNumOfComptAtGe, fTrackID, fParentID, fNumOfe, ftmp, NbOfEvents, pMProcNum, pNBrem;
  G4double  fAngle, fCsIDepEng, fGammaFinalEnergy,
    fHPGeEng, fPostKEnergy, ftmpEnergy, fEeBrem, fEeBremO;
  G4String fProcess, fGenerator;
  G4VPhysicalVolume *fVolume, *fPostVolume;
  G4Track *aTrack;
  std::vector<G4int> fCompteTrackIDVec;
  std::set<G4String> fLProcName, fLevProcName;

  // RecordGammaFinalAngle
  G4ThreeVector fpostMomentum;
  G4double postX, postY, postZ;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
