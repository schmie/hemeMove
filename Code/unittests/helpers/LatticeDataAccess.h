// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_UNITTESTS_HELPERS_LATTICEDATAACCESS_H
#define HEMELB_UNITTESTS_HELPERS_LATTICEDATAACCESS_H

#include <algorithm>
#include <functional>
#include "geometry/LatticeData.h"

namespace hemelb
{
  namespace unittests
  {
    namespace helpers
    {

      // Empty FOld distribution
      void zeroOutFOld(geometry::LatticeData * const latDat);
      // Initializes a lattice to empty distributions and forces, execpt at one
      // place
      template<class LATTICE>
      void allZeroButOne(geometry::LatticeData *latDat, LatticeVector const &_pos);

      // FNew at given site
      template<class Lattice>
      distribn_t const * GetFNew(geometry::LatticeData *latDat, LatticeVector const &_pos);

      // Population i set to some distribution
      template<class LATTICE>
      void setUpDistribution(geometry::LatticeData *latDat, size_t _i,
                             std::function<Dimensionless(PhysicalVelocity const &)> _function);

      // Class to setup/manipulate lattice data
      class LatticeDataAccess
      {
        public:
          LatticeDataAccess(geometry::LatticeData * const latDat) :
              latDat(latDat)
          {
          }
          ;

          // Empty distribution
          void ZeroOutFOld() const;
          // Empty Forces
          void ZeroOutForces() const;
          // Set FOld for a given site and direction
          template<class LATTICE>
          void SetFOld(site_t _x, site_t _y, site_t _z, site_t _dir, distribn_t _value) const
          {
            SetFOld<LATTICE>(LatticeVector(_x, _y, _z), _dir, _value);
          }
          template<class LATTICE>
          void SetFOld(LatticeVector const &_pos, site_t _dir, distribn_t _value) const;

          // Get FNew for a given site and direction
          template<class LATTICE>
          distribn_t const * GetFNew(site_t _x, site_t _y, site_t _z) const
          {
            return GetFNew<LATTICE>(LatticeVector(_x, _y, _z));
          }
          template<class LATTICE>
          distribn_t const * GetFNew(LatticeVector const &_pos) const;
          distribn_t const * GetFNew(site_t index) const
          {
            return latDat->GetFNew(index);
          }


          void SetMinWallDistance(PhysicalDistance _mindist);
          void SetWallDistance(PhysicalDistance _mindist);

          // Sets up linear distribution f_i(coeffs) = (1, x, y, z) * coeffs
          // Where * is the dot product
          template<class LATTICE>
          void SetUpDistribution(size_t _i,
                                 std::function<Dimensionless(PhysicalVelocity const &)> _function);

        protected:
          // Reference to the wrapped lattice data.
          geometry::LatticeData * const latDat;
      };

      inline void LatticeDataAccess::ZeroOutFOld() const
      {
        std::fill(latDat->oldDistributions.begin(), latDat->oldDistributions.end(), 0);
      }
      inline void LatticeDataAccess::ZeroOutForces() const
      {
        std::fill(latDat->forceAtSite.begin(), latDat->forceAtSite.end(), LatticeForceVector::Zero());
      }

      template<class LATTICE>
      void LatticeDataAccess::SetFOld(LatticeVector const &_pos, site_t _dir,
                                      distribn_t _value) const
      {
        // Figure location of distribution in memory using const access to FOld.
        // This way, access is resilient versus (some) changes in memory layout.
        geometry::Site<geometry::LatticeData> const site(latDat->GetSite(_pos));
        distribn_t const * const siteFOld(site.GetFOld<LATTICE>());
        distribn_t const * const firstFOld = &latDat->oldDistributions[0];
        size_t const indexFOld(siteFOld - firstFOld);
        latDat->oldDistributions[indexFOld + _dir] = _value;
      }

      template<class LATTICE>
      distribn_t const *
      LatticeDataAccess::GetFNew(LatticeVector const &_pos) const
      {
        // Figure location of distribution in memory using const access to FOld.
        // This way, access is resilient versus (some) changes in memory layout.
        geometry::Site<geometry::LatticeData> const site(latDat->GetSite(_pos));
        distribn_t const * const siteFOld(site.GetFOld<LATTICE>());
        distribn_t const * const firstFOld = &latDat->oldDistributions[0];
        size_t const indexFOld(siteFOld - firstFOld);
        return latDat->GetFNew(indexFOld);
      }

      void LatticeDataAccess::SetMinWallDistance(PhysicalDistance _mindist)
      {
        typedef std::vector<distribn_t>::iterator iterator;
        iterator i_first = latDat->distanceToWall.begin();
        iterator const i_end = latDat->distanceToWall.end();
        for (; i_first != i_end; ++i_first)
          if (*i_first > 0e0 and *i_first < _mindist)
            *i_first = _mindist;
      }

