// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include <iostream>
#include <memory>
#include <sstream>

#include <catch2/catch.hpp>

#include "lb/kernels/Kernels.h"
#include "lb/streamers/Streamers.h"
#include "lb/iolets/InOutLets.h"
#include "geometry/SiteData.h"
#include "util/utilityFunctions.h"

#include "tests/helpers/ApproxVector.h"
#include "tests/helpers/FourCubeBasedTestFixture.h"

namespace hemelb
{
  namespace tests
  {
    constexpr distribn_t allowedError = 1e-10;
    static Approx apprx(double x) {
      return Approx(x).margin(allowedError);
    }

    /**
     * Helper class that exposes implementation details to the tests.
     */
    using lb::streamers::VSExtra;
    class LocalVSExtra : public VSExtra<lb::lattices::D3Q15>
    {
    public:
      //typedef hemelb::lb::streamers::VSExtra::UnitVec UnitVec;
      LocalVSExtra(lb::iolets::InOutLet& iolet) :
	VSExtra<lb::lattices::D3Q15> (iolet)
      {
      }
      const UnitVec& GetE1() const
      {
	return e1;
      }
      const UnitVec& GetE2() const
      {
	return e2;
      }
      const UnitVec& GetE3() const
      {
	return n;
      }
    };

    static LatticeVelocity GetVelocity(LatticeVector pos)
    {
      LatticeVelocity c(0.01, 0.01, 0.);
      LatticeVelocity u(0.);
      u.z = c.Dot(pos);
      return u;
    }
    static LatticeDensity GetDensity(LatticeVector pos)
    {
      util::Vector3D<LatticeDensity> gradRho(0., 0., -1e-2);
      return 1.045 + gradRho.Dot(pos);
    }

    using lb::iolets::InOutLetCosine;
    static InOutLetCosine* GetIolet(lb::iolets::BoundaryValues& iolets)
    {
      InOutLetCosine* ans =
	dynamic_cast<lb::iolets::InOutLetCosine*> (iolets.GetLocalIolet(0));
      REQUIRE(ans != nullptr);
      return ans;
    }

