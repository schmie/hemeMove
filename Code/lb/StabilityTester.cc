#include "lb/StabilityTester.h"
#include "lb/LbmParameters.h"

namespace hemelb
{
  namespace lb
  {

    StabilityTester::StabilityTester(const geometry::LatticeData * iLatDat,
                                     net::Net* net,
                                     SimulationState* simState) :
      net::PhasedBroadcastRegular<>(net, simState, SPREADFACTOR), mLatDat(iLatDat),
          mSimState(simState)
    {
      Reset();
    }

    void StabilityTester::Reset()
    {
      // Re-initialise all values to be Stable.
      mUpwardsStability = Stable;
      mDownwardsStability = Stable;

      mSimState->SetStability(Stable);

      for (unsigned int ii = 0; ii < SPREADFACTOR; ii++)
      {
        mChildrensStability[ii] = Stable;
      }
    }

    void StabilityTester::PostReceiveFromChildren(unsigned long splayNumber)
    {
      // No need to test children's stability if this node is already unstable.
      if (mUpwardsStability == Stable)
      {
        for (int ii = 0; ii < (int) SPREADFACTOR; ii++)
        {
          if (mChildrensStability[ii] == Unstable)
          {
            mUpwardsStability = Unstable;
            break;
          }
        }
      }
    }

    void StabilityTester::ProgressFromChildren(unsigned long splayNumber)
    {
      ReceiveFromChildren<int> (mChildrensStability, 1);
    }

    void StabilityTester::ProgressFromParent(unsigned long splayNumber)
    {
      ReceiveFromParent<int> (&mDownwardsStability, 1);
    }

    void StabilityTester::ProgressToChildren(unsigned long splayNumber)
    {
      SendToChildren<int> (&mDownwardsStability, 1);
    }

    void StabilityTester::ProgressToParent(unsigned long splayNumber)
    {
      // No need to bother testing out local lattice points if we're going to be
      // sending up a 'Unstable' value anyway.
      if (mUpwardsStability != Unstable)
      {
        for (site_t i = 0; i < mLatDat->GetLocalFluidSiteCount(); i++)
        {
          for (unsigned int l = 0; l < D3Q15::NUMVECTORS; l++)
          {
            distribn_t value = *mLatDat->GetFNew(i * D3Q15::NUMVECTORS + l);

            if (value < 0.0 || value != value)
            {
              mUpwardsStability = Unstable;
            }
          }
        }
      }

      SendToParent<int> (&mUpwardsStability, 1);
    }

    void StabilityTester::TopNodeAction()
    {
      mDownwardsStability = mUpwardsStability;
    }

    void StabilityTester::Effect()
    {
      mSimState->SetStability((Stability) mDownwardsStability);
    }

  }
}
