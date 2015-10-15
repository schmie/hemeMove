//
// Copyright (C) University College London, 2007-2012, all rights reserved.
//
// This file is part of HemeLB and is CONFIDENTIAL. You may not work
// with, install, use, duplicate, modify, redistribute or share this
// file, or any part thereof, other than as allowed by any agreement
// specifically made by you with University College London.
//

#ifndef HEMELB_UNITTESTS_REDBLOOD_BORDERS_TESTS_H
#define HEMELB_UNITTESTS_REDBLOOD_BORDERS_TESTS_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "redblood/Borders.h"

namespace hemelb
{
  namespace unittests
  {
    namespace redblood
    {
      using namespace hemelb::redblood;
      // Tests functionality that is *not* part of the HemeLB API
      // Checks that we know how to compute geometric properties between facets
      // However, HemeLB only cares about energy and forces
      class BordersTests : public CppUnit::TestFixture
      {
          CPPUNIT_TEST_SUITE (BordersTests);
          CPPUNIT_TEST(testNothingToIterate);
          CPPUNIT_TEST(testIterate);
          CPPUNIT_TEST_SUITE_END();
        public:

          size_t index(size_t x, size_t y, size_t z) const
          {
            return (x + 1) + (y + 1) * 3 + (z + 1) * 9;
          }
          std::vector<bool> visitme(BorderBoxIterator iterator)
          {
            std::vector<bool> result(21, false);
            for(; iterator; ++iterator)
            {
              result[index(iterator->x, iterator->y, iterator->z)] = true;
            }
            return result;
          }

          void testNothingToIterate()
          {
            auto const visited = visitme(BorderBoxIterator(0));
            for(auto const v: visited)
            {
              CPPUNIT_ASSERT(not v);
            }
          }

          void testIterate(size_t border, std::vector<LatticeVector> const &indices)
          {
            auto visited = visitme(BorderBoxIterator(border));
            for(auto const &id: indices)
            {
              CPPUNIT_ASSERT(visited[index(id.x, id.y, id.z)]);
              visited[index(id.x, id.y, id.z)] = false;
            }
            for(auto const v: visited)
            {
              CPPUNIT_ASSERT(not v);
            }
          }
          void testIterate()
          {
            testIterate((size_t)Borders::BOTTOM, {{-1, 0, 0}});
            testIterate((size_t)Borders::TOP, {{1, 0, 0}});
            testIterate((size_t)Borders::SOUTH, {{0, -1, 0}});
            testIterate((size_t)Borders::NORTH, {{0, 1, 0}});
            testIterate((size_t)Borders::WEST, {{0, 0, -1}});
            testIterate((size_t)Borders::EAST, {{0, 0, 1}});
            testIterate(
                (size_t)Borders::BOTTOM + (size_t)Borders::TOP,
                {{-1, 0, 0}, {1, 0, 0}}
            );
            testIterate(
                (size_t)Borders::BOTTOM + (size_t)Borders::NORTH,
                {{-1, 0, 0}, {0, 1, 0}, {-1, 1, 0}}
            );
            testIterate(
                (size_t)Borders::BOTTOM + (size_t)Borders::NORTH + (size_t)Borders::EAST,
                {{-1, 0, 0}, {0, 1, 0}, {-1, 1, 0}, {0, 0, 1}, {-1, 0, 1}, {0, 1, 1}, {-1, 1, 1}}
            );
          }
      };

      CPPUNIT_TEST_SUITE_REGISTRATION (BordersTests);
    }
  }
}

#endif  // ONCE
