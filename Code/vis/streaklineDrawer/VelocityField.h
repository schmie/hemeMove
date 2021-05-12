// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_VIS_STREAKLINEDRAWER_VELOCITYFIELD_H
#define HEMELB_VIS_STREAKLINEDRAWER_VELOCITYFIELD_H

#include <vector>
#include <map>

#include "constants.h"
#include "net/mpi.h"

#include "geometry/LatticeData.h"
#include "lb/MacroscopicPropertyCache.h"
#include "net/IOCommunicator.h"

#include "vis/streaklineDrawer/NeighbouringProcessor.h"
#include "vis/streaklineDrawer/VelocitySiteData.h"

namespace hemelb
{
  namespace vis
  {
    namespace streaklinedrawer
    {
      class VelocityField
      {
        public:
          VelocityField(proc_t localRank,
                        std::map<proc_t, NeighbouringProcessor>& iNeighbouringProcessors,
                        const lb::MacroscopicPropertyCache& propertyCache);

          void BuildVelocityField(const geometry::LatticeData& latDat);

          bool BlockContainsData(size_t iBlockNumber) const;

          VelocitySiteData* GetVelocitySiteData(const geometry::LatticeData& latDat,
                                                const util::Vector3D<site_t>& location);

          void GetVelocityFieldAroundPoint(const util::Vector3D<site_t> location,
                                           const geometry::LatticeData& latDat,
                                           util::Vector3D<float> localVelocityField[2][2][2]);

          util::Vector3D<float>
          InterpolateVelocityForPoint(
              const util::Vector3D<float> position,
              const util::Vector3D<float> localVelocityField[2][2][2]) const;

          void InvalidateAllCalculatedVelocities();

          void UpdateLocalField(const util::Vector3D<site_t>& position,
                                const geometry::LatticeData& latDat);

          bool NeededFromNeighbour(const util::Vector3D<site_t> location,
                                   const geometry::LatticeData& latDat, proc_t* sourceProcessor);

        private:
          void UpdateLocalField(VelocitySiteData* localVelocitySiteData,
                                const geometry::LatticeData& latDat);

          // Counter to make sure the velocity field blocks are correct for the current iteration.
          site_t counter;

          VelocitySiteData& GetSiteData(site_t iBlockNumber, site_t iSiteNumber);

          void InitializeVelocityFieldBlock(const geometry::LatticeData& latDat,
                                            const util::Vector3D<site_t> location,
                                            const proc_t proc_id);

          const proc_t localRank;
          // Vector containing VelocityFields
          std::vector<std::vector<VelocitySiteData> > velocityField;
          std::map<proc_t, NeighbouringProcessor>& neighbouringProcessors;
          const lb::MacroscopicPropertyCache& propertyCache;
      };
    }
  }
}

#endif // HEMELB_VIS_STREAKLINEDRAWER_VELOCITYFIELD_H
