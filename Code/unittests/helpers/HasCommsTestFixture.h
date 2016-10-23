
// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_UNITTESTS_HELPERS_HASCOMMSTESTFIXTURE_H
#define HEMELB_UNITTESTS_HELPERS_HASCOMMSTESTFIXTURE_H
#include <cppunit/TestFixture.h>
#include "comm/Communicator.h"

namespace hemelb
{
  namespace unittests
  {
    namespace helpers
    {
      class HasCommsTestFixture : public CppUnit::TestFixture
      {

        public:
          void setUp()
          {
            //hemelbCommunicator = net::IOCommunicator::Instance();
          }

          void tearDown()
          {
            //hemelbCommunicator = NULL;
          }

          static void Init(comm::Communicator::ConstPtr inst)
          {
            hemelbCommunicator = inst;
          }

        protected:
          static comm::Communicator::ConstPtr Comms()
          {
            return hemelbCommunicator;
          }
        private:
	static comm::Communicator::ConstPtr hemelbCommunicator;
      };

      comm::Communicator::ConstPtr HasCommsTestFixture::hemelbCommunicator = NULL;
    }
  }
}
#endif // HEMELB_UNITTESTS_HELPERS_HASCOMMSTESTFIXTURE_H
