// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_VIS_RAYTRACER_CLUSTERNORMAL_H
#define HEMELB_VIS_RAYTRACER_CLUSTERNORMAL_H

#include "vis/rayTracer/Cluster.h"
#include "vis/rayTracer/SiteData.h"

namespace hemelb
{
  namespace vis
  {
    namespace raytracer
    {
      class ClusterNormal : public Cluster<ClusterNormal>
      {
        public:
          ClusterNormal(unsigned short xBlockCount, unsigned short yBlockCount,
                        unsigned short zBlockCount, const util::Vector3D<float>& minimalSite,
                        const util::Vector3D<float>& maximalSite,
                        const util::Vector3D<float>& minimalSiteOnMinimalBlock,
                        const util::Vector3D<site_t>& minimalBlock);

          const util::Vector3D<double>* DoGetWallData(site_t iBlockNumber,
                                                      site_t iSiteNumber) const;

          void DoSetWallData(site_t iBlockNumber, site_t iSiteNumber,
                             const util::Vector3D<double>& iData);
      };

    }
  }
}

#endif // HEMELB_VIS_RAYTRACER_CLUSTERNORMAL_H
