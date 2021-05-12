// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include <string>

#include "steering/common/Tags.h"
#include "steering/common/Steerer.h"
namespace hemelb
{
  namespace steering
  {
    HEMELB_STEERING_INITIALISE_TAG(ctr_x, 0.);HEMELB_STEERING_INITIALISE_TAG(ctr_y, 0.);HEMELB_STEERING_INITIALISE_TAG(ctr_z, 0.);

HEMELB_STEERING_INITIALISE_TAG    (longitude, 45.);HEMELB_STEERING_INITIALISE_TAG(latitude, 45.);

HEMELB_STEERING_INITIALISE_TAG    (zoom, 1.);HEMELB_STEERING_INITIALISE_TAG(brightness, 0.03);

HEMELB_STEERING_INITIALISE_TAG    (velocity_max, 0.1);HEMELB_STEERING_INITIALISE_TAG(stress_max, 0.1);

HEMELB_STEERING_INITIALISE_TAG    (pressure_min, 80.);HEMELB_STEERING_INITIALISE_TAG(pressure_max, 120.);

HEMELB_STEERING_INITIALISE_TAG    (glyph_length, 1.);

HEMELB_STEERING_INITIALISE_TAG    (pixels_x, 512);HEMELB_STEERING_INITIALISE_TAG(pixels_y, 512);

HEMELB_STEERING_INITIALISE_TAG    (terminate_signal, false);HEMELB_STEERING_INITIALISE_TAG(vis_mode, 0);HEMELB_STEERING_INITIALISE_TAG(streaklines_per_pulsatile_period, 5);HEMELB_STEERING_INITIALISE_TAG(streakline_length, 100);HEMELB_STEERING_INITIALISE_TAG(doRendering, false);

}  }
