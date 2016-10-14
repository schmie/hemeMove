
// This file is part of HemeLB and is Copyright (C)
// the HemeLB team and/or their institutions, as detailed in the
// file AUTHORS. This software is provided under the terms of the
// license in the file LICENSE.

#include "net/MpiCommunicator.h"
#include "net/MpiGroup.h"
#include "net/MpiRequest.h"

namespace hemelb
{
  namespace net
  {
    namespace
    {
      void Deleter(MPI_Comm* comm)
      {
        int finalized;
        HEMELB_MPI_CALL(MPI_Finalized, (&finalized));
        if (!finalized)
          HEMELB_MPI_CALL(MPI_Comm_free, (comm));
        delete comm;
      }
    }

    MpiCommunicator MpiCommunicator::World()
    {
      return MpiCommunicator(MPI_COMM_WORLD, false);
    }

    MpiCommunicator::MpiCommunicator() : commPtr()
    {
    }

    MpiCommunicator::MpiCommunicator(MPI_Comm communicator, bool owner) : commPtr()
    {
      if (communicator == MPI_COMM_NULL)
        return;

      if (owner)
      {
        commPtr.reset(new MPI_Comm(communicator), Deleter);
      }
      else
      {
        commPtr.reset(new MPI_Comm(communicator));
      }
    }

    MpiCommunicator::~MpiCommunicator()
    {
    }

    bool operator==(const MpiCommunicator& comm1, const MpiCommunicator& comm2)
    {
      if (comm1)
      {
        if (comm2)
        {
          int result;
          HEMELB_MPI_CALL(MPI_Comm_compare,
              (comm1, comm2, &result));
          return result == MPI_IDENT;
        }
        return false;
      }
      return (!comm2);
    }

    bool operator!=(const MpiCommunicator& comm1, const MpiCommunicator& comm2)
    {
      return ! (comm1 == comm2);
    }

    int MpiCommunicator::Rank() const
    {
      int rank;
      HEMELB_MPI_CALL(MPI_Comm_rank, (*commPtr, &rank));
      return rank;
    }

    int MpiCommunicator::Size() const
    {
      int size;
      HEMELB_MPI_CALL(MPI_Comm_size, (*commPtr, &size));
      return size;
    }

    MpiGroup MpiCommunicator::Group() const
    {
      MPI_Group grp;
      HEMELB_MPI_CALL(MPI_Comm_group, (*commPtr, &grp));
      return MpiGroup(grp, true);
    }

    MpiCommunicator MpiCommunicator::Create(const MpiGroup& grp) const
    {
      MPI_Comm newComm;
      HEMELB_MPI_CALL(MPI_Comm_create, (*commPtr, grp, &newComm));
      return MpiCommunicator(newComm, true);
    }

    void MpiCommunicator::Abort(int errCode) const
    {
      HEMELB_MPI_CALL(MPI_Abort, (*commPtr, errCode));
    }

    MpiCommunicator MpiCommunicator::Duplicate() const
    {
      MPI_Comm newComm;
      HEMELB_MPI_CALL(MPI_Comm_dup, (*commPtr, &newComm));
      return MpiCommunicator(newComm, true);
    }

    void MpiCommunicator::Barrier() const
    {
      HEMELB_MPI_CALL(MPI_Barrier, (*commPtr));
    }

    MpiRequest MpiCommunicator::Ibarrier() const
    {
      MPI_Request req;
      HEMELB_MPI_CALL(MPI_Ibarrier, (*commPtr, &req));
      return MpiRequest(req);
    }

    bool MpiCommunicator::Iprobe(int source, int tag, MPI_Status* stat) const
    {
      int flag;
      HEMELB_MPI_CALL(
          MPI_Iprobe,
          (source, tag, *commPtr, &flag, stat)
      );
      return flag;
    }

  }
}
