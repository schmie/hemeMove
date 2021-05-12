// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_MULTISCALE_SHAREDVALUE_H
#define HEMELB_MULTISCALE_SHAREDVALUE_H

#include <iosfwd>

#include "multiscale/Intercommunicand.h"

/*
 * IMPORTANT NOTES
 * - SharedValues are defined generally in InOutlets (iolet) or similar data transfer regions.
 * - BaseSharedValues are then passed on to the Intercommunicator.
 * -> Therefore, the Payload shortcuts in this file work *only* on the iolet level, *NOT* on the Intercommunicator level!
 */

namespace hemelb
{
  namespace multiscale
  {
    /***
     * Shared value common base, allowing shared values with different runtime types to be contained together.
     * These will be static cast to specialised shared values, depending on the type of value contained, using runtime type information
     * stored in an IntercommunicandType
     */
    class BaseSharedValue
    {
      public:
        /***
         * Syntactic sugar making it prettier to register shared values with their owner in an initialiser list.
         * @param owner
         */
        BaseSharedValue(Intercommunicand *owner)
        {
          owner->RegisterSharedValue(this);
        }

    };
    /***
     * A shared value is a field in an object which can be read and set by the multiscale system.
     * Should be used as a field in classes inheriting Intercommunicand.
     * The cast operators are defined so these fields can be treated as interchangeable with their contents.
     * @tparam payload The type of value contained.
     */
    template<class Payload> class SharedValue : public BaseSharedValue
    {
      public:
        typedef Payload PayloadType;
        /***
         * Construct a shared value
         * @param owner The owning intercommunicand, used as "this" in an initialiser list in the parent.
         * @param val An initial value for the payload.
         */
        SharedValue(Intercommunicand* owner, Payload val = Payload()) :
            BaseSharedValue(owner), contents(val)
        {
        }

        Payload GetPayload() const
        {
          return contents;
        }

        void SetPayload(const Payload & input)
        {
          contents = input;
        }

        /***
         * Cast operator to payload value. Leaving this in to make it work with SimConfig...(have taken the others out to improve code clarity)
         */
        operator Payload &()
        {
          return contents;
        }

      private:
        Payload contents;
        template<class U>
        friend std::istream& operator>>(std::istream& stream, SharedValue<U>& sv);
    };

    template<class Payload>
    std::ostream & operator<<(std::ostream & stream, const SharedValue<Payload> & sv)
    {
      return stream << static_cast<Payload&>(sv);
    }
    template<class Payload>
    std::istream& operator>>(std::istream& stream, SharedValue<Payload> & sv)
    {
      return stream >> sv.contents;
    }
  }
}

#endif // HEMELB_MULTISCALE_SHAREDVALUE_H
