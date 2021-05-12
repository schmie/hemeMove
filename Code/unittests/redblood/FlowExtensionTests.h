// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_UNITTESTS_REDBLOOD_FLOWEXTENSIONTESTS_H
#define HEMELB_UNITTESTS_REDBLOOD_FLOWEXTENSIONTESTS_H

#include <cppunit/TestFixture.h>
#include "redblood/FlowExtension.h"
#include "redblood/xmlIO.h"
#include "redblood/RBCConfig.h"
#include "unittests/redblood/Fixtures.h"
#include "util/Vector3D.h"
#include "units.h"

namespace hemelb
{
  namespace unittests
  {
    namespace redblood
    {
      class FlowExtensionTests : public FlowExtensionFixture
      {
          CPPUNIT_TEST_SUITE (FlowExtensionTests);
          CPPUNIT_TEST (testAxis);
          CPPUNIT_TEST (testCircumference);
          CPPUNIT_TEST (testLinearWeight);
          CPPUNIT_TEST (testReadFromXML);CPPUNIT_TEST_SUITE_END();

          typedef util::Vector3D<LatticeDistance> Point;

        public:

          void testAxis()
          {
            // Check points along the axis from just outside the start of the cylinder (x = 0) to just inside
            CPPUNIT_ASSERT_MESSAGE("Point (-0.001, 0, 0) is outside the cylinder",
                                   !contains(flowExt, Point(-0.001, 0, 0)));
            CPPUNIT_ASSERT_MESSAGE("Point ( 0.000, 0, 0) is inside the cylinder",
                                   contains(flowExt, Point(0.000, 0, 0)));
            CPPUNIT_ASSERT_MESSAGE("Point ( 0.001, 0, 0) is inside the cylinder",
                                   contains(flowExt, Point(0.001, 0, 0)));

            // Check points along the axis from just inside the end of the cylinder (x = 10) to just outside
            CPPUNIT_ASSERT_MESSAGE("Point ( 9.999, 0, 0) is inside the cylinder",
                                   contains(flowExt, Point(9.999, 0, 0)));
            CPPUNIT_ASSERT_MESSAGE("Point (10.000, 0, 0) is inside the cylinder",
                                   contains(flowExt, Point(10.000, 0, 0)));
            CPPUNIT_ASSERT_MESSAGE("Point (10.001, 0, 0) is outside the cylinder",
                                   !contains(flowExt, Point(10.001, 0, 0)));
          }

          void testCircumference()
          {
            // Points around the circumference should be inside the cylinder
            CPPUNIT_ASSERT_MESSAGE("Point (0.0,    1.000,   0.000 ) is inside the cylinder",
                                   contains(flowExt, Point(0.0, 1.0, 0.0)));
            CPPUNIT_ASSERT_MESSAGE("Point (0.0,  cos(45),  sin(45)) is inside the cylinder",
                                   contains(flowExt, Point(0.0, cos(45), sin(45))));
            CPPUNIT_ASSERT_MESSAGE("Point (0.0,    0.000,   1.000 ) is inside the cylinder",
                                   contains(flowExt, Point(0.0, 0.0, 1.0)));
            CPPUNIT_ASSERT_MESSAGE("Point (0.0,  cos(45), -sin(45)) is inside the cylinder",
                                   contains(flowExt, Point(0.0, cos(45), -sin(45))));
            CPPUNIT_ASSERT_MESSAGE("Point (0.0,   -1.000,   0.000 ) is inside the cylinder",
                                   contains(flowExt, Point(0.0, -1.0, 0.0)));
            CPPUNIT_ASSERT_MESSAGE("Point (0.0, -cos(45), -sin(45)) is inside the cylinder",
                                   contains(flowExt, Point(0.0, -cos(45), -sin(45))));
            CPPUNIT_ASSERT_MESSAGE("Point (0.0,    0.000,  -1.000 ) is inside the cylinder",
                                   contains(flowExt, Point(0.0, 0.0, -1.0)));
            CPPUNIT_ASSERT_MESSAGE("Point (0.0, -cos(45),  sin(45)) is inside the cylinder",
                                   contains(flowExt, Point(0.0, -cos(45), sin(45))));

            // Points forming a square around the axis should be outside the cylinder
            CPPUNIT_ASSERT_MESSAGE("Point(0.0,  1.0,  1.0) is outside the cylinder",
                                   !contains(flowExt, Point(0.0, 1.0, 1.0)));
            CPPUNIT_ASSERT_MESSAGE("Point(0.0,  1.0, -1.0) is outside the cylinder",
                                   !contains(flowExt, Point(0.0, 1.0, -1.0)));
            CPPUNIT_ASSERT_MESSAGE("Point(0.0, -1.0, -1.0) is outside the cylinder",
                                   !contains(flowExt, Point(0.0, -1.0, -1.0)));
            CPPUNIT_ASSERT_MESSAGE("Point(0.0, -1.0,  1.0) is outside the cylinder",
                                   !contains(flowExt, Point(0.0, -1.0, 1.0)));
          }

