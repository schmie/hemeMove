// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "vis/BasicPixel.h"

namespace hemelb
{
  namespace vis
  {
    BasicPixel::BasicPixel()
    {

    }
    BasicPixel::BasicPixel(int iIn, int jIn) :
        i(iIn), j(jIn)
    {

    }

    int BasicPixel::GetI() const
    {
      return i;
    }

    int BasicPixel::GetJ() const
    {
      return j;
    }

    bool BasicPixel::operator <(const BasicPixel& right) const
    {
      return (i < right.i || (i == right.i && j < right.j));
    }

    bool BasicPixel::operator ==(const BasicPixel& right) const
    {
      return (i == right.i && j == right.j);
    }

    MPI_Datatype BasicPixel::GetMPIType()
    {
      const int typeCount = 2;
      int blocklengths[typeCount] = { 1, 1 };

      MPI_Datatype types[typeCount] = { MPI_INT, MPI_INT };

      BasicPixel example;

      MPI_Aint displacements[typeCount];

      MPI_Get_address(&example.i, &displacements[0]);
      MPI_Get_address(&example.j, &displacements[1]);

      displacements[1] -= displacements[0];
      displacements[0] = 0;

      MPI_Datatype ret;

      HEMELB_MPI_CALL(MPI_Type_create_struct, (typeCount, blocklengths, displacements, types, &ret));

      return ret;
    }
  }

  namespace net
  {
    template<>
    MPI_Datatype net::MpiDataTypeTraits<hemelb::vis::BasicPixel>::RegisterMpiDataType()
    {
      MPI_Datatype ret = vis::BasicPixel::GetMPIType();
      HEMELB_MPI_CALL(MPI_Type_commit, (&ret));
      return ret;
    }
  }
}
