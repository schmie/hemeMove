// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_MULTISCALE_INTERCOMMUNICAND_H
#define HEMELB_MULTISCALE_INTERCOMMUNICAND_H
#include <vector>

namespace hemelb
{
  namespace multiscale
  {
    class BaseSharedValue;

    /***
     * An "intercommunicand" is an object which has values which must be shared with other processes in a multiscale simulation.
     * This communication occurs through the intercommunicator.
     * It manages an array of shared values. These are registered with this intercommunicator which is used as a base class.
     * The order the shared values have within the intercommunicand is significant: in the IntercommunicandType class, the string labels for the fields
     * are specified based on this ordering.
     */
    class Intercommunicand
    {
      public:
        /***
         * Register a shared value by adding it to the list of shared values
         * The constructor for shared values calls back to the intercommunicand, providing syntactic sugar for this.
         * @param value
         */
        void RegisterSharedValue(BaseSharedValue *value)
        {
          values.push_back(value);
        }
        Intercommunicand() :
            values()
        {
        }
        /* For the copy constructor, do not copy the references to the values
         * These will need to be registered by the deriving copy constructor.
         */
        Intercommunicand(const Intercommunicand &other) :
            values()
        {
        }
        std::vector<BaseSharedValue *> & SharedValues()
        {
          return values;
        }
      private:
        std::vector<BaseSharedValue *> values;
    };
  }
}

#endif // HEMELB_MULTISCALE_INTERCOMMUNICAND_H
