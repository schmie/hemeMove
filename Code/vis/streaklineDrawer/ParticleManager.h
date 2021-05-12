// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_VIS_STREAKLINEDRAWER_PARTICLEMANAGER_H
#define HEMELB_VIS_STREAKLINEDRAWER_PARTICLEMANAGER_H

#include <vector>
#include <map>

#include "constants.h"
#include "net/mpi.h"

#include "geometry/LatticeData.h"
#include "net/IOCommunicator.h"

#include "vis/streaklineDrawer/NeighbouringProcessor.h"
#include "vis/streaklineDrawer/Particle.h"
#include "vis/streaklineDrawer/VelocityField.h"
#include "vis/streaklineDrawer/VelocitySiteData.h"

namespace hemelb
{
  namespace vis
  {
    namespace streaklinedrawer
    {
      class ParticleManager
      {
        public:
          // Constructor
          ParticleManager(std::map<proc_t, NeighbouringProcessor>& iNeighbouringProcessors);

          // Functions for manipulating the particle store
          void AddParticle(const Particle& iParticle);
          std::vector<Particle>& GetParticles();
          size_t GetNumberOfLocalParticles() const;
          void DeleteParticle(site_t iIndex);
          void DeleteAll();

          // Function for updating the particles' positions.
          void ProcessParticleMovement();

          // Function for moving the particles between cores.
          void CommunicateParticles(net::Net& streakNet, const geometry::LatticeData& iLatDat,
                                    VelocityField& iVelocityField);

        private:
          std::vector<Particle> particles;
          std::map<proc_t, NeighbouringProcessor>& neighbouringProcessors;

      };
    }
  }
}

#endif // HEMELB_VIS_STREAKLINEDRAWER_PARTICLEMANAGER_H
