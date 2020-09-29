// Copyright (c) 2010-2020, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. All Rights reserved. See files
// LICENSE and NOTICE for details. LLNL-CODE-806117.
//
// This file is part of the MFEM library. For more information and source code
// availability visit https://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the BSD-3 license. We welcome feedback and contributions, see file
// CONTRIBUTING.md for details.

#ifndef MFEM_MUMPS
#define MFEM_MUMPS

#include "../config/config.hpp"

#ifdef MFEM_USE_MUMPS
#ifdef MFEM_USE_MPI
#include "operator.hpp"
#include "hypre.hpp"

#include <mpi.h>
#include "dmumps_c.h"
#include <vector>

namespace mfem
{
class MUMPSSolver : public mfem::Solver
{
public:
   // Default Constructor.
   MUMPSSolver() {}

   // Factor and solve the linear system y = Op^{-1} x.
   void Mult(const Vector &x, Vector &y) const;

   // Set the operator.
   void SetOperator(const Operator &op);

   void UseDistributedRHS(bool dist_rhs_) { dist_rhs = dist_rhs_; }

   void UseDistributedSol(bool dist_sol_) { dist_sol = dist_sol_; }

   // Default destructor.
   ~MUMPSSolver();

private:
   MPI_Comm comm;

   int numProcs;

   int myid;

   int row_start;

   int *I;

   int *J;

   Vector rhs_glob;

   Vector sol_loc;

   Array<int> row_starts;

   Array<int> recv_counts;

   Array<int> displs;

   Array<int> irhs_loc;

   Array<int> isol_loc;

   // MUMPS workspace
   // macro s.t. indices match MUMPS documentation
#define ICNTL(I) icntl[(I) -1]
#define INFO(I) info[(I) -1]

   DMUMPS_STRUC_C *id;

   void SetParameters();

   int GetRowRank(int i, const Array<int> &row_starts_) const;

   void RedistributeSol(const Array<int> &row_map,
                        const Vector &x,
                        Vector &y) const;

   // flag for distributed rhs
   bool dist_rhs = false;
   // flag for distributed sol
   bool dist_sol = false;

}; // mfem::MUMPSSolver class

} // namespace mfem

#endif // MFEM_USE_MPI
#endif // MFEM_USE_MUMPS
#endif // MFEM_MUMPS
