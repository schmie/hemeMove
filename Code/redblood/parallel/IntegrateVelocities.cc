// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "redblood/parallel/IntegrateVelocities.h"
#include "util/Iterator.h"
#include <boost/uuid/uuid_io.hpp>

namespace hemelb
{
  namespace redblood
  {
    namespace parallel
    {
      void IntegrateVelocities::PostMessageLength(LentCells const &lent)
      {
        sendNodeCount.GetSendBuffer().resize(sendNodeCount.GetCommunicator().GetNeighborsCount());
        std::fill(sendNodeCount.GetSendBuffer().begin(), sendNodeCount.GetSendBuffer().end(), 0);

        auto const neighbors = sendNodeCount.GetCommunicator().GetNeighbors();
        for (auto const lentCells : lent)
        {
          for (auto const &cell : lentCells.second)
          {
            auto const i_index = std::find(neighbors.begin(), neighbors.end(), lentCells.first);
            assert(i_index != neighbors.end());
            sendNodeCount.GetSendBuffer()[i_index - neighbors.begin()] += cell->GetNumberOfNodes();
          }
        }
        // Then send message
        sendNodeCount.send();
      }

      void IntegrateVelocities::UpdatePositionsNonLocal(NodeDistributions const& distributions,
                                                        CellContainer &owned)
      {
        sendVelocities.receive();

        std::map<int, int> offsets;
        for (auto const neighbor : sendVelocities.GetCommunicator().GetNeighbors())
        {
          offsets[neighbor] = 0;
        }

        auto const neighbors = sendNodeCount.GetCommunicator().GetNeighbors();
        // Count the number of vertices and cells
        for (auto const & cell : owned)
        {
          assert(distributions.count(cell->GetTag()) == 1);
          auto const dist = distributions.find(cell->GetTag())->second;
          for (auto const neighbor : util::enumerate(neighbors))
          {
            auto const nVertices = dist.CountNodes(neighbor.value);
            if (nVertices == 0)
            {
              continue;
            }

            auto const offset = offsets[neighbor.value];
            offsets[neighbor.value] += nVertices;

            for (auto const item : util::enumerate(dist[neighbor.value]))
            {
              assert(int(item.value) <= cell->GetNumberOfNodes());
              auto &pos = cell->GetVertices()[item.value];
              pos += sendVelocities.GetReceive(neighbor.value, offset + item.index);
            }
          }
        }
      }
    } // parallel
  } // redblood
} // hemelb
