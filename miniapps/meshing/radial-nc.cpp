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
//
//       --------------------------------------------------------------
//       Radial NC: radial non-conforming mesh generator
//       --------------------------------------------------------------
//
// This miniapp TODO
//
// Compile with: make radial-nc
//
// Sample runs:  radial-nc
//               radial-nc TODO

#include "mfem.hpp"
#include <fstream>
#include <iostream>

using namespace mfem;
using namespace std;


struct Params
{
   double r, dr;
   double a, da;
   double b, db;

   Params() = default;
   Params(double r0, double r1, double a0, double a1)
      : r(r0), dr(r1 - r0), a(a0), da(a1 - a0) {}
   Params(double r0, double r1, double a0, double a1, double b0, double b1)
      : r(r0), dr(r1 - r0), a(a0), da(a1 - a0), b(b0), db(b1 - b0) {}
};


Mesh* Make2D(int nsteps, double rstep, double phi, double aspect, int order,
             bool sfc)
{
   Mesh *mesh = new Mesh(2, 0, 0);

   int origin = mesh->AddVertex(0.0, 0.0);

   // n is the number of steps in the polar direction
   int n = 1;
   while (phi * rstep / n * aspect > rstep) { n++; }

   double r = rstep;
   int first = mesh->AddVertex(r, 0.0);

   Array<Params> params;
   Array<Pair<int, int>> blocks;

   // create triangles around the origin
   double prev_alpha = 0.0;
   for (int i = 0; i < n; i++)
   {
      double alpha = phi * (i+1) / n;
      mesh->AddVertex(r*cos(alpha), r*sin(alpha));
      mesh->AddTriangle(origin, first+i, first+i+1);

      params.Append(Params(0, r, prev_alpha, alpha));
      prev_alpha = alpha;
   }

   mesh->AddBdrSegment(origin, first, 1);
   mesh->AddBdrSegment(first+n, origin, 2);

   for (int k = 1; k < nsteps; k++)
   {
      // m is the number of polar steps of the previous row
      int m = n;
      int prev_first = first;

      double prev_r = r;
      r += rstep;

      if (phi * (r + prev_r)/2 / n * aspect <= rstep)
      {
         if (k == 1) { blocks.Append(Pair<int, int>(mesh->GetNE(), n)); }

         first = mesh->AddVertex(r, 0.0);
         mesh->AddBdrSegment(prev_first, first, 1);

         // create a row of quads, same number as in previous row
         prev_alpha = 0.0;
         for (int i = 0; i < n; i++)
         {
            double alpha = phi * (i+1) / n;
            mesh->AddVertex(r*cos(alpha), r*sin(alpha));
            mesh->AddQuad(prev_first+i, first+i, first+i+1, prev_first+i+1);

            params.Append(Params(prev_r, r, prev_alpha, alpha));
            prev_alpha = alpha;
         }

         mesh->AddBdrSegment(first+n, prev_first+n, 2);
      }
      else // we need to double the number of elements per row
      {
         n *= 2;

         blocks.Append(Pair<int, int>(mesh->GetNE(), n));

         // first create hanging vertices
         int hang;
         for (int i = 0; i < m; i++)
         {
            double alpha = phi * (2*i+1) / n;
            int index = mesh->AddVertex(prev_r*cos(alpha), prev_r*sin(alpha));
            mesh->AddVertexParents(index, prev_first+i, prev_first+i+1);
            if (!i) { hang = index; }
         }

         first = mesh->AddVertex(r, 0.0);
         int a = prev_first, b = first;

         mesh->AddBdrSegment(a, b, 1);

         // create a row of quad pairs
         prev_alpha = 0.0;
         for (int i = 0; i < m; i++)
         {
            int c = hang+i, e = a+1;

            double alpha_half = phi * (2*i+1) / n;
            int d = mesh->AddVertex(r*cos(alpha_half), r*sin(alpha_half));

            double alpha = phi * (2*i+2) / n;
            int f = mesh->AddVertex(r*cos(alpha), r*sin(alpha));

            mesh->AddQuad(a, b, d, c);
            mesh->AddQuad(c, d, f, e);

            a = e, b = f;

            params.Append(Params(prev_r, r, prev_alpha, alpha_half));
            params.Append(Params(prev_r, r, alpha_half, alpha));
            prev_alpha = alpha;
         }

         mesh->AddBdrSegment(b, a, 2);
      }
   }

   for (int i = 0; i < n; i++)
   {
      mesh->AddBdrSegment(first+i, first+i+1, 3);
   }

   // reorder blocks of elements with Grid SFC ordering
   if (sfc)
   {
      blocks.Append(Pair<int, int>(mesh->GetNE(), 0));

      Array<Params> new_params(params.Size());

      Array<int> ordering(mesh->GetNE());
      for (int i = 0; i < blocks[0].one; i++)
      {
         ordering[i] = i;
         new_params[i] = params[i];
      }

      Array<int> coords;
      for (int i = 0; i < blocks.Size()-1; i++)
      {
         int beg = blocks[i].one;
         int width = blocks[i].two;
         int height = (blocks[i+1].one - blocks[i].one) / width;

         NCMesh::GridSfcOrdering2D(width, height, coords);

         for (int j = 0, k = 0; j < coords.Size(); k++, j += 2)
         {
            int sfc = ((i & 1) ? coords[j] : (width-1 - coords[j]))
                      + coords[j+1]*width;
            int old_index = beg + sfc;

            ordering[old_index] = beg + k;
            new_params[beg + k] = params[old_index];
         }
      }

      mesh->ReorderElements(ordering, false);

      mfem::Swap(params, new_params);
   }

   mesh->FinalizeMesh();

   // create high-order curvature
   if (order > 1)
   {
      mesh->SetCurvature(order);

      GridFunction *nodes = mesh->GetNodes();
      const FiniteElementSpace *fes = mesh->GetNodalFESpace();

      Array<int> dofs;
      MFEM_ASSERT(params.Size() == mesh->GetNE(), "");

      for (int i = 0; i < mesh->GetNE(); i++)
      {
         const Params &par = params[i];
         const IntegrationRule &ir = fes->GetFE(i)->GetNodes();
         Geometry::Type geom = mesh->GetElementBaseGeometry(i);
         fes->GetElementDofs(i, dofs);

         for (int j = 0; j < dofs.Size(); j++)
         {
            double r, a;
            if (geom == Geometry::SQUARE)
            {
               r = par.r + ir[j].x * par.dr;
               a = par.a + ir[j].y * par.da;
            }
            else
            {
               double rr = ir[j].x + ir[j].y;
               if (std::abs(rr) < 1e-12) { continue; }
               r = par.r + rr * par.dr;
               a = par.a + ir[j].y/rr * par.da;
            }
            (*nodes)(fes->DofToVDof(dofs[j], 0)) = r*cos(a);
            (*nodes)(fes->DofToVDof(dofs[j], 1)) = r*sin(a);
         }
      }

      nodes->RestrictConforming();
   }

   return mesh;
}


