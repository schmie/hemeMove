// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_VIS_GLYPHDRAWER_H
#define HEMELB_VIS_GLYPHDRAWER_H

#include "geometry/LatticeData.h"
#include "lb/MacroscopicPropertyCache.h"

#include "vis/BasicPixel.h"
#include "vis/PixelSet.h"
#include "vis/PixelSetStore.h"
#include "vis/Screen.h"
#include "vis/DomainStats.h"
#include "vis/Viewpoint.h"
#include "vis/VisSettings.h"

#include <vector>
#include <set>

namespace hemelb
{
  namespace vis
  {
    // Class for drawing glyphs.
    class GlyphDrawer : public PixelSetStore<PixelSet<BasicPixel> >
    {
      public:
        // Constructor and destructor
        GlyphDrawer(geometry::LatticeData* iLatDat, Screen* iScreen, DomainStats* iDomainStats,
                    Viewpoint* iViewpoint, VisSettings* iVisSettings);
        ~GlyphDrawer();

        // Function to perform the rendering.
        PixelSet<BasicPixel>* Render(const lb::MacroscopicPropertyCache& propertyCache);

      private:
        // A struct to represent a single glyph.
        struct Glyph
        {
            /**
             * The 3D coordinates of the glyph.
             */
            float x, y, z;
            /**
             * The local contiguous site id near there.
             */
            site_t siteId;
        };

        void RenderLine(const XYCoordinates<float>& endPoint1,
                        const XYCoordinates<float>& endPoint2, const VisSettings* visSettings,
                        PixelSet<BasicPixel>*);

        template<bool xLimited>
        void RenderLineHelper(int x, int y, int incE, int incNE, int limit,
                              const VisSettings* visSettings, PixelSet<BasicPixel>*);

        geometry::LatticeData* mLatDat;

        Screen* mScreen;
        DomainStats* mDomainStats;
        Viewpoint* mViewpoint;
        VisSettings* mVisSettings;

        std::vector<Glyph> mGlyphs;

    };

  }
}

#endif //HEMELB_VIS_GLYPHDRAWER_H
