#include "mpiInclude.h"
#include "Particle.h"

namespace hemelb
{
  namespace vis
  {
    namespace streaklinedrawer
    {
      Particle::Particle() :
          position(NO_VALUE), velocity(NO_VALUE), vel(NO_VALUE), inletID(-1)
      {

      }

      Particle::Particle(float iX, float iY, float iZ, unsigned int iInletId) :
          position(iX, iY, iZ), velocity(0), vel(0), inletID(iInletId)
      {
      }
    }
  }

  template<>
  MPI_Datatype MpiDataTypeTraits<hemelb::vis::streaklinedrawer::Particle>::RegisterMpiDataType()
  {
    const int elementCount = 5;
    int elementBlockLengths[elementCount] = { 1, 1, 1, 1, 1 };

    MPI_Datatype elementTypes[elementCount] = { MPI_LB,
                                                MpiDataType<util::Vector3D<float> >(),
                                                MPI_FLOAT,
                                                MPI_UNSIGNED,
                                                MPI_UB };

    MPI_Aint elementDisplacements[elementCount];

    vis::streaklinedrawer::Particle particle[2];

    MPI_Address(&particle[0], &elementDisplacements[0]);
    MPI_Address(&particle[0].position, &elementDisplacements[1]);
    MPI_Address(&particle[0].vel, &elementDisplacements[2]);
    MPI_Address(&particle[0].inletID, &elementDisplacements[3]);
    MPI_Address(&particle[1], &elementDisplacements[4]);
    for (int element = elementCount - 1; element >= 0; element--)
    {
      elementDisplacements[element] -= elementDisplacements[0];
    }

    MPI_Datatype type;
    MPI_Type_struct(elementCount, elementBlockLengths, elementDisplacements, elementTypes, &type);
    MPI_Type_commit(&type);
    return type;
  }
}
