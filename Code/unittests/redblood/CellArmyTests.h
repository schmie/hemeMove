// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_UNITTESTS_REDBLOOD_CELLARMYTESTS_H
#define HEMELB_UNITTESTS_REDBLOOD_CELLARMYTESTS_H

#include <cppunit/TestFixture.h>
#include "unittests/redblood/Fixtures.h"
#include "redblood/CellArmy.h"
#include "Traits.h"

namespace hemelb
{
  namespace unittests
  {
    namespace redblood
    {
      //! Mock cell for ease of use
      class FakeCell : public hemelb::redblood::Cell
      {
        public:
          mutable size_t nbcalls = 0;
#         ifndef CPP11_HAS_CONSTRUCTOR_INHERITANCE
          FakeCell(Mesh const &mesh) :
              Cell(mesh)
          {
          }
          FakeCell(std::shared_ptr<MeshData> const &mesh) :
              Cell(mesh)
          {
          }
#         else
          using hemelb::redblood::Cell::Cell;
#         endif 
          //! Facet bending energy
          virtual LatticeEnergy operator()() const override
          {
            return 0;
          }
          //! Facet bending energy
          virtual LatticeEnergy operator()(std::vector<LatticeForceVector> &) const override
          {
            ++nbcalls;
            return 0;
          }
      };

      class CellArmyTests : public helpers::FourCubeBasedTestFixture
      {
          CPPUNIT_TEST_SUITE (CellArmyTests);
          CPPUNIT_TEST (testCell2Fluid);
          CPPUNIT_TEST (testCell2FluidWithoutCells);
          CPPUNIT_TEST (testCellInsertion);
          CPPUNIT_TEST (testCellRemoval);
          CPPUNIT_TEST (testCellOutput);
          CPPUNIT_TEST (testFluid2Cell);
          CPPUNIT_TEST_SUITE_END();

          LatticeDistance const cutoff = 5.0;
          typedef lb::lattices::D3Q15 D3Q15;
          typedef hemelb::Traits<>::Reinstantiate<D3Q15, lb::GuoForcingLBGK>::Type::ChangeStencil<
              stencil::FourPoint>::Type Traits;

        public:
          void setUp();
          void tearDown();
          void testCell2Fluid();
          void testCell2FluidWithoutCells();
          void testFluid2Cell();
          void testCellInsertion();
          void testCellRemoval();
          void testCellOutput();

        private:
          virtual size_t CubeSize() const
          {
            return 32 + 2;
          }

          std::shared_ptr<TemplateCellContainer> BuildTemplateContainer(CellContainer const &cellContainer) const
          {
            auto templates = std::make_shared<TemplateCellContainer>();
            for (auto cell : cellContainer)
            {
              templates->emplace(cell->GetTemplateName(), cell->clone());
            }
            return templates;
          }

          hemelb::reporting::Timers* timers;
      };

      void CellArmyTests::setUp()
      {
        helpers::FourCubeBasedTestFixture::setUp();
        timers = new hemelb::reporting::Timers(Comms());
      }

      void CellArmyTests::tearDown()
      {
        helpers::FourCubeBasedTestFixture::tearDown();
        delete timers;
      }

      void CellArmyTests::testCell2FluidWithoutCells()
      {
        CellContainer cells;
        CellArmy<Traits> army(*latDat, cells, BuildTemplateContainer(cells), *timers, cutoff);
        army.SetCell2Cell(/* intensity */1e0, /* cutoff */0.5);
        army.SetCell2Wall(/* intensity */1e0, /* cutoff */0.5);
        army.Cell2FluidInteractions();
      }

      void CellArmyTests::testCell2Fluid()
      {
        // Fixture all pairs far from one another
        auto cells = TwoPancakeSamosas<FakeCell>(cutoff);
        assert(cells.size() == 2);
        assert(std::dynamic_pointer_cast<FakeCell>( (*cells.begin()))->nbcalls == 0);
        assert(std::dynamic_pointer_cast<FakeCell>( (*std::next(cells.begin())))->nbcalls == 0);

        helpers::ZeroOutFOld(latDat);
        helpers::ZeroOutForces(latDat);

        CellArmy<Traits> army(*latDat, cells, BuildTemplateContainer(cells), *timers, cutoff);
        army.SetCell2Cell(/* intensity */1e0, /* cutoff */0.5);
        army.SetCell2Wall(/* intensity */1e0, /* cutoff */0.5);
        army.Cell2FluidInteractions();

        CPPUNIT_ASSERT(std::dynamic_pointer_cast<FakeCell>( (*cells.begin()))->nbcalls == 1);
        CPPUNIT_ASSERT(std::dynamic_pointer_cast<FakeCell>( (*std::next(cells.begin())))->nbcalls
            == 1);

        for (site_t i(0); i < latDat->GetLocalFluidSiteCount(); ++i)
        {
          CPPUNIT_ASSERT(helpers::is_zero(latDat->GetSite(i).GetForce()));
        }

        LatticePosition const n0(15 - 0.1, 15.5, 15.5);
        LatticePosition const n1(15 + 0.2, 15.5, 15.5);
        (*cells.begin())->GetVertices().front() = n0;
        (*std::next(cells.begin()))->GetVertices().front() = n1;
        army.updateDNC();
        army.Cell2FluidInteractions();

        CPPUNIT_ASSERT(std::dynamic_pointer_cast<FakeCell>( (*cells.begin()))->nbcalls == 2);
        CPPUNIT_ASSERT(std::dynamic_pointer_cast<FakeCell>( (*std::next(cells.begin())))->nbcalls
            == 2);
        CPPUNIT_ASSERT(not helpers::is_zero(latDat->GetSite(15, 15, 15).GetForce()));
      }

