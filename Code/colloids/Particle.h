// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_COLLOIDS_PARTICLE_H
#define HEMELB_COLLOIDS_PARTICLE_H

#include "net/mpi.h"
#include "colloids/PersistedParticle.h"
#include "geometry/LatticeData.h"
#include "io/xml/XmlAbstractionLayer.h"
#include "lb/MacroscopicPropertyCache.h"
#include "util/Vector3D.h"
#include "io/writers/Writer.h"

namespace hemelb
{
  namespace colloids
  {
    struct ParticleSorter;

    /**
     * represents a single simulated biocolloid particle
     *
     * all persisted properties, i.e. those that are read in from a config file,
     * are inherited from the PersistedParticle class (which handles the I/O)
     */
    class Particle : PersistedParticle
    {
      public:
        /** constructor - gets initial values from an xml configuration file */
        Particle(const geometry::LatticeData& latDatLBM, const hemelb::lb::LbmParameters *lbmParams,
                 io::xml::Element& xml);

        /** constructor - gets an invalid particle for making MPI data types */
        Particle()
        {
        }
        ;

        /** property getter for particleId */
        const unsigned long GetParticleId() const
        {
          return particleId;
        }
        const LatticePosition& GetGlobalPosition() const
        {
          return globalPosition;
        }

        const LatticeVelocity GetVelocity() const
        {
          return velocity + bodyForces * CalculateDragCoefficient() + lubricationVelocityAdjustment;
        }

        const PhysicalMass GetMass() const
        {
          return mass;
        }
        const LatticeDistance GetRadius() const
        {
          return smallRadius_a0;
        }

        /**
         * normalised particle radius
         *
         * a = 1/(1/a0 - 1/ah) = 1/((ah-a0)/(a0*ah)) = (a0*ah)/(ah-a0)
         */
        const LatticeDistance GetNormalisedRadius() const
        {
          return (smallRadius_a0 * largeRadius_ah) / (largeRadius_ah - smallRadius_a0);
        }

        /**
         * inverse of normalised particle radius
         *
         * 1/a = (1/a0 - 1/ah) = (ah-a0)/(a0*ah)
         */
        const LatticeDistance GetInverseNormalisedRadius() const
        {
          return (largeRadius_ah - smallRadius_a0) / (smallRadius_a0 * largeRadius_ah);
        }

        const LatticeTimeStep& GetLastCheckpointTimestep() const
        {
          return lastCheckpointTimestep;
        }
        const LatticeTimeStep& GetDeletionMarker() const
        {
          return markedForDeletionTimestep;
        }

        /** unsets the deletion marker - the particle will not be deleted */
        const void SetDeletionMarker()
        {
          markedForDeletionTimestep = SITE_OR_BLOCK_SOLID;
        }

        /** sets the deletion marker to the current timestep
         *  the particle will be deleted after the next checkpoint
         */
        const void SetDeletionMarker(LatticeTimeStep timestep)
        {
          if (timestep < markedForDeletionTimestep)
            markedForDeletionTimestep = timestep;
        }

        /** property getter for ownerRank */
        const proc_t GetOwnerRank() const
        {
          return ownerRank;
        }

        /** property getter for isValid */
        const bool IsValid() const
        {
          return isValid;
        }

        /**
         * less than operator for comparing particle objects
         *
         * when used to sort a container of particle objects
         * the ordering produced by this operator is:
         * - increasing particleId
         * - grouped by owner rank
         * - with local rank first
         */
        //const bool operator<(const Particle& other) const;
        /** determines if the owner rank of this particle is an existing key in map */
        const bool IsOwnerRankKnown(
            std::map<proc_t, std::pair<unsigned int, unsigned int> > map) const;

        const bool IsReadyToBeDeleted() const;

        /** for debug purposes only - outputs all properties to info log */
        const void OutputInformation() const;

        /** for serialisation into output file */
        const void WriteToStream(const LatticeTimeStep currentTimestep,
                                 io::writers::Writer& writer);

        /** obtains the fluid viscosity at the position of this particle */
        // TODO: currently returns BLOOD_VISCOSITY_Pa_s, which has the wrong units
        const Dimensionless GetViscosity() const;

        /** calculates the drag coefficient = 1/(6*pi*viscosity*radius) */
        const Dimensionless CalculateDragCoefficient() const;

        /** updates the position of this particle using body forces and fluid velocity */
        const void UpdatePosition(const geometry::LatticeData& latDatLBM);

        /** calculates the effects of all body forces on this particle */
        const void CalculateBodyForces();

        /** calculates the effects of this particle on each lattice site */
        const void CalculateFeedbackForces(const geometry::LatticeData& latDatLBM) const;

        /** interpolates the fluid velocity to the location of each particle */
        const void InterpolateFluidVelocity(const geometry::LatticeData& latDatLBM,
                                            const lb::MacroscopicPropertyCache& propertyCache);

        /** accumulate contributions to velocity from remote processes */
        const void AccumulateVelocity(util::Vector3D<double>& contribution)
        {
          velocity += contribution;
        }
        ;

        /** sets the value for the velocity adjustment due to the lubrication BC */
        const void SetLubricationVelocityAdjustment(const LatticeVelocity adjustment)
        {
          lubricationVelocityAdjustment = adjustment;
        }

        /** creates a derived MPI datatype that represents a single particle object
         *  the fields included are all those from the PersistedParticle base class
         *  note - this data type uses displacements rather than absolute addresses
         *  refer to Example 4.17 on pp114-117 of the MPI specification version 2.2
         *  when you no longer need this type, remember to call MPI_Type_free
         */
        const MPI_Datatype CreateMpiDatatypeWithPosition() const;

        /** creates a derived MPI datatype that represents a single particle object
         *  the fields included in this type are: particleId and velocity(xyz) only
         *  note - this data type uses displacements rather than absolute addresses
         *  refer to Example 4.17 on pp114-117 of the MPI specification version 2.2
         *  when you no longer need this type, remember to call MPI_Type_free
         */
        const MPI_Datatype CreateMpiDatatypeWithVelocity() const;

      private:
        /** partial interpolation of fluid velocity - temporary value only */
        LatticeVelocity velocity;

        /** the effect of all body forces on this particle - this is NOT a force vector */
        LatticeVelocity bodyForces;

        /* an adjustment to the velocity from the LubricationBC boundary condition */
        LatticeVelocity lubricationVelocityAdjustment;

        /* supplies the value of tau for calculating viscosity */
        const hemelb::lb::LbmParameters *lbmParams;

        proc_t ownerRank;

        bool isValid;
        // Allow the sorter class to see our private members.
        friend struct ParticleSorter;
    };
  }
  namespace net
  {
    template<>
    MPI_Datatype MpiDataTypeTraits<colloids::Particle>::RegisterMpiDataType();
    template<>
    MPI_Datatype MpiDataTypeTraits<std::pair<unsigned long, util::Vector3D<double> > >::RegisterMpiDataType();
  }
}

#endif /* HEMELB_COLLOIDS_PARTICLE_H */
