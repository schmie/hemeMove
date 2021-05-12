// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_UNITS_H
#define HEMELB_UNITS_H

#include <cstdint>
#include "util/Vector3D.h"

namespace hemelb
{
  // Basic types for use in HemeLB.

  // ------ OLD POLICY:  ---------
  //Any variable which scales as a function of the number of sites
  // can have type site_t, processors proc_t.
  // Any variable whose precision should roughly match that of the lattice sites' velocity
  // distributions can have type distribn_t.
  typedef int64_t site_t;
  typedef int proc_t;
  typedef double distribn_t;
  typedef unsigned Direction;
  typedef uint64_t sitedata_t;

  // ------- NEW POLICY -------------
  // Types should reflect the meaning of a quantity as well as the precision
  // the type name should reflect the dimensionality and the base of the units

  typedef double PhysicalDensity;
  typedef distribn_t LatticeDensity;
  typedef float ScreenDensity;

  typedef double PhysicalPressure;
  typedef distribn_t LatticePressure;

  typedef double PhysicalStress;
  typedef distribn_t LatticeStress;
  typedef float ScreenStress;

  typedef unsigned long LatticeTimeStep; // lattice time steps.
  typedef double LatticeTime;
  typedef double PhysicalTime; // seconds

  typedef distribn_t LatticeReciprocalTime; ///< 1/timestep
  typedef double PhysicalReciprocalTime; ///< 1/seconds
  typedef double PhysicalRate; // inverse seconds / Hz

  typedef double PhysicalMass; // kilograms

  typedef double Angle;

  // TODO: deprecated, use PhysicalDistance instead - should be fixed as part of ticket #437
  typedef double PhysicalLength_deprecated;

  typedef double PhysicalDistance; // continuous distance in physical units
  typedef double LatticeVolume; // continuous volume in physical units
  typedef double LatticeArea; // continuous area in physical units
  typedef double LatticeDistance; // continuous distance in lattice units
  typedef int64_t LatticeCoordinate; // discrete distance in lattice units

  typedef util::Vector3D<LatticeDistance> LatticePosition; // origin of lattice is at {0.0,0.0,0.0}
  typedef util::Vector3D<LatticeCoordinate> LatticeVector; // origin of lattice is at {0,0,0}

  typedef util::Vector3D<PhysicalDistance> PhysicalPosition;

  typedef double PhysicalEnergy; // type for energy
  typedef double PhysicalForce; // continuous scalar force in physical units
  typedef double LatticeEnergy; // continuous scalar energy in lattice units
  typedef double LatticeForce; // continuous scalar force in lattice units
  typedef double LatticeModulus; // placeholder for any moduli, though actual dimension may differ
  typedef util::Vector3D<LatticeForce> LatticeForceVector; // continuous force in lattice units

  // TODO: xxxVelocity is a Vector3D<xxxSpeed> not a scalar - should be fixed as part of ticket #437
  typedef double PhysicalVelocity_deprecated;

  typedef double PhysicalSpeed;
  typedef double LatticeSpeed;
  typedef util::Vector3D<PhysicalSpeed> PhysicalVelocity;
  typedef util::Vector3D<LatticeSpeed> LatticeVelocity;

  typedef double PhysicalPressureGradient;
  typedef double LatticePressureGradient;

  typedef double PhysicalDynamicViscosity;
  typedef double PhysicalKinematicViscosity;
  typedef double LatticeDynamicViscosity;
  typedef double LatticeKinematicViscosity;

  typedef double Dimensionless;
}
#endif //HEMELB_UNITS_H