struct Vert : public Hashed2
{
   int id;
};

int GetMidVertex(int v1, int v2, double r, double a, double b,
                 Mesh *mesh, HashTable<Vert> &hash)
{
   int vmid = hash.FindId(v1, v2);
   if (vmid < 0)
   {
      vmid = hash.GetId(v1, v2);
      hash[vmid].id =
         mesh->AddVertex(r*cos(a)*cos(b), r*sin(a)*cos(b), r*sin(b));
   }
   return hash[vmid].id;
}

void MakeLayer(int vx1, int vy1, int vz1, int vx2, int vy2, int vz2, int level,
               double r1, double r2, double a1, double a2, double a3,
               double b1, double b2, Mesh *mesh, HashTable<Vert> &hash,
               Array<Params> &params)
{
   if (level <= 0)
   {
      mesh->AddWedge(vx1, vy1, vz1, vx2, vy2, vz2);
      params.Append(Params(r1, r2, a1, a2, b1, b2));
   }
   else
   {
      double a12 = (a1+a2)/2, a23 = (a2+a3)/2, a31 = (a3+a1)/2;
      double b12 = (b1+b2)/2;

      int vxy1 = GetMidVertex(vx1, vy1, r1, a12, b1, mesh, hash);
      int vyz1 = GetMidVertex(vy1, vz1, r1, a23, b12, mesh, hash);
      int vxz1 = GetMidVertex(vx1, vz1, r1, a31, b12, mesh, hash);
      int vxy2 = GetMidVertex(vx2, vy2, r2, a12, b1, mesh, hash);
      int vyz2 = GetMidVertex(vy2, vz2, r2, a23, b12, mesh, hash);
      int vxz2 = GetMidVertex(vx2, vz2, r2, a31, b12, mesh, hash);

      MakeLayer(vx1, vxy1, vxz1, vx2, vxy2, vxz2, level-1, r1, r2,
                a1, a12, a31, b1, b12, mesh, hash, params);
      MakeLayer(vxy1, vy1, vyz1, vxy2, vy2, vyz2, level-1, r1, r2,
                a12, a2, a23, b1, b12, mesh, hash, params);
      MakeLayer(vxz1, vyz1, vz1, vxz2, vyz2, vz2, level-1, r1, r2,
                a31, a23, a3, b12, b2, mesh, hash, params);
      MakeLayer(vyz1, vxz1, vxy1, vyz2, vxz2, vxy2, level-1, r1, r2,
                a23, a31, a12, b12, b1, mesh, hash, params);
   }
}

