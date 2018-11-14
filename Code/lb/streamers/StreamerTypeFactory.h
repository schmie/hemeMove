
// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_LB_STREAMERS_STREAMERTYPEFACTORY_H
#define HEMELB_LB_STREAMERS_STREAMERTYPEFACTORY_H

#include "lb/kernels/BaseKernel.h"
#include "lb/streamers/BaseStreamer.h"
#include "lb/streamers/SimpleCollideAndStreamDelegate.h"

namespace hemelb
{
  namespace lb
  {
    namespace streamers
    {
      void DoStreamAndCollideGPU(
        site_t firstIndex,
        site_t siteCount,
        distribn_t lbmParams_tau,
        distribn_t lbmParams_omega,
        const iolets::InOutLetCosineGPU* inlets,
        const iolets::InOutLetCosineGPU* outlets,
        const site_t* neighbourIndices,
        const geometry::SiteData* siteData,
        const distribn_t* fOld,
        distribn_t* fNew,
        unsigned long timeStep
      );

      /**
       * Template to produce Streamers that can cope with fluid-fluid,
       * fluid-wall and fluid-iolet links. Requires three classes as arguments:
       * 1) the Collision class,
       * 2) a StreamerDelegate class that will handle the wall links, and
       * 3) a StreamerDelegate class that will handle the iolet links.
       *
       * It is intended that a simpler metafunction partially specialise this
       * template on WallLinkImpl and IoletLinkImpl.
       */
      template<typename CollisionImpl, typename WallLinkImpl, typename IoletLinkImpl>
      class StreamerTypeFactory : public BaseStreamer<StreamerTypeFactory<CollisionImpl,
          WallLinkImpl, IoletLinkImpl> >
      {
        public:
          typedef CollisionImpl CollisionType;
          typedef typename CollisionType::CKernel::LatticeType LatticeType;

        private:
          CollisionType collider;
          SimpleCollideAndStreamDelegate<CollisionType> bulkLinkDelegate;
          WallLinkImpl wallLinkDelegate;
          IoletLinkImpl ioletLinkDelegate;

        public:
          StreamerTypeFactory(kernels::InitParams& initParams) :
            collider(initParams), bulkLinkDelegate(collider, initParams), wallLinkDelegate(collider, initParams),
                ioletLinkDelegate(collider, initParams)
          {

          }

          template<bool tDoRayTracing>
          inline void DoStreamAndCollide(const site_t firstIndex,
                                         const site_t siteCount,
                                         const LbmParameters* lbmParams,
                                         geometry::LatticeData* latDat,
                                         lb::MacroscopicPropertyCache& propertyCache,
                                         lb::SimulationState* simState)
          {
            if ( siteCount == 0 )
            {
              return;
            }

            if (lbmParams->UseGPU() && !propertyCache.RequiresRefresh())
            {
              // launch DoStreamAndCollide kernel
              DoStreamAndCollideGPU(
                firstIndex,
                siteCount,
                lbmParams->GetTau(),
                lbmParams->GetOmega(),
                inlets_dev,
                outlets_dev,
                latDat->GetNeighbourIndicesGPU(),
                latDat->GetSiteDataGPU(),
                latDat->GetFOldGPU(0),
                latDat->GetFNewGPU(0),
                simState->Get0IndexedTimeStep()
              );
              CUDA_SAFE_CALL(cudaGetLastError());
            }

            else
            {
              for (site_t siteIndex = firstIndex; siteIndex < (firstIndex + siteCount); siteIndex++)
              {
                geometry::Site<geometry::LatticeData> site = latDat->GetSite(siteIndex);

                const distribn_t* fOld = site.GetFOld<LatticeType> ();

                kernels::HydroVars<typename CollisionType::CKernel> hydroVars(fOld);

                ///< @todo #126 This value of tau will be updated by some kernels within the collider code (e.g. LBGKNN). It would be nicer if tau is handled in a single place.
                hydroVars.tau = lbmParams->GetTau();

                collider.CalculatePreCollision(hydroVars, site);

                collider.Collide(lbmParams, hydroVars);

                for (Direction ii = 0; ii < LatticeType::NUMVECTORS; ii++)
                {
                  if (site.HasIolet(ii))
                  {
                    ioletLinkDelegate.StreamLink(lbmParams, latDat, site, hydroVars, ii);
                  }
                  else if (site.HasWall(ii))
                  {
                    wallLinkDelegate.StreamLink(lbmParams, latDat, site, hydroVars, ii);
                  }
                  else
                  {
                    bulkLinkDelegate.StreamLink(lbmParams, latDat, site, hydroVars, ii);
                  }
                }

                //TODO: Necessary to specify sub-class?
                BaseStreamer<StreamerTypeFactory>::template UpdateMinsAndMaxes<tDoRayTracing>(site,
                                                                                              hydroVars,
                                                                                              lbmParams,
                                                                                              propertyCache);
              }
            }
          }

          template<bool tDoRayTracing>
          inline void DoPostStep(const site_t firstIndex,
                                 const site_t siteCount,
                                 const LbmParameters* lbmParams,
                                 geometry::LatticeData* latticeData,
                                 lb::MacroscopicPropertyCache& propertyCache)
          {
            for (site_t siteIndex = firstIndex; siteIndex < (firstIndex + siteCount); siteIndex++)
            {
              geometry::Site<geometry::LatticeData> site = latticeData->GetSite(siteIndex);
              for (unsigned int direction = 0; direction < LatticeType::NUMVECTORS; direction++)
              {
                if (site.HasWall(direction))
                {
                  wallLinkDelegate.PostStepLink(latticeData, site, direction);
                }
                else if (site.HasIolet(direction))
                {
                  ioletLinkDelegate.PostStepLink(latticeData, site, direction);
                }
              }
            }

          }
      };
    }
  }
}
#endif // HEMELB_LB_STREAMERS_STREAMERTYPEFACTORY_H
