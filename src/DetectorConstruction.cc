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
/// \file DetectorConstruction.cc
/// \brief Implementation of the DetectorConstruction class
//
//
// $Id: DetectorConstruction.cc 101905 2016-12-07 11:34:39Z gunter $
//
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DetectorConstruction.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4VisAttributes.hh"

#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"

#include "G4RunManager.hh"
#include "G4SubtractionSolid.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction(),
      fWorldMaterial(0), fTargetMat(0),
      fSolidWorld(0), fLogicWorld(0), fPhysiWorld(0),
      fComptTarget(0), fLVComptTarget(0),
      fPComptTarget(0),
      fCheckOverlaps(true)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
DetectorConstruction::~DetectorConstruction()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4VPhysicalVolume *DetectorConstruction::Construct()
{
  DefineMaterials();
  return ConstructCalorimeter();
}

void DetectorConstruction::DefineMaterials()
{
  G4NistManager *man = G4NistManager::Instance();
  fWorldMaterial  = man->FindOrBuildMaterial("G4_AIR");
  fTargetMat      = man->FindOrBuildMaterial("G4_CESIUM_IODIDE");
  fHPGeMat        = man->FindOrBuildMaterial("G4_Ge");

  // print table
  //
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;

  // Clean old geometry, if any
  //
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();
}

G4VPhysicalVolume *DetectorConstruction::ConstructCalorimeter()
{
  //
  // World
  //
  fSolidWorld = new G4Box("World", 10. * m, 10. * m, 10. * m);
  fLogicWorld = new G4LogicalVolume(fSolidWorld, fWorldMaterial, "World");
  fPhysiWorld = new G4PVPlacement(0,
                                  G4ThreeVector(),
                                  fLogicWorld,
                                  "World",
                                  0,
                                  false,
                                  0,
                                  fCheckOverlaps);

  G4int fHalfHeight = 107;

  //
  // Compton target
  //
  G4RotationMatrix fRTgt = G4RotationMatrix();
  fRTgt.rotateZ(0*deg);
  G4ThreeVector tgtPosition = G4ThreeVector(0., 0., 0 * mm);
  G4Transform3D fTtransform = G4Transform3D(fRTgt, tgtPosition);
  fComptTarget = new G4Box("G4_CESIUM_IODIDE",
                                1 * cm,
                                3 * cm,
                                3 * cm);
                                
  fLVComptTarget = new G4LogicalVolume(fComptTarget,
                                       fTargetMat,
                                       "G4_CESIUM_IODIDE");
  fPComptTarget =
    new G4PVPlacement(fTtransform,
                                    fLVComptTarget,
                                    "G4_CESIUM_IODIDE",
                                    fLogicWorld,
                                    false,
                                    0,
                                    fCheckOverlaps);
                                    
  fHPGePosition = G4ThreeVector(100*std::cos(120 * deg) * cm, 100*std::sin(120 * deg) * cm, 0);
  G4RotationMatrix fRHPGe = G4RotationMatrix();
  fRHPGe.rotateX(0*deg);
  G4Transform3D transform = G4Transform3D(fRHPGe, fHPGePosition);
  fHPGe   = new G4Tubs("HPGe", 0 * mm, 8 * mm, 5 * mm, 0 * deg, 360 * deg);
  fLVHPGe = new G4LogicalVolume(fHPGe, fHPGeMat, "HPGe");
  fPHPGe  = new G4PVPlacement(transform, fLVHPGe, "HPGe", fLogicWorld, false, 0, fCheckOverlaps);
                                  


  // Invisible the world.
  fLogicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

  G4VisAttributes *simpleBoxVisAtt = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0));
  simpleBoxVisAtt->SetVisibility(true);
  fLVComptTarget->SetVisAttributes(simpleBoxVisAtt);


  // fLogicHPGeShield->SetVisAttributes(simpleBoxVisAtt);
  // fLogicSecondDetector->SetVisAttributes(simpleBoxVisAtt);
  // fLogicalShield->SetVisAttributes(simpleBoxVisAtt);
  //
  //always return the physical World
  //
  return fPhysiWorld;
}