Mesh* Make3D(int nsteps, double rstep, double aspect, int order)
{
   Mesh *mesh = new Mesh(3, 0, 0);

   HashTable<Vert> hash;
   Array<Params> params;

   int a = mesh->AddVertex(rstep, 0, 0);
   int b = mesh->AddVertex(0, rstep, 0);
   int c = mesh->AddVertex(0, 0, rstep);

   double r = rstep + rstep;
   int d = mesh->AddVertex(r, 0, 0);
   int e = mesh->AddVertex(0, r, 0);
   int f = mesh->AddVertex(0, 0, r);

   const double pi2 = M_PI / 2;

   MakeLayer(a, b, c, d, e, f, 3, rstep, r, 0, pi2, 0, 0, pi2, mesh, hash, params);

   mesh->FinalizeMesh();

   return mesh;
}


int main(int argc, char *argv[])
{
   int dim = 2;
   double radius = 1.0;
   int nsteps = 10;
   double angle = 90;
   double aspect = 1.0;
   int order = 2;
   bool sfc = true;

   // parse command line
   OptionsParser args(argc, argv);
   args.AddOption(&dim, "-d", "--dim", "Mesh dimension (2 or 3).");
   args.AddOption(&radius, "-r", "--radius", "Radius of the domain.");
   args.AddOption(&nsteps, "-n", "--nsteps",
                  "Number of elements along the radial direction");
   args.AddOption(&aspect, "-a", "--aspect",
                  "Target aspect ratio of the elements.");
   args.AddOption(&angle, "-phi", "--phi", "Angular range.");
   args.AddOption(&order, "-o", "--order",
                  "Polynomial degree of mesh curvature.");
   args.AddOption(&sfc, "-sfc", "--sfc", "-no-sfc", "--no-sfc",
                  "Try to order elements along a space-filling curve.");
   args.Parse();
   if (!args.Good())
   {
      args.PrintUsage(cout);
      return EXIT_FAILURE;
   }
   args.PrintOptions(cout);

   // validate options
   MFEM_VERIFY(dim >= 2 && dim <= 3, "");
   MFEM_VERIFY(angle > 0 && angle < 360, "");
   MFEM_VERIFY(nsteps > 0, "");

   double phi = angle * M_PI / 180;

   // generate
   Mesh *mesh;
   if (dim == 2)
   {
      mesh = Make2D(nsteps, radius/nsteps, phi, aspect, order, sfc);
   }
   else
   {
      mesh = Make3D(nsteps, radius/nsteps, aspect, order);
   }

   // save the final mesh
   ofstream ofs("radial.mesh");
   ofs.precision(8);
   mesh->Print(ofs);

   delete mesh;

   return EXIT_SUCCESS;
}

