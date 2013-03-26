#ifndef HEMELB_LB_STREAMERS_NASHZEROTHORDERPRESSUREIOLET_H
#define HEMELB_LB_STREAMERS_NASHZEROTHORDERPRESSUREIOLET_H

#include "lb/streamers/StreamerTypeFactory.h"
#include "lb/streamers/NashZerothOrderPressureDelegate.h"
#include "lb/streamers/SimpleBounceBackDelegate.h"
#include "lb/streamers/BouzidiFirdaousLallemandDelegate.h"

namespace hemelb
{
  namespace lb
  {
    namespace streamers
    {

      template<class CollisionType>
      struct NashZerothOrderPressureIolet
      {
          typedef IoletStreamerTypeFactory<CollisionType, NashZerothOrderPressureDelegate<CollisionType> > Type;
      };

      template<class CollisionType>
      struct NashZerothOrderPressureIoletSBB
      {
          typedef WallIoletStreamerTypeFactory<CollisionType, SimpleBounceBackDelegate<CollisionType> ,
              NashZerothOrderPressureDelegate<CollisionType> > Type;
      };

      template<class CollisionType>
      struct NashZerothOrderPressureIoletBFL
      {
          typedef WallIoletStreamerTypeFactory<CollisionType, BouzidiFirdaousLallemandDelegate<CollisionType> ,
              NashZerothOrderPressureDelegate<CollisionType> > Type;
      };

      template<class CollisionType>
      struct NashZerothOrderPressureIoletGZS
      {
          typedef WallIoletStreamerTypeFactory<CollisionType, GuoZhengShiDelegate<CollisionType> ,
              NashZerothOrderPressureDelegate<CollisionType> > Type;
      };
    }
  }
}

#endif /* HEMELB_LB_STREAMERS_NASHZEROTHORDERPRESSUREIOLET_H */
