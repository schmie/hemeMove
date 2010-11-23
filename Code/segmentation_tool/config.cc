#include "config.h"


unsigned long SOLID_TYPE  = 0U;
unsigned long FLUID_TYPE  = 1U;
unsigned long INLET_TYPE  = 2U;
unsigned long OUTLET_TYPE = 3U;

unsigned int BOUNDARIES        = 3U;
unsigned int INLET_BOUNDARY    = 0U;
unsigned int OUTLET_BOUNDARY   = 1U;
unsigned int WALL_BOUNDARY     = 2U;

unsigned int SITE_TYPE_BITS       = 2U;
unsigned int BOUNDARY_CONFIG_BITS = 14U;
unsigned int BOUNDARY_DIR_BITS    = 4U;
unsigned int BOUNDARY_ID_BITS     = 10U;

unsigned int BOUNDARY_CONFIG_SHIFT = SITE_TYPE_BITS;
unsigned int BOUNDARY_DIR_SHIFT    = BOUNDARY_CONFIG_SHIFT + BOUNDARY_CONFIG_BITS;
unsigned int BOUNDARY_ID_SHIFT     = BOUNDARY_DIR_SHIFT + BOUNDARY_DIR_BITS;

unsigned long SITE_TYPE_MASK       = ((1U << SITE_TYPE_BITS) - 1U);
unsigned long BOUNDARY_CONFIG_MASK = ((1U << BOUNDARY_CONFIG_BITS) - 1U) << BOUNDARY_CONFIG_SHIFT;
unsigned long BOUNDARY_DIR_MASK    = ((1U << BOUNDARY_DIR_BITS) - 1U)    << BOUNDARY_DIR_SHIFT;
unsigned long BOUNDARY_ID_MASK     = ((1U << BOUNDARY_ID_BITS) - 1U)     << BOUNDARY_ID_SHIFT;

int e[] = {
   1, 0, 0,
  -1, 0, 0,
   0, 1, 0,
   0,-1, 0,
   0, 0, 1,
   0, 0,-1,
   1, 1, 1,
  -1,-1,-1,
   1, 1,-1,
  -1,-1, 1,
   1,-1, 1,
  -1, 1,-1,
   1,-1,-1,
  -1, 1, 1
};

int inv_dir[] = {1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12};


Screen screen;

Viewpoint viewpoint;

Vis vis;