      void CellArmyTests::testCellInsertion()
      {
        auto cell = std::make_shared<FakeCell>(pancakeSamosa());
        // Shift cell to be contained in flow domain
        *cell += LatticePosition(2, 2, 2);

        helpers::ZeroOutFOld(latDat);
        helpers::ZeroOutForces(latDat);

        auto cells = CellContainer();
        CellArmy<Traits> army(*latDat, cells, BuildTemplateContainer(cells), *timers, cutoff);
        int called = 0;
        auto callback = [cell, &called](std::function<void(CellContainer::value_type)> inserter)
        {
          ++called;
          inserter(cell);
        };
        army.SetCellInsertion(callback);

        army.CallCellInsertion();
        CPPUNIT_ASSERT_EQUAL(1, called);
        CPPUNIT_ASSERT_EQUAL(3ul, army.GetDNC().size());
        CPPUNIT_ASSERT_EQUAL(1ul, army.GetCells().size());
      }

      void CellArmyTests::testFluid2Cell()
      {
        // Checks that positions of cells are updated. Does not check that attendant
        // DNC is.
        auto cells = TwoPancakeSamosas<FakeCell>(cutoff);
        auto const orig = TwoPancakeSamosas<FakeCell>(cutoff);
        auto const normal = Facet( (*cells.begin())->GetVertices(),
                                  (*cells.begin())->GetFacets()[0]).normal();

        LatticePosition gradient;
        Dimensionless non_neg_pop;
        std::function<Dimensionless(LatticeVelocity const &)> linear, linear_inv;
        std::tie(non_neg_pop, gradient, linear, linear_inv) = helpers::makeLinearProfile(CubeSize(),
                                                                                         latDat,
                                                                                         normal);

        CellArmy<Traits> army(*latDat, cells, BuildTemplateContainer(cells), *timers, cutoff);
        army.Fluid2CellInteractions();

        for (size_t i(0); i < cells.size(); ++i)
        {
          auto const disp = (*std::next(cells.begin(), i))->GetVertices().front()
              - (*std::next(orig.begin(), i))->GetVertices().front();
          auto i_nodeA = (*std::next(cells.begin(), i))->GetVertices().begin();
          auto i_nodeB = (*std::next(orig.begin(), i))->GetVertices().begin();
          auto const i_end = (*std::next(cells.begin(), i))->GetVertices().end();

          for (; i_nodeA != i_end; ++i_nodeA, ++i_nodeB)
          {
            CPPUNIT_ASSERT(helpers::is_zero( (*i_nodeA - *i_nodeB) - disp));
          }
        }
      }

      void CellArmyTests::testCellOutput()
      {
        auto cell = std::make_shared<FakeCell>(tetrahedron());
        // Shift cell to be contained in flow domain
        *cell += LatticePosition(2, 2, 2);

        MeshData::Vertices::value_type barycentre;
        CellArmy<Traits>::CellChangeListener callback =
            [&barycentre](const CellContainer & container)
            {
              barycentre = (*(container.begin()))->GetBarycenter();
            };

        CellContainer intel;
        intel.insert(cell);
        CellArmy<Traits> army(*latDat, intel, BuildTemplateContainer(intel), *timers, cutoff);
        army.AddCellChangeListener(callback);

        army.NotifyCellChangeListeners();
        CPPUNIT_ASSERT_EQUAL(barycentre, cell->GetBarycenter());
      }

      void CellArmyTests::testCellRemoval()
      {
        // The flow extension takes the z > CubeSize()/2 half of the cube
        FlowExtension const outlet(util::Vector3D<Dimensionless>(0, 0, 1),
                                   LatticePosition(CubeSize()/2, CubeSize()/2, CubeSize()/2),
                                   CubeSize()/2,
                                   CubeSize()/2,
                                   CubeSize()/4);
        auto cell = std::make_shared<FakeCell>(pancakeSamosa());
        // Shift cell to be contained in flow domain
        *cell += LatticePosition(2, 2, 2);

        helpers::ZeroOutFOld(latDat);
        helpers::ZeroOutForces(latDat);

        CellContainer intel;
        intel.insert(cell);
        CellArmy<Traits> army(*latDat, intel, BuildTemplateContainer(intel), *timers, cutoff);
        army.SetOutlets(std::vector<FlowExtension>(1, outlet));

        // Check status before attempting to remove cell that should *not* be removed
        CPPUNIT_ASSERT_EQUAL(3ul, army.GetDNC().size());
        CPPUNIT_ASSERT_EQUAL(1ul, army.GetCells().size());

        // Check status after attempting to remove cell that should *not* be removed
        army.CellRemoval();
        CPPUNIT_ASSERT_EQUAL(3ul, army.GetDNC().size());
        CPPUNIT_ASSERT_EQUAL(1ul, army.GetCells().size());

        // Check status after attempting to remove cell that should be removed.
        // Move the cell beyond the fading section of the flow extension.
        *cell += LatticePosition(outlet.origin + outlet.normal * outlet.fadeLength * 1.1);
        army.CellRemoval();
        CPPUNIT_ASSERT_EQUAL(0ul, army.GetDNC().size());
        CPPUNIT_ASSERT_EQUAL(0ul, army.GetCells().size());
      }

      CPPUNIT_TEST_SUITE_REGISTRATION (CellArmyTests);
    }
  }
}

#endif  // ONCE