          void testLinearWeight()
          {
            FlowExtension const flow(util::Vector3D<Dimensionless>(-1.0, 0, 0),
                                     LatticePosition(0.5, 0.5, 0.5),
                                     2.0,
                                     0.5,
                                     1.5);
            std::vector<Dimensionless> ys;
            ys.push_back(0.5);
            ys.push_back(0.7);
            std::vector<Dimensionless> epsilons;
            epsilons.push_back(0.3);
            epsilons.push_back(0.5);
            epsilons.push_back(0.7);
            epsilons.push_back(1.);
            for (auto y : ys)
            {
              CPPUNIT_ASSERT_DOUBLES_EQUAL(0e0,
                                           linearWeight(flow, LatticePosition(2.4, y, 0.5)),
                                           1e-8);
              CPPUNIT_ASSERT_DOUBLES_EQUAL(1e0,
                                           linearWeight(flow, LatticePosition(0.5, y, 0.5)),
                                           1e-8);
              for (auto epsilon : epsilons)
              {
                auto const pos = flow.origin + flow.normal * flow.fadeLength * epsilon
                    + LatticePosition(0, y - 0.5, 0);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(1e0 - epsilon, linearWeight(flow, pos), 1e-8);
              }
            }
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0e0,
                                         linearWeight(flow, LatticePosition(0.5, 0.5, 1.0 + 1e-8)),
                                         1e-8);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1e0,
                                         linearWeight(flow, LatticePosition(0.5, 0.5, 1.0 - 1e-8)),
                                         1e-8);
          }

          void testReadFromXML()
          {
            TiXmlDocument doc;
            doc.Parse("<inlet>"
                      "  <normal units=\"dimensionless\" value=\"(0.0,1.0,1.0)\" />"
                      "  <position units=\"m\" value=\"(0.1,0.2,0.3)\" />"
                      "  <flowextension>"
                      "    <length units=\"m\" value=\"0.1\" />"
                      "    <radius units=\"m\" value=\"0.01\" />"
                      "    <fadelength units=\"m\" value=\"0.05\" />"
                      "  </flowextension>"
                      "</inlet>");
            util::UnitConverter converter(0.5, 0.6, PhysicalPosition{0.7}, DEFAULT_FLUID_DENSITY_Kg_per_m3, 0.0);
            auto flow = readFlowExtension(doc.FirstChildElement("inlet"), converter);
            // Normal is opposite direction compared to XML inlet definition
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-2e0 / std::sqrt(2),
                                         LatticePosition(0, 1, 1).Dot(flow.normal),
                                         1e-8);
            auto const length = converter.ConvertToLatticeUnits("m", 0.1);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(length, flow.length, 1e-8);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(converter.ConvertToLatticeUnits("m", 0.01),
                                         flow.radius,
                                         1e-8);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(converter.ConvertToLatticeUnits("m", 0.05),
                                         flow.fadeLength,
                                         1e-8);

            // position is at opposite end compared to XML inlet definition
            auto const position = converter.ConvertPositionToLatticeUnits(LatticePosition(0.1,
                                                                                          0.2,
                                                                                          0.3));
            CPPUNIT_ASSERT_DOUBLES_EQUAL(position.x, flow.origin.x, 1e-8);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(position.y + length / std::sqrt(2e0), flow.origin.y, 1e-8);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(position.z + length / std::sqrt(2e0), flow.origin.z, 1e-8);

            // Check flow extension is positioned correctly
            auto isInFlow = [&flow, &converter](double x, double y, double z)
            {
              LatticePosition const pos(x, y, z);
              LatticePosition const conv = converter.ConvertPositionToLatticeUnits(pos);
              return contains(flow, conv);
            };
            auto const l = 0.1 / std::sqrt(2);
            CPPUNIT_ASSERT(isInFlow(0.1, 0.25, 0.35));
            CPPUNIT_ASSERT(isInFlow(0.1, 0.2 + 0.001, 0.3 + 0.001));
            CPPUNIT_ASSERT(isInFlow(0.1, 0.2 + l - 0.001, 0.3 + l - 0.001));
            CPPUNIT_ASSERT(not isInFlow(0.1, 0.2 + l + 0.001, 0.3 + l + 0.001));
            CPPUNIT_ASSERT(not isInFlow(0.1, 0.2 - 0.001, 0.3 - 0.001));
          }
      };

      CPPUNIT_TEST_SUITE_REGISTRATION (FlowExtensionTests);

    } // namespace: redblood
  } // namespace: unittests
} // namespace: hemelb

#endif // HEMELB_UNITTESTS_REDBLOOD_FLOWEXTENSION_TESTS_H
