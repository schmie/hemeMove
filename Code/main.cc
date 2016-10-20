
// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "comm/MpiEnvironment.h"
#include "configuration/CommandLine.h"
#include "SimulationMaster.h"

int main(int argc, char *argv[])
{
  // main function needed to perform the entire simulation. Some
  // simulation parameters and performance statistics are output on
  // standard output

  // Bring up MPI
  hemelb::comm::MpiEnvironment mpi(argc, argv);
  hemelb::log::Logger::Init();
  try
  {
    auto commWorld = mpi.World();
    
    try
    {
      // Parse command line
      hemelb::configuration::CommandLine options = hemelb::configuration::CommandLine(argc, argv);

      // Start the debugger (if requested)
      hemelb::debug::Debugger::Init(options.GetDebug(), argv[0], commWorld.get());

      // Prepare main simulation object...
      SimulationMaster master = SimulationMaster(options, commWorld);

      // ..and run it.
      master.RunSimulation();
    }

    // Interpose this catch to print usage before propagating the error.
    catch (hemelb::configuration::CommandLine::OptionError& e)
    {
      hemelb::log::Logger::Log<hemelb::log::Critical, hemelb::log::Singleton>(hemelb::configuration::CommandLine::GetUsage());
      throw;
    }
  }
  catch (std::exception& e)
  {
    hemelb::log::Logger::Log<hemelb::log::Critical, hemelb::log::OnePerCore>(e.what());
    mpi.Abort(-1);
  }
  // MPI gets finalised by MpiEnv's d'tor.
}
