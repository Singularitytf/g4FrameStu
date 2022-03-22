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
/// \file DetectorConstruction.hh
/// \brief Definition of the DetectorConstruction class
//
//
// $Id: DetectorConstruction.hh 98242 2016-07-04 16:57:39Z gcosmo $
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"

class G4Box;
class G4Tubs;
class G4SubtractionSolid;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
  
    DetectorConstruction();
    virtual ~DetectorConstruction();

  public:
     virtual G4VPhysicalVolume* Construct();

  public:
     G4Material *GetTargetFoilMaterial() { return fFoil; }
     const G4VPhysicalVolume *GetphysiWorld() { return fPhysiWorld; };
     const G4VPhysicalVolume *GetCsIDetector() const { return fPCsI; };
     const G4VPhysicalVolume *GetHPGe() const { return fPHPGe; };

  private:
  // Define mat.
     G4Material *fWorldMaterial;
     G4Material *fCsIMat, *fHPGeMat, *fFoil;
  // Define World Geo.
     G4Box *fSolidWorld;
     G4LogicalVolume *fLogicWorld;
     G4VPhysicalVolume *fPhysiWorld;
  // Define Compton target foil.
     G4Box  *fCsI;
     G4Tubs *fHPGe;
     G4LogicalVolume *fLVHPGe, *fLVCsI;
     G4VPhysicalVolume *fPHPGe, *fPCsI;
     G4ThreeVector fHPGePosition;

  private:
     G4bool fCheckOverlaps;
     void DefineMaterials();
     G4VPhysicalVolume* ConstructCalorimeter();     
};


#endif

