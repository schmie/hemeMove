// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include <iostream>

#include "util/Vector3D.h"
#include "vis/DomainStats.h"
#include "vis/rayTracer/RayDataNormal.h"

namespace hemelb
{
  namespace vis
  {
    namespace raytracer
    {

      RayDataNormal::RayDataNormal(int i, int j) :
          RayData<RayDataNormal>(i, j)
      {
        mVelR = 0.0F;
        mVelG = 0.0F;
        mVelB = 0.0F;

        mStressR = 0.0F;
        mStressG = 0.0F;
        mStressB = 0.0F;
      }

      RayDataNormal::RayDataNormal()
      {

      }

      void RayDataNormal::DoUpdateDataForNormalFluidSite(const SiteData_t& iSiteData,
                                                         const util::Vector3D<float>& iRayDirection,
                                                         const float iRayLengthInVoxel,
                                                         const VisSettings& iVisSettings)
      {
        float lPalette[3];

        // update the volume rendering of the velocity flow field
        PickColour(iSiteData.velocity * (float) mDomainStats->velocity_threshold_max_inv, lPalette);

        UpdateVelocityColour(iRayLengthInVoxel, lPalette);

        if (iVisSettings.mStressType != lb::ShearStress)
        {
          // update the volume rendering of the von Mises stress flow field
          float lScaledStress = iSiteData.stress * (float) mDomainStats->stress_threshold_max_inv;

          PickColour(lScaledStress, lPalette);

          UpdateStressColour(iRayLengthInVoxel, lPalette);
        }
      }

      void RayDataNormal::DoUpdateDataForWallSite(const SiteData_t& iSiteData,
                                                  const util::Vector3D<float>& iRayDirection,
                                                  const float iRayLengthInVoxel,
                                                  const VisSettings& iVisSettings,
                                                  const util::Vector3D<double>* iWallNormal)
      {
        DoUpdateDataForNormalFluidSite(iSiteData, iRayDirection, iRayLengthInVoxel, iVisSettings);
      }

      void RayDataNormal::DoGetVelocityColour(unsigned char oColour[3],
                                              const float iNormalisedDistanceToFirstCluster,
                                              const DomainStats& iDomainStats) const
      {
        MakeColourComponent(mVelR * 255.0F, oColour[0]);
        MakeColourComponent(mVelG * 255.0F, oColour[1]);
        MakeColourComponent(mVelB * 255.0F, oColour[2]);
      }

      void RayDataNormal::DoGetStressColour(unsigned char oColour[3],
                                            const float iNormalisedDistanceToFirstCluster,
                                            const DomainStats& iDomainStats) const
      {
        MakeColourComponent(mStressR, oColour[0]);
        MakeColourComponent(mStressG, oColour[1]);
        MakeColourComponent(mStressB, oColour[2]);
      }

      void RayDataNormal::MakeColourComponent(float value, unsigned char& colour) const
      {
        colour =
            util::NumericalFunctions::enforceBounds<unsigned char>((unsigned char) (value
                                                                       / GetCumulativeLengthInFluid()),
                                                                   0,
                                                                   255);
      }

      void RayDataNormal::DoCombine(const RayDataNormal& iOtherRayData)
      {
        mVelR += iOtherRayData.mVelR;
        mVelG += iOtherRayData.mVelG;
        mVelB += iOtherRayData.mVelB;

        mStressR += iOtherRayData.mStressR;
        mStressG += iOtherRayData.mStressG;
        mStressB += iOtherRayData.mStressB;
      }

      void RayDataNormal::UpdateVelocityColour(float iDt, const float iPalette[3])
      {
        mVelR += iDt * iPalette[0];
        mVelG += iDt * iPalette[1];
        mVelB += iDt * iPalette[2];
      }

      void RayDataNormal::UpdateStressColour(float iDt, const float iPalette[3])
      {
        mStressR += iDt * iPalette[0];
        mStressG += iDt * iPalette[1];
        mStressB += iDt * iPalette[2];
      }

      void RayDataNormal::DoProcessTangentingVessel()
      {
      }

      MPI_Datatype RayDataNormal::GetMPIType()
      {
        HEMELB_MPI_TYPE_BEGIN(type, RayDataNormal, 12);

        HEMELB_MPI_TYPE_ADD_MEMBER(i);
        HEMELB_MPI_TYPE_ADD_MEMBER(j);
        HEMELB_MPI_TYPE_ADD_MEMBER(mLengthBeforeRayFirstCluster);
        HEMELB_MPI_TYPE_ADD_MEMBER(mCumulativeLengthInFluid);
        HEMELB_MPI_TYPE_ADD_MEMBER(mDensityAtNearestPoint);
        HEMELB_MPI_TYPE_ADD_MEMBER(mStressAtNearestPoint);
        HEMELB_MPI_TYPE_ADD_MEMBER(mVelR);
        HEMELB_MPI_TYPE_ADD_MEMBER(mVelG);
        HEMELB_MPI_TYPE_ADD_MEMBER(mVelB);
        HEMELB_MPI_TYPE_ADD_MEMBER(mStressR);
        HEMELB_MPI_TYPE_ADD_MEMBER(mStressG);
        HEMELB_MPI_TYPE_ADD_MEMBER(mStressB);

        HEMELB_MPI_TYPE_END(type, RayDataNormal);
        return type;
      }

      const DomainStats* RayDataNormal::mDomainStats = nullptr;
    }
  }

  namespace net
  {
    template<>
    MPI_Datatype MpiDataTypeTraits<hemelb::vis::raytracer::RayDataNormal>::RegisterMpiDataType()
    {
      MPI_Datatype ret = vis::raytracer::RayDataNormal::GetMPIType();
      HEMELB_MPI_CALL(MPI_Type_commit, (&ret));
      return ret;
    }
  }
}

