// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#ifndef HEMELB_CONFIGURATION_COMMANDLINE_H
#define HEMELB_CONFIGURATION_COMMANDLINE_H

#include <vector>
#include <string>

#include "Exception.h"
#include "log/Logger.h"

namespace hemelb
{
  namespace configuration
  {
    /**
     * Abstraction of HemeLB command line parameters.
     * Arguments should be:
     * - -in input xml configuration file (default input.xml)
     * - -out output folder (empty default, but the hemelb::io::PathManager will guess a value from the input file if not given.)
     * - -i number of images (default 10)
     * - -ss steering session i.d. (default 1)
     */
    class CommandLine
    {
      public:
        /**
         * Constructor, constructed from the command line arguments.
         * Parses the command line arguments, and also initialises MPI.
         * @param aargc count of arguments supplied to program, including program name
         * @param aargv values of arguments supplied to program, of which first is program name
         */
        CommandLine(int aargc, const char * const aargv[])
          : CommandLine(std::vector<std::string>(aargv, aargv + aargc))
        {
        };
        CommandLine(std::initializer_list<char const*> init)
          : CommandLine(std::vector<std::string>(init.begin(), init.end()))
        {
        }
        CommandLine(std::vector<std::string> const &argv);
        /**
         * Report to standard output an error message describing the usage
         */
        static std::string GetUsage();
        /**
         * @return Number of images that should be produced
         */
        unsigned int NumberOfImages() const
        {
          return (images);
        }
        /**
         * The output directory that should be used for result files.
         * Empty default, but the hemelb::io::PathManager will guess a value from the input file if not given.)
         * @return Reference to member, the relative or full path to which output files should be written.
         */
        std::string const & GetOutputDir() const
        {
          return (outputDir);
        }
        /**
         * @return Reference to member, the relative or full path from which an input xml config file should be loaded.
         */
        std::string const & GetInputFile() const
        {
          return (inputFile);
        }
        /**
         * @return A unique integer representing the steering session to which to attach.
         */
        int GetSteeringSessionId() const
        {
          return (steeringSessionId);
        }

        /**
         * @return Whether the user requested a debug mode.
         */
        bool GetDebug() const
        {
          return debugMode;
        }

        /**
         * @return  Total count of command line arguments.
         */
        int ArgumentCount() const
        {
          return argv.size();
        }
        /**
         *
         * @return the command line arguments that were given.
         */
        std::vector<std::string> const& Arguments() const
        {
          return argv;
        }

        /**
         * Allow separate catching of these exceptions to enable usage printing.
         */
        class OptionError : public Exception
        {
        };
      private:
        std::string inputFile; //! local or full path to input file
        std::string outputDir; //! local or full path to input file
        unsigned int images; //! images to produce
        int steeringSessionId; //! unique identifier for steering session
        bool debugMode; //! Use debugger
        std::vector<std::string> argv; //! command line arguments
    };
  }
}

#endif //HEMELB_CONFIGURATION_COMMANDLINE_H