      void LatticeDataAccess::SetWallDistance(PhysicalDistance _mindist)
      {
        typedef std::vector<distribn_t>::iterator iterator;
        iterator i_first = latDat->distanceToWall.begin();
        iterator const i_end = latDat->distanceToWall.end();
        for (; i_first != i_end; ++i_first)
          if (*i_first > 0e0)
            *i_first = _mindist;
      }

      inline void ZeroOutFOld(geometry::LatticeData * const latDat)
      {
        LatticeDataAccess(latDat).ZeroOutFOld();
      }
      inline void ZeroOutForces(geometry::LatticeData * const latDat)
      {
        LatticeDataAccess(latDat).ZeroOutForces();
      }
      inline void ZeroOutForces(geometry::LatticeData & latDat)
      {
        return ZeroOutForces(&latDat);
      }

      template<class LATTICE>
      void LatticeDataAccess::SetUpDistribution(
          size_t _i, std::function<Dimensionless(PhysicalVelocity const &)> _function)
      {
        for (site_t i(0); i < latDat->GetLocalFluidSiteCount(); ++i)
        {
          geometry::Site<geometry::LatticeData> site = latDat->GetSite(i);
          LatticeVector const pos = site.GetGlobalSiteCoords();
          LatticePosition const pos_real(pos[0], pos[1], pos[2]);
          distribn_t const * const siteFOld(site.GetFOld<LATTICE>());
          distribn_t const * const firstFOld = &latDat->oldDistributions[0];
          size_t const indexFOld(siteFOld - firstFOld);
          latDat->newDistributions[indexFOld + _i] = latDat->oldDistributions[indexFOld + _i] = _function(pos_real);
        }
      }

      // Population i set to some distribution
      template<class LATTICE>
      void setUpDistribution(geometry::LatticeData *latDat, size_t _i,
                             std::function<Dimensionless(PhysicalVelocity const &)> _function)
      {
        LatticeDataAccess(latDat).SetUpDistribution<LATTICE>(_i, _function);
      }

      template<class LATTICE>
      void allZeroButOne(geometry::LatticeData *latDat, LatticeVector const &_pos)
      {
        LatticeDataAccess manip(latDat);
        manip.ZeroOutFOld();
        manip.ZeroOutForces();
        for (size_t i(0); i < LATTICE::NUMVECTORS; ++i)
          manip.SetFOld<LATTICE>(_pos, i, 0.5 + i);
        latDat->GetSite(_pos).SetForce(LatticeForceVector(1, 2, 3));
      }

      template<class LATTICE>
      distribn_t const * GetFNew(geometry::LatticeData *latDat, LatticeVector const &_pos)
      {
        return LatticeDataAccess(latDat).GetFNew<LATTICE>(_pos);
      }
      distribn_t const * GetFNew(geometry::LatticeData *latDat, site_t const &index)
      {
        return LatticeDataAccess(latDat).GetFNew(index);
      }

      void SetMinWallDistance(geometry::LatticeData * const latDat, PhysicalDistance _mindist)
      {
        LatticeDataAccess(latDat).SetMinWallDistance(_mindist);
      }
      void SetWallDistance(geometry::LatticeData * const latDat, PhysicalDistance _mindist)
      {
        LatticeDataAccess(latDat).SetWallDistance(_mindist);
      }

      template<class LATTICE = lb::lattices::D3Q15>
      std::tuple<Dimensionless, PhysicalVelocity,
          std::function<Dimensionless(PhysicalVelocity const &)>,
          std::function<Dimensionless(PhysicalVelocity const &)> > makeLinearProfile(
          size_t _cubeSize, geometry::LatticeData * const latDat, PhysicalVelocity const &_grad)
      {

        PhysicalVelocity const gradient = _grad.GetNormalised();
        /* any number big enough to avoid negative populations */
        Dimensionless const non_neg_pop(_cubeSize * 3);
        auto linear = [non_neg_pop, gradient](PhysicalVelocity const &_v)
        {
          return non_neg_pop + _v.Dot(gradient);
        };
        auto linear_inv = [non_neg_pop, gradient](PhysicalVelocity const &_v)
        {
          return 2.0 * non_neg_pop - _v.Dot(gradient);
        };
        setUpDistribution<LATTICE>(latDat, 0, linear);
        setUpDistribution<LATTICE>(latDat, 1, linear_inv);

        return std::make_tuple(non_neg_pop, gradient, linear, linear_inv);
      }

    }
  }
}
#endif
