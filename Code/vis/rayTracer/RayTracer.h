// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_VIS_RAYTRACER_RAYTRACER_H
#define HEMELB_VIS_RAYTRACER_RAYTRACER_H

#include <map>
#include <stack>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>

#include "constants.h"
#include "geometry/LatticeData.h"
#include "lb/LbmParameters.h"
#include "log/Logger.h"
#include "net/IOCommunicator.h"
#include "util/utilityFunctions.h" 
#include "util/Vector3D.h"
#include "vis/DomainStats.h"
#include "vis/PixelSet.h"
#include "vis/PixelSetStore.h"
#include "vis/Screen.h"
#include "vis/Viewpoint.h"
#include "vis/VisSettings.h"
#include "vis/XYCoordinates.h"
#include "vis/rayTracer/Cluster.h"
#include "vis/rayTracer/ClusterBuilder.h"
#include "vis/rayTracer/ClusterRayTracer.h"
#include "vis/rayTracer/Ray.h"
#include "vis/rayTracer/RayTracer.h"
#include "vis/rayTracer/SiteData.h"

namespace hemelb
{
  namespace vis
  {
    namespace raytracer
    {
      template<typename ClusterType, typename RayDataType>
      class RayTracer : public PixelSetStore<PixelSet<RayDataType> >
      {
        public:
          // Constructor and destructor do all the usual stuff.
          RayTracer(const geometry::LatticeData* iLatDat, const DomainStats* iDomainStats,
                    Screen* iScreen, Viewpoint* iViewpoint, VisSettings* iVisSettings) :
              mClusterBuilder(iLatDat, iLatDat->GetLocalRank()), mLatDat(iLatDat),
                  mDomainStats(iDomainStats), mScreen(iScreen), mViewpoint(iViewpoint),
                  mVisSettings(iVisSettings)
          {
            mClusterBuilder.BuildClusters();
          }

          ~RayTracer()
          {
          }

          // Render the current state into an image.
          PixelSet<RayDataType>* Render(const lb::MacroscopicPropertyCache& propertyCache)
          {
            PixelSet<RayDataType>* pixels =
                PixelSetStore<PixelSet<RayDataType> >::GetUnusedPixelSet();
            pixels->Clear();

            ClusterRayTracer<ClusterType, RayDataType> lClusterRayTracer(*mViewpoint,
                                                                         *mScreen,
                                                                         *mDomainStats,
                                                                         *mVisSettings,
                                                                         *mLatDat,
                                                                         propertyCache);

            for (unsigned int clusterId = 0; clusterId < mClusterBuilder.GetClusters().size();
                clusterId++)
            {
              lClusterRayTracer.RenderCluster(mClusterBuilder.GetClusters()[clusterId], *pixels);
            }

            return pixels;
          }

        private:
          ClusterBuilder<ClusterType> mClusterBuilder;
          const geometry::LatticeData* mLatDat;

          const DomainStats* mDomainStats;
          Screen* mScreen;
          Viewpoint* mViewpoint;
          VisSettings* mVisSettings;
      };
    }
  }
}

#endif // HEMELB_VIS_RAYTRACER_RAYTRACER_H
