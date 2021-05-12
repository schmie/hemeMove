// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_UNITTESTS_REDBLOOD_CELLINTEGRATIONTESTS_H
#define HEMELB_UNITTESTS_REDBLOOD_CELLINTEGRATIONTESTS_H

#include <cppunit/TestFixture.h>
#include <cstdio>
#include "Traits.h"
#include "SimulationMaster.h"
#include "redblood/Cell.h"
#include "redblood/CellController.h"
#include "unittests/redblood/Fixtures.h"
#include "unittests/helpers/LatticeDataAccess.h"
#include "unittests/helpers/FolderTestFixture.h"

namespace hemelb
{
  namespace unittests
  {
    namespace redblood
    {
      class CellIntegrationTests : public helpers::FolderTestFixture
      {
          CPPUNIT_TEST_SUITE (CellIntegrationTests);
          CPPUNIT_TEST (testCellOutOfBounds);
          CPPUNIT_TEST (testIntegration);
          CPPUNIT_TEST (testIntegrationWithoutCells);CPPUNIT_TEST_SUITE_END();

          typedef Traits<>::ChangeKernel<lb::GuoForcingLBGK>::Type Traits;
          typedef CellController<Traits> CellControll;
          typedef SimulationMaster<Traits> MasterSim;
        public:
          void setUp()
          {
            argv[0] = "hemelb";
            argv[1] = "-in";
            argv[2] = "large_cylinder.xml";
            argv[3] = "-i";
            argv[4] = "1";
            argv[5] = "-ss";
            argv[6] = "1111";
            FolderTestFixture::setUp();
            CopyResourceToTempdir("red_blood_cell.txt");
            TiXmlDocument doc(resources::Resource("large_cylinder.xml").Path());
            CopyResourceToTempdir("large_cylinder.xml");
            std::vector<std::string> intel;
            intel.push_back("simulation");
            intel.push_back("steps");
            intel.push_back("value");
            ModifyXMLInput("large_cylinder.xml", std::move(intel), 8);
            CopyResourceToTempdir("large_cylinder.gmy");
            options = std::make_shared<hemelb::configuration::CommandLine>(argc, argv);

            auto cell = std::make_shared<Cell>(icoSphere(4));
            cell->moduli = Cell::Moduli(1e-6, 1e-6, 1e-6, 1e-6);
            cells.insert(cell);

            master = std::make_shared<MasterSim>(*options, Comms());
            helpers::LatticeDataAccess(&master->GetLatticeData()).ZeroOutForces();
          }

          void tearDown()
          {
            master->Finalise();
            master.reset();
          }

          // No errors when interpolation/spreading hits nodes out of bounds
          void testCellOutOfBounds()
          {
            (*cells.begin())->operator+=(master->GetLatticeData().GetGlobalSiteMins() * 2);
            auto controller = std::make_shared<CellControll>(master->GetLatticeData(), cells);
            master->RegisterActor(*controller, 1);
            master->RunSimulation();
            AssertPresent("results/report.txt");
            AssertPresent("results/report.xml");
          }

          // Check that the particles move and result in some force acting on the fluid
          void testIntegration()
          {
            // setup cell position
            auto const &latticeData = master->GetLatticeData();
            auto const mid = LatticePosition(latticeData.GetGlobalSiteMaxes()
                + latticeData.GetGlobalSiteMins()) * 0.5;
            (**cells.begin()) += mid - (*cells.begin())->GetBarycenter();
            (**cells.begin()) += LatticePosition(0, 0, 8 - mid.z);
            (**cells.begin()) *= 5.0;
            auto controller = std::make_shared<CellControll>(master->GetLatticeData(), cells);
            auto const barycenter = (*cells.begin())->GetBarycenter();

            // run
            master->RegisterActor(*controller, 1);
            master->RunSimulation();

            // check position of cell has changed
            auto const moved = (*cells.begin())->GetBarycenter();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0e0, barycenter.x - moved.x, 1e-12);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0e0, barycenter.y - moved.y, 1e-12);
            CPPUNIT_ASSERT(std::abs(barycenter.z - moved.z) > 1e-8);

            // check there is force on one of the lattice site near a node
            // node position is guessed at from geometry
            auto const nodepos = mid + LatticePosition(0, 0, 8 - 5 - mid.z);
            auto const force = latticeData.GetSite(nodepos).GetForce();
            CPPUNIT_ASSERT(std::abs(force.z) > 1e-4);

            AssertPresent("results/report.txt");
            AssertPresent("results/report.xml");
          }

          // Check that the particles move and result in some force acting on the fluid
          void testIntegrationWithoutCells()
          {
            // setup cell position
            hemelb::redblood::CellContainer empty;
            auto controller = std::make_shared<CellControll>(master->GetLatticeData(), empty);

            // run
            master->RegisterActor(*controller, 1);
            master->RunSimulation();

            AssertPresent("results/report.txt");
            AssertPresent("results/report.xml");
          }

        private:
          std::shared_ptr<MasterSim> master;
          std::shared_ptr<hemelb::configuration::CommandLine> options;
          hemelb::redblood::CellContainer cells;
          int const argc = 7;
          char const * argv[7];
      };

      CPPUNIT_TEST_SUITE_REGISTRATION (CellIntegrationTests);
    }
  }
}

#endif  // ONCE