    /**
     * VirtualSiteIoletStreamerTests:
     *
     * This class tests the streamer implementations. We assume the
     * collision operators are correct (as they're tested elsewhere),
     * then compare the post-streamed values with the values we expect
     * to have been streamed there.
     */
    TEST_CASE_METHOD(helpers::FourCubeBasedTestFixture, "VirtualSiteIoletStreamerTests") {

      typedef lb::lattices::D3Q15 Lattice;
      typedef lb::kernels::LBGK<Lattice> Kernel;
      typedef lb::collisions::Normal<Kernel> Collision;
      typedef lb::streamers::VirtualSite<Lattice> VirtualSite;
      typedef lb::iolets::InOutLetCosine InOutLetCosine;
      typedef lb::streamers::RSHV RSHV;

      auto Info = [&]() -> const lb::lattices::LatticeInfo& {
	return Lattice::GetLatticeInfo();
      };
	  
      auto CheckAllHVUpdated = [&](lb::iolets::BoundaryValues& iolets, LatticeTimeStep expectedT) {
	VSExtra<Lattice> * extra =
	dynamic_cast<VSExtra<Lattice>*> (iolets.GetLocalIolet(0)->GetExtraData());
	for (RSHV::Map::iterator hvPtr = extra->hydroVarsCache.begin();
	     hvPtr != extra->hydroVarsCache.end(); ++hvPtr) {
	  site_t siteGlobalIdx = hvPtr->first;
	  LatticeVector sitePos;
	  latDat->GetGlobalCoordsFromGlobalNoncontiguousSiteId(siteGlobalIdx, sitePos);
	  RSHV& hv = hvPtr->second;
	  REQUIRE(expectedT == hv.t);
	  REQUIRE(apprx(GetDensity(sitePos)) == hv.rho);
	  LatticeVelocity u = GetVelocity(sitePos);
	  for (unsigned i = 0; i < 3; ++i)
	    REQUIRE(apprx(u[i]) == hv.u[i]);
	}
      };

      auto CheckPointInCache = [&](LocalVSExtra& extra, LatticeVector expectedPt,
				   LatticePosition expectedIoletPos)
      {
	site_t expectedGlobalIdx =
	  latDat->GetGlobalNoncontiguousSiteIdFromGlobalCoords(expectedPt);
	RSHV::Map::iterator hvPtr = extra.hydroVarsCache.find(expectedGlobalIdx);
	
	REQUIRE(hvPtr != extra.hydroVarsCache.end());
	for (unsigned i = 0; i < 3; ++i)
	  REQUIRE(apprx(expectedIoletPos[i]) == hvPtr->second.posIolet[i]);
      };

      auto InitialiseGradientHydroVars = [&](){
	for (unsigned i = 1; i <= 4; ++i) {
	  for (unsigned j = 1; j <= 4; ++j) {
	    for (unsigned k = 1; k <= 4; ++k) {
	      LatticeVector pos(i, j, k);
	      site_t siteIdx = latDat->GetContiguousSiteId(pos);
	      //geometry::Site < geometry::LatticeData > site = latDat->GetSite(siteIdx);
	      distribn_t* fOld = latDat->GetFNew(siteIdx * Lattice::NUMVECTORS);
	      LatticeDensity rho = GetDensity(pos);
	      LatticeVelocity u = GetVelocity(pos);
	      u *= rho;
	      Lattice::CalculateFeq(rho, u.x, u.y, u.z, fOld);
	    }
	  }
	}
	latDat->SwapOldAndNew();
      };

      auto propertyCache = lb::MacroscopicPropertyCache(*simState, *latDat);

      auto inletBoundary = lb::iolets::BoundaryValues(geometry::INLET_TYPE,
						      latDat,
						      simConfig->GetInlets(),
						      simState.get(),
						      Comms(),
						      *unitConverter);
      InOutLetCosine* inlet = GetIolet(inletBoundary);
      // We have to make the outlet sane and consistent with the geometry now.
      inlet->SetNormal(util::Vector3D<Dimensionless>(0, 0, 1));
      PhysicalPosition inletCentre(2.5, 2.5, 0.5);
      inletCentre *= simConfig->GetVoxelSize();
      inlet->SetPosition(unitConverter->ConvertPositionToLatticeUnits(inletCentre));
      // Want to set the density gradient to be 0.01 in lattice units,
      // starting at 1.0 at the outlet.
      inlet->SetPressureAmp(0.);
      inlet->SetPressureMean(unitConverter->ConvertPressureToPhysicalUnits(1.04 * Cs2));

      // Same for the cut distances of outlet sites.
      for (unsigned i = 1; i <= 4; ++i) {
	for (unsigned j = 1; j <= 4; ++j) {
	  site_t siteId = latDat->GetContiguousSiteId(LatticeVector(i, j, 1));
	  geometry::Site < geometry::LatticeData > site = latDat->GetSite(siteId);
	  for (Direction p = 0; p < Info().GetNumVectors(); ++p) {
	    if (Info().GetVector(p).z < 0.) {
	      // Sanity check
	      REQUIRE(site.HasIolet(p));
	      // Set the cut distance to half way
	      latDat->SetBoundaryDistance(siteId, p, 0.5);
	    }
	  }
	}
      }

      auto outletBoundary = lb::iolets::BoundaryValues(geometry::OUTLET_TYPE,
						       latDat,
						       simConfig->GetOutlets(),
						       simState.get(),
						       Comms(),
						       *unitConverter);

      InOutLetCosine* outlet = GetIolet(outletBoundary);
      // We have to make the outlet sane and consistent with the geometry now.
      outlet->SetNormal(util::Vector3D<Dimensionless>(0, 0, -1));
      PhysicalPosition outletCentre(2.5, 2.5, 4.5);
      outletCentre *= simConfig->GetVoxelSize();
      outlet->SetPosition(unitConverter->ConvertPositionToLatticeUnits(outletCentre));
      outlet->SetPressureAmp(0.);
      outlet->SetPressureMean(unitConverter->ConvertPressureToPhysicalUnits(1.0 * Cs2));

      // Same for the cut distances of outlet sites.
      for (unsigned i = 1; i <= 4; ++i) {
	for (unsigned j = 1; j <= 4; ++j) {
	  site_t siteId = latDat->GetContiguousSiteId(LatticeVector(i, j, 4));
	  geometry::Site < geometry::LatticeData > site = latDat->GetSite(siteId);
	  for (Direction p = 0; p < Info().GetNumVectors(); ++p) {
	    if (Info().GetVector(p).z > 0.) {
	      // Sanity check
	      REQUIRE(site.HasIolet(p));
	      // Set the cut distance to half way
	      latDat->SetBoundaryDistance(siteId, p, 0.5);
	    }
	  }
	}
      }
    
      SECTION("TestVirtualSiteMatrixInverse") {
	distribn_t inv[3][3];
	distribn_t det;

	// Try the identity matrix first
	distribn_t eye[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
	det = VirtualSite::Matrix3DInverse(eye, inv);
	REQUIRE(apprx(1.) == det);
	for (unsigned i = 0; i < 3; ++i)
	  for (unsigned j = 0; j < 3; ++j)
	    REQUIRE(apprx(eye[i][j]) == inv[i][j]);

	// arbitrary matrix with nice ish values
	distribn_t a[3][3] = { { 1, 2, 3 }, { 0, 4, 5 }, { 1, 0, 6 } };
	distribn_t aInv22[3][3] = { { 24, -12, -2 }, { 5, 3, -5 }, { -4, 2, 4 } };

	det = VirtualSite::Matrix3DInverse(a, inv);
	REQUIRE(apprx(22.) == det);
	for (unsigned i = 0; i < 3; ++i)
	  for (unsigned j = 0; j < 3; ++j)
	    REQUIRE(apprx(aInv22[i][j] / 22) == inv[i][j]);

      }

      SECTION("TestVirtualSiteConstruction") {
	InOutLetCosine& outlet = *GetIolet(outletBoundary);
	LocalVSExtra extra(outlet);
	// Expected basis for iolet coords
	util::Vector3D<Dimensionless> e1(-1, 0, 0), e2(0, 1, 0), e3(0, 0, -1);
	REQUIRE(ApproxV(e1).Margin(allowedError) == extra.GetE1());
	REQUIRE(ApproxV(e2).Margin(allowedError) == extra.GetE2());
	REQUIRE(ApproxV(e3).Margin(allowedError) == extra.GetE3());

	// This point should be just outside the 4 cube's outlet
	LatticeVector vLoc(2, 2, 5);
	// Create it
	VirtualSite vSite(initParams, extra, vLoc);

	// The iolet coords
	REQUIRE(ApproxVector<LatticePosition>(0.5, -0.5, -0.5).Margin(allowedError) == vSite.hv.posIolet);

	// For D3Q15, there should be 5 cut links.
	REQUIRE(size_t(5) == vSite.neighbourGlobalIds.size());
	// each with q = 0.5
	REQUIRE(apprx(1.25) == vSite.sumQiSq);
	// For each site index in the neighbourhood
	for (Direction p = 0; p < Info().GetNumVectors(); ++p) {
	  //              if (Info().GetVector(p).z < 0.)
	  //              {
	  //                CPPUNIT_ASSERT(vSite.streamingIndices[p] < 4 * 4 * 4 * 15);
	  //              }
	  //              else
	  //              {
	  //                CPPUNIT_ASSERT_EQUAL(site_t(4 * 4 * 4 * 15), vSite.streamingIndices[p]);
	  //              }
	}

	// The velocity matrix = sum over i of:
	// x_i^2   x_i y_i x_i
	// x_i y_i y_i^2   y_i
	// x_i     y_i     1
	// distribn_t velMat[3][3] = { { 5.25, -1.25, 2.5 },
	//                             { -1.25, 5.25, -2.5 },
	//                             { 2.5, -2.5,  5.0 } };
	// It's inverse is
	distribn_t velMatInv[3][3] = { { 0.25, 0., -0.125 },
				       { 0., 0.25, 0.125 },
				       { -0.125, 0.125, 0.325 } };
	for (unsigned i = 0; i < 3; ++i) {
	  for (unsigned j = 0; j < 3; ++j) {
	    REQUIRE(apprx(velMatInv[i][j]) == vSite.velocityMatrixInv[i][j]);
	  }
	}

	// Now check that appropriate entries have been added to the hydroCache
	CheckPointInCache(extra, LatticeVector(1, 1, 4), LatticePosition(1.5, -1.5, 0.5));
	CheckPointInCache(extra, LatticeVector(1, 3, 4), LatticePosition(1.5, 0.5, 0.5));
	CheckPointInCache(extra, LatticeVector(3, 1, 4), LatticePosition(-0.5, -1.5, 0.5));
	CheckPointInCache(extra, LatticeVector(3, 3, 4), LatticePosition(-0.5, 0.5, 0.5));
	CheckPointInCache(extra, LatticeVector(2, 2, 4), LatticePosition(0.5, -0.5, 0.5));
      }

      SECTION("TestStreamerInitialisation") {
	initParams.boundaryObject = &outletBoundary;
	// Set up the ranges to cover Mid 3 (pure outlet) and Mid 5 (outlet/wall)
	initParams.siteRanges.resize(2);
	initParams.siteRanges[0].first = latDat->GetMidDomainCollisionCount(0)
	  + latDat->GetMidDomainCollisionCount(1) + latDat->GetMidDomainCollisionCount(2);
	initParams.siteRanges[0].second = initParams.siteRanges[0].first
	  + latDat->GetMidDomainCollisionCount(3);
	initParams.siteRanges[1].first = initParams.siteRanges[0].second
	  + latDat->GetMidDomainCollisionCount(4);
	initParams.siteRanges[1].second = initParams.siteRanges[1].first
	  + latDat->GetMidDomainCollisionCount(5);
	lb::streamers::VirtualSiteIolet<Collision> outletStreamer(initParams);

	// All the sites at the outlet plane (x, y, 3) should be in the cache.
	InOutLetCosine& outlet = *GetIolet(outletBoundary);
	VSExtra<Lattice>* extra = dynamic_cast<VSExtra<Lattice>*> (outlet.GetExtraData());
	REQUIRE(extra != nullptr);

	for (unsigned i = 1; i <= 4; ++i) {
	  for (unsigned j = 1; j <= 4; ++j) {
	    LatticeVector pos(i, j, 4);
	    site_t globalIdx = latDat->GetGlobalNoncontiguousSiteIdFromGlobalCoords(pos);
	    //                site_t localIdx = latDat->GetLocalContiguousIdFromGlobalNoncontiguousId(globalIdx);
	    //                geometry::Site < geometry::LatticeData > site = latDat->GetSite(localIdx);

	    RSHV::Map::iterator hvPtr = extra->hydroVarsCache.find(globalIdx);
	    REQUIRE(hvPtr != extra->hydroVarsCache.end());
	  }
	}

	// And the reverse is true: every cache entry should be a site at the outlet plane
	for (RSHV::Map::iterator hvPtr = extra->hydroVarsCache.begin();
	     hvPtr != extra->hydroVarsCache.end(); ++hvPtr) {
	  site_t globalIdx = hvPtr->first;
	  LatticeVector pos;
	  latDat->GetGlobalCoordsFromGlobalNoncontiguousSiteId(globalIdx, pos);
	  REQUIRE(hemelb::util::NumericalFunctions::IsInRange<LatticeCoordinate>(pos.x,
										 1,
										 4));
	  REQUIRE(hemelb::util::NumericalFunctions::IsInRange<LatticeCoordinate>(pos.y,
										 1,
										 4));
	  REQUIRE(LatticeCoordinate(4) == pos.z);
	}
      }

      SECTION("TestStep") {
	initParams.boundaryObject = &inletBoundary;
	lb::streamers::VirtualSiteIolet<Collision> inletStreamer(initParams);
	initParams.boundaryObject = &outletBoundary;
	lb::streamers::VirtualSiteIolet<Collision> outletStreamer(initParams);

	InitialiseGradientHydroVars();

	// Stream and collide
	site_t offset = 0;
	offset += latDat->GetMidDomainCollisionCount(0);
	offset += latDat->GetMidDomainCollisionCount(1);
	inletStreamer.DoStreamAndCollide<false> (offset,
						 latDat->GetMidDomainCollisionCount(2),
						 lbmParams,
						 static_cast<geometry::LatticeData*> (latDat),
						 propertyCache);
	offset += latDat->GetMidDomainCollisionCount(2);

	outletStreamer.StreamAndCollide<false> (offset,
						latDat->GetMidDomainCollisionCount(3),
						lbmParams,
						latDat,
						propertyCache);
	offset += latDat->GetMidDomainCollisionCount(3);

	inletStreamer.StreamAndCollide<false> (offset,
					       latDat->GetMidDomainCollisionCount(4),
					       lbmParams,
					       latDat,
					       propertyCache);
	offset += latDat->GetMidDomainCollisionCount(4);

	outletStreamer.StreamAndCollide<false> (offset,
						latDat->GetMidDomainCollisionCount(5),
						lbmParams,
						latDat,
						propertyCache);

	// Now every entry in the RSHV cache should have been updated
	CheckAllHVUpdated(inletBoundary, 1);
	CheckAllHVUpdated(outletBoundary, 1);

	// Stream and collide
	offset = 0;
	offset += latDat->GetMidDomainCollisionCount(0);
	offset += latDat->GetMidDomainCollisionCount(1);
	inletStreamer.DoPostStep<false> (offset,
					 latDat->GetMidDomainCollisionCount(2),
					 lbmParams,
					 static_cast<geometry::LatticeData*> (latDat),
					 propertyCache);
	offset += latDat->GetMidDomainCollisionCount(2);

	outletStreamer.DoPostStep<false> (offset,
					  latDat->GetMidDomainCollisionCount(3),
					  lbmParams,
					  latDat,
					  propertyCache);
	offset += latDat->GetMidDomainCollisionCount(3);

	inletStreamer.DoPostStep<false> (offset,
					 latDat->GetMidDomainCollisionCount(4),
					 lbmParams,
					 latDat,
					 propertyCache);
	offset += latDat->GetMidDomainCollisionCount(4);

	outletStreamer.DoPostStep<false> (offset,
					  latDat->GetMidDomainCollisionCount(5),
					  lbmParams,
					  latDat,
					  propertyCache);

	// Check that all the vsites have sensible hydro values
	InOutLetCosine* inlet = GetIolet(inletBoundary);
	VSExtra<Lattice> * inExtra = dynamic_cast<VSExtra<Lattice>*> (inlet->GetExtraData());

	for (VirtualSite::Map::iterator vsIt = inExtra->vSites.begin();
	     vsIt != inExtra->vSites.end(); ++vsIt) {
	  site_t vSiteGlobalIdx = vsIt->first;
	  VirtualSite& vSite = vsIt->second;

	  REQUIRE(LatticeTimeStep(1) == vSite.hv.t);
	  REQUIRE(apprx(LatticeDensity(1.045)) == vSite.hv.rho);

	  LatticeVector pos;
	  latDat->GetGlobalCoordsFromGlobalNoncontiguousSiteId(vSiteGlobalIdx, pos);

	  if (vSite.neighbourGlobalIds.size() > 3)
	    REQUIRE(apprx(GetVelocity(pos).z) ==  vSite.hv.u.z);
	}

	InOutLetCosine* outlet = GetIolet(outletBoundary);
	VSExtra<Lattice> * outExtra = dynamic_cast<VSExtra<Lattice>*> (outlet->GetExtraData());

	for (VirtualSite::Map::iterator vsIt = outExtra->vSites.begin();
	     vsIt != outExtra->vSites.end(); ++vsIt) {
	  site_t vSiteGlobalIdx = vsIt->first;
	  VirtualSite& vSite = vsIt->second;

	  REQUIRE(LatticeTimeStep(1) == vSite.hv.t);
	  REQUIRE(apprx(LatticeDensity(0.995)) == vSite.hv.rho);

	  LatticeVector pos;
	  latDat->GetGlobalCoordsFromGlobalNoncontiguousSiteId(vSiteGlobalIdx, pos);

	  if (vSite.neighbourGlobalIds.size() > 3)
	    REQUIRE(apprx(GetVelocity(pos).z) == vSite.hv.u.z);

	}
      }

    }

  }
}

