// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_VIS_RENDERING_H
#define HEMELB_VIS_RENDERING_H

#include "vis/BasicPixel.h"
#include "vis/PixelSet.h"
#include "vis/rayTracer/RayDataNormal.h"
#include "vis/ResultPixel.h"
#include "vis/streaklineDrawer/StreakPixel.h"

namespace hemelb
{
  namespace vis
  {
    /**
     * Rendering: A class that acts as the interface between the visualisation controller and the
     * drawn renderings from each component drawer.
     */
    class Rendering
    {
      public:
        Rendering(PixelSet<BasicPixel>* glyph, PixelSet<raytracer::RayDataNormal>* ray,
                  PixelSet<streaklinedrawer::StreakPixel>* streak);
        void ReleaseAll();

        void ReceivePixelCounts(net::Net* inNet, proc_t source);

        void ReceivePixelData(net::Net* inNet, proc_t source);

        void SendPixelCounts(net::Net* inNet, proc_t destination);

        void SendPixelData(net::Net* inNet, proc_t destination);
        void Combine(const Rendering& other);
        void PopulateResultSet(PixelSet<ResultPixel>* resultSet);

      private:
        template<typename pixelType>
        void AddPixelsToResultSet(PixelSet<ResultPixel>* resultSet,
                                  const std::vector<pixelType>& inPixels)
        {
          for (typename std::vector<pixelType>::const_iterator it = inPixels.begin();
              it != inPixels.end(); it++)
          {
            resultSet->AddPixel(ResultPixel(&*it));
          }
        }

        PixelSet<BasicPixel>* glyphResult;
        PixelSet<raytracer::RayDataNormal>* rayResult;
        PixelSet<streaklinedrawer::StreakPixel>* streakResult;
    };
  }
}

#endif /* HEMELB_VIS_RENDERING_H */
