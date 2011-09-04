#ifndef HEMELB_LB_RHEOLOGYMODELS_TRUNCATEDPOWERLAWRHEOLOGYMODEL_H_
#define HEMELB_LB_RHEOLOGYMODELS_TRUNCATEDPOWERLAWRHEOLOGYMODEL_H_

#include "lb/rheology_models/AbstractRheologyModel.h"

namespace hemelb
{
  namespace lb
  {
    namespace rheology_models
    {
      /*
       * GAMMA_{ZERO,INF} are computed with the power-law formula: nu = m * gamma^{n-1} for
       * given values of nu_{max,min}, m, and n.
       */
      static const double GAMMA_ZERO = 6.25e-4; // TODO units, gamma=6.25e-4 for m=BLOOD_VISCOSITY_Pa_s / BLOOD_DENSITY_Kg_per_m3 n=0.5 and nu_max=0.16 Pa_s / BLOOD_DENSITY_Kg_per_m3
      static const double GAMMA_INF = 1.3061; // TODO units, gamma=1 for m=BLOOD_VISCOSITY_Pa_s / BLOOD_DENSITY_Kg_per_m3 n=0.5 and nu_min=0.0035 Pa_s / BLOOD_DENSITY_Kg_per_m3

      static const double M_CONSTANT = BLOOD_VISCOSITY_Pa_s / BLOOD_DENSITY_Kg_per_m3; // m^2/s
      static const double N_CONSTANT = 0.5; // dimensionless, n<1 shear thinning, n>1 shear thickening, n=1 newtonian with nu=M_CONSTANT

      class TruncatedPowerLawRheologyModel : public AbstractRheologyModel<TruncatedPowerLawRheologyModel>
      {
        public:
          /*
           *  Compute nu for a given shear rate according to the truncated power-law:
           *
           *                     { M_CONSTANT * GAMMA_ZERO^{N_CONSTANT - 1},  if iShearRate<GAMMA_ZERO
           *  nu = eta/density = { M_CONSTANT *  GAMMA_INF^{N_CONSTANT - 1},  if iShearRate>GAMMA_INF
           *                     { M_CONSTANT * iShearRate^{N_CONSTANT - 1},  otherwise
           *
           *  @param iShearRate local shear rate value (s^{-1}).
           *  @param iDensity local density. TODO at the moment this value is not used
           *         in any subclass.
           *
           *  @return kinematic viscosity (m^2/s).
           */
          static double CalculateViscosityForShearRate(const double &iShearRate,
                                                       const distribn_t &iDensity);
      };
    }
  }
}

#endif /* HEMELB_LB_RHEOLOGYMODELS_TRUNCATEDPOWERLAWRHEOLOGYMODEL_H_ */
