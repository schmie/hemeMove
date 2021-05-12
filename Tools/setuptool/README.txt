
# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.
You can build the setuptool locally with:

   python setup.py build_ext --inplace

As a prerequisite you will need to run the same command in hemelb-dev/hemelb/Tools to generate a number of  C source files from Cython code.

In order to do a clean build (after modifications, etc.),  you will need to remove the following files:

   HemeLbSetupTool/Model/Generation.py 
   HemeLbSetupTool/Model/Generation/Wrap.cpp
   HemeLbSetupTool/Model/_Generation.so
