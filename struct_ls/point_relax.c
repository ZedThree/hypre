/*BHEADER**********************************************************************
 * (c) 1999   The Regents of the University of California
 *
 * See the file COPYRIGHT_and_DISCLAIMER for a complete copyright
 * notice, contact person, and disclaimer.
 *
 * $Revision$
 *********************************************************************EHEADER*/
/******************************************************************************
 *
 *
 *****************************************************************************/

#include "headers.h"

/* this currently cannot be greater than 7 */
#ifdef MAX_DEPTH
#undef MAX_DEPTH
#endif
#define MAX_DEPTH 7

/*--------------------------------------------------------------------------
 * hypre_PointRelaxData data structure
 *--------------------------------------------------------------------------*/

typedef struct
{
   MPI_Comm                comm;
                       
   double                  tol;                /* not yet used */
   int                     max_iter;
   int                     rel_change;         /* not yet used */
   int                     zero_guess;
   double                  weight;
                         
   int                     num_pointsets;
   int                    *pointset_sizes;
   int                    *pointset_ranks;
   hypre_Index            *pointset_strides;
   hypre_Index           **pointset_indices;
                       
   hypre_StructMatrix     *A;
   hypre_StructVector     *b;
   hypre_StructVector     *x;

   hypre_StructVector     *t;

   int                     diag_rank;

   hypre_ComputePkg      **compute_pkgs;

   /* log info (always logged) */
   int                     num_iterations;
   int                     time_index;
   int                     flops;

} hypre_PointRelaxData;

/*--------------------------------------------------------------------------
 * hypre_PointRelaxCreate
 *--------------------------------------------------------------------------*/

void *
hypre_PointRelaxCreate( MPI_Comm  comm )
{
   hypre_PointRelaxData *relax_data;

   hypre_Index           stride;
   hypre_Index           indices[1];

   relax_data = hypre_CTAlloc(hypre_PointRelaxData, 1);

   (relax_data -> comm)       = comm;
   (relax_data -> time_index) = hypre_InitializeTiming("PointRelax");

   /* set defaults */
   (relax_data -> tol)              = 1.0e-06;
   (relax_data -> max_iter)         = 1000;
   (relax_data -> rel_change)       = 0;
   (relax_data -> zero_guess)       = 0;
   (relax_data -> weight)           = 1.0;
   (relax_data -> num_pointsets)    = 0;
   (relax_data -> pointset_sizes)   = NULL;
   (relax_data -> pointset_ranks)   = NULL;
   (relax_data -> pointset_strides) = NULL;
   (relax_data -> pointset_indices) = NULL;
   (relax_data -> t)                = NULL;

   hypre_SetIndex(stride, 1, 1, 1);
   hypre_SetIndex(indices[0], 0, 0, 0);
   hypre_PointRelaxSetNumPointsets((void *) relax_data, 1);
   hypre_PointRelaxSetPointset((void *) relax_data, 0, 1, stride, indices);

   return (void *) relax_data;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelaxDestroy
 *--------------------------------------------------------------------------*/

int
hypre_PointRelaxDestroy( void *relax_vdata )
{
   hypre_PointRelaxData *relax_data = relax_vdata;
   int                   i;
   int                   ierr = 0;

   if (relax_data)
   {
      for (i = 0; i < (relax_data -> num_pointsets); i++)
      {
         hypre_TFree(relax_data -> pointset_indices[i]);
         hypre_ComputePkgDestroy(relax_data -> compute_pkgs[i]);
      }
      hypre_TFree(relax_data -> pointset_sizes);
      hypre_TFree(relax_data -> pointset_ranks);
      hypre_TFree(relax_data -> pointset_strides);
      hypre_TFree(relax_data -> pointset_indices);
      hypre_StructMatrixDestroy(relax_data -> A);
      hypre_StructVectorDestroy(relax_data -> b);
      hypre_StructVectorDestroy(relax_data -> x);
      hypre_TFree(relax_data -> compute_pkgs);
      hypre_StructVectorDestroy(relax_data -> t);

      hypre_FinalizeTiming(relax_data -> time_index);
      hypre_TFree(relax_data);
   }

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelaxSetup
 *--------------------------------------------------------------------------*/

int
hypre_PointRelaxSetup( void               *relax_vdata,
                       hypre_StructMatrix *A,
                       hypre_StructVector *b,
                       hypre_StructVector *x           )
{
   hypre_PointRelaxData  *relax_data = relax_vdata;

   int                    num_pointsets    = (relax_data -> num_pointsets);
   int                   *pointset_sizes   = (relax_data -> pointset_sizes);
   hypre_Index           *pointset_strides = (relax_data -> pointset_strides);
   hypre_Index          **pointset_indices = (relax_data -> pointset_indices);
   hypre_StructVector    *t;
   int                    diag_rank;
   hypre_ComputePkg     **compute_pkgs;

   hypre_Index            unit_stride;
   hypre_Index            diag_index;
   hypre_IndexRef         stride;
   hypre_IndexRef         index;
                       
   hypre_StructGrid      *grid;
   hypre_StructStencil   *stencil;
                       
   hypre_BoxArrayArray   *send_boxes;
   hypre_BoxArrayArray   *recv_boxes;
   int                  **send_processes;
   int                  **recv_processes;
   int                   *send_order;
   int                   *recv_order;
   hypre_BoxArrayArray   *indt_boxes;
   hypre_BoxArrayArray   *dept_boxes;

   hypre_BoxArrayArray   *orig_indt_boxes;
   hypre_BoxArrayArray   *orig_dept_boxes;
   hypre_BoxArrayArray   *box_aa;
   hypre_BoxArray        *box_a;
   hypre_Box             *box;
   int                    box_aa_size;
   int                    box_a_size;
   hypre_BoxArrayArray   *new_box_aa;
   hypre_BoxArray        *new_box_a;
   hypre_Box             *new_box;

   double                 scale;
   int                    frac;

   int                    i, j, k, p, m, compute_i;
   int                    ierr = 0;
                       
   /*----------------------------------------------------------
    * Set up the temp vector
    *----------------------------------------------------------*/

   if ((relax_data -> t) == NULL)
   {
      t = hypre_StructVectorCreate(hypre_StructVectorComm(b),
                                   hypre_StructVectorGrid(b));
      hypre_StructVectorSetNumGhost(t, hypre_StructVectorNumGhost(b));
      hypre_StructVectorInitialize(t);
      hypre_StructVectorAssemble(t);
      (relax_data -> t) = t;
   }

   /*----------------------------------------------------------
    * Find the matrix diagonal
    *----------------------------------------------------------*/

   grid    = hypre_StructMatrixGrid(A);
   stencil = hypre_StructMatrixStencil(A);

   hypre_SetIndex(diag_index, 0, 0, 0);
   diag_rank = hypre_StructStencilElementRank(stencil, diag_index);

   /*----------------------------------------------------------
    * Set up the compute packages
    *----------------------------------------------------------*/

   hypre_SetIndex(unit_stride, 1, 1, 1);

   compute_pkgs = hypre_CTAlloc(hypre_ComputePkg *, num_pointsets);

   for (p = 0; p < num_pointsets; p++)
   {
      hypre_CreateComputeInfo(grid, stencil,
                              &send_boxes, &recv_boxes,
                              &send_processes, &recv_processes,
                              &send_order, &recv_order,
                              &orig_indt_boxes, &orig_dept_boxes);

      stride = pointset_strides[p];

      for (compute_i = 0; compute_i < 2; compute_i++)
      {
         switch(compute_i)
         {
            case 0:
            box_aa = orig_indt_boxes;
            break;

            case 1:
            box_aa = orig_dept_boxes;
            break;
         }
         box_aa_size = hypre_BoxArrayArraySize(box_aa);
         new_box_aa = hypre_BoxArrayArrayCreate(box_aa_size);

         for (i = 0; i < box_aa_size; i++)
         {
            box_a = hypre_BoxArrayArrayBoxArray(box_aa, i);
            box_a_size = hypre_BoxArraySize(box_a);
            new_box_a = hypre_BoxArrayArrayBoxArray(new_box_aa, i);
            hypre_BoxArraySetSize(new_box_a, box_a_size * pointset_sizes[p]);

            k = 0;
            for (m = 0; m < pointset_sizes[p]; m++)
            {
               index  = pointset_indices[p][m];

               for (j = 0; j < box_a_size; j++)
               {
                  box = hypre_BoxArrayBox(box_a, j);
                  new_box = hypre_BoxArrayBox(new_box_a, k);
                  
                  hypre_CopyBox(box, new_box);
                  hypre_ProjectBox(new_box, index, stride);
                  
                  k++;
               }
            }
         }

         switch(compute_i)
         {
            case 0:
            indt_boxes = new_box_aa;
            break;

            case 1:
            dept_boxes = new_box_aa;
            break;
         }
      }

      hypre_ComputePkgCreate(send_boxes, recv_boxes,
                             unit_stride, unit_stride,
                             send_processes, recv_processes,
                             send_order, recv_order,
                             indt_boxes, dept_boxes,
                             stride, grid,
                             hypre_StructVectorDataSpace(x), 1,
                             &compute_pkgs[p]);

      hypre_BoxArrayArrayDestroy(orig_indt_boxes);
      hypre_BoxArrayArrayDestroy(orig_dept_boxes);
   }

   /*----------------------------------------------------------
    * Set up the relax data structure
    *----------------------------------------------------------*/

   (relax_data -> A) = hypre_StructMatrixRef(A);
   (relax_data -> x) = hypre_StructVectorRef(x);
   (relax_data -> b) = hypre_StructVectorRef(b);
   (relax_data -> diag_rank)    = diag_rank;
   (relax_data -> compute_pkgs) = compute_pkgs;

   /*-----------------------------------------------------
    * Compute flops
    *-----------------------------------------------------*/

   scale = 0.0;
   for (p = 0; p < num_pointsets; p++)
   {
      stride = pointset_strides[p];
      frac   = hypre_IndexX(stride);
      frac  *= hypre_IndexY(stride);
      frac  *= hypre_IndexZ(stride);
      scale += (pointset_sizes[p] / frac);
   }
   (relax_data -> flops) = scale * (hypre_StructMatrixGlobalSize(A) +
                                    hypre_StructVectorGlobalSize(x));

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelax
 *--------------------------------------------------------------------------*/

int
hypre_PointRelax( void               *relax_vdata,
                  hypre_StructMatrix *A,
                  hypre_StructVector *b,
                  hypre_StructVector *x           )
{
   hypre_PointRelaxData  *relax_data = relax_vdata;

   int                    max_iter         = (relax_data -> max_iter);
   int                    zero_guess       = (relax_data -> zero_guess);
   double                 weight           = (relax_data -> weight);
   int                    num_pointsets    = (relax_data -> num_pointsets);
   int                   *pointset_ranks   = (relax_data -> pointset_ranks);
   hypre_Index           *pointset_strides = (relax_data -> pointset_strides);
   hypre_StructVector    *t                = (relax_data -> t);
   int                    diag_rank        = (relax_data -> diag_rank);
   hypre_ComputePkg     **compute_pkgs     = (relax_data -> compute_pkgs);

   hypre_ComputePkg      *compute_pkg;
   hypre_CommHandle      *comm_handle;
                        
   hypre_BoxArrayArray   *compute_box_aa;
   hypre_BoxArray        *compute_box_a;
   hypre_Box             *compute_box;
                        
   hypre_Box             *A_data_box;
   hypre_Box             *b_data_box;
   hypre_Box             *x_data_box;
   hypre_Box             *t_data_box;
                        
   int                    Ai;
   int                    bi;
   int                    xi;
   int                    xoff0;
   int                    xoff1;
   int                    xoff2;
   int                    xoff3;
   int                    xoff4;
   int                    xoff5;
   int                    xoff6;
   int                    ti;
                        
   double                *Ap;
   double                *Ap0;
   double                *Ap1;
   double                *Ap2;
   double                *Ap3;
   double                *Ap4;
   double                *Ap5;
   double                *Ap6;
   double                *bp;
   double                *xp;
   double                *tp;
                        
   hypre_IndexRef         stride;
   hypre_IndexRef         start;
   hypre_Index            loop_size;
                        
   hypre_StructStencil   *stencil;
   hypre_Index           *stencil_shape;
   int                    stencil_size;
                        
   int                    constant_coefficient;

   int                    iter, p, compute_i, i, j, k;
   int                    si, sk, ssi[MAX_DEPTH], depth;
   int                    loopi, loopj, loopk;
   int                    pointset;

   int                    ierr = 0;
   double xdotx, bdotb;

   /*----------------------------------------------------------
    * Initialize some things and deal with special cases
    *----------------------------------------------------------*/

   bdotb = hypre_StructInnerProd(b,b);
   xdotx = hypre_StructInnerProd(x,x);

   hypre_BeginTiming(relax_data -> time_index);

   hypre_StructMatrixDestroy(relax_data -> A);
   hypre_StructVectorDestroy(relax_data -> b);
   hypre_StructVectorDestroy(relax_data -> x);
   (relax_data -> A) = hypre_StructMatrixRef(A);
   (relax_data -> x) = hypre_StructVectorRef(x);
   (relax_data -> b) = hypre_StructVectorRef(b);

   (relax_data -> num_iterations) = 0;

   /* if max_iter is zero, return */
   if (max_iter == 0)
   {
      /* if using a zero initial guess, return zero */
      if (zero_guess)
      {
         hypre_StructVectorSetConstantValues(x, 0.0);
      }

      hypre_EndTiming(relax_data -> time_index);
      return ierr;
   }

   constant_coefficient = hypre_StructMatrixConstantCoefficient(A);

   stencil       = hypre_StructMatrixStencil(A);
   stencil_shape = hypre_StructStencilShape(stencil);
   stencil_size  = hypre_StructStencilSize(stencil);

   /*----------------------------------------------------------
    * Do zero_guess iteration
    *----------------------------------------------------------*/

   p    = 0;
   iter = 0;

   if (zero_guess)
   {
      if (num_pointsets > 1)
      {
         hypre_StructVectorSetConstantValues(x, 0.0);
      }
      pointset = pointset_ranks[p];
      compute_pkg = compute_pkgs[pointset];
      stride = pointset_strides[pointset];

      for (compute_i = 0; compute_i < 2; compute_i++)
      {
         switch(compute_i)
         {
         case 0:
         {
            compute_box_aa = hypre_ComputePkgIndtBoxes(compute_pkg);
         }
         break;

         case 1:
         {
            compute_box_aa = hypre_ComputePkgDeptBoxes(compute_pkg);
         }
         break;
         }

         hypre_ForBoxArrayI(i, compute_box_aa)
            {
               compute_box_a = hypre_BoxArrayArrayBoxArray(compute_box_aa, i);

               A_data_box =
                  hypre_BoxArrayBox(hypre_StructMatrixDataSpace(A), i);
               b_data_box =
                  hypre_BoxArrayBox(hypre_StructVectorDataSpace(b), i);
               x_data_box =
                  hypre_BoxArrayBox(hypre_StructVectorDataSpace(x), i);

               Ap = hypre_StructMatrixBoxData(A, i, diag_rank);
               bp = hypre_StructVectorBoxData(b, i);
               xp = hypre_StructVectorBoxData(x, i);

               hypre_ForBoxI(j, compute_box_a)
                  {
                     compute_box = hypre_BoxArrayBox(compute_box_a, j);

                     start  = hypre_BoxIMin(compute_box);
                     hypre_BoxGetStrideSize(compute_box, stride, loop_size);

                     if ( constant_coefficient )
                     {
                        Ai = hypre_CCBoxIndexRank( A_data_box, start );
                        hypre_BoxLoop2Begin(loop_size,
                                            b_data_box, start, stride, bi,
                                            x_data_box, start, stride, xi);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,bi,xi
#include "hypre_box_smp_forloop.h"
                        hypre_BoxLoop2For(loopi, loopj, loopk, bi, xi)
                           {
                              xp[xi] = bp[bi] / Ap[Ai];
                                    /* for debugging >>> */
                                    assert( xp[xi]<1.0e20 );
                                    assert( xp[xi]>-1.0e20 );
                           }
                        hypre_BoxLoop2End(bi, xi);
                     }
                     else
                     {
                        hypre_BoxLoop3Begin(loop_size,
                                            A_data_box, start, stride, Ai,
                                            b_data_box, start, stride, bi,
                                            x_data_box, start, stride, xi);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,Ai,bi,xi
#include "hypre_box_smp_forloop.h"
                        hypre_BoxLoop3For(loopi, loopj, loopk, Ai, bi, xi)
                           {
                              xp[xi] = bp[bi] / Ap[Ai];
                           }
                        hypre_BoxLoop3End(Ai, bi, xi);
                     }
                  }
            }
      }

      if (weight != 1.0)
      {
         hypre_StructScale(weight, x);
      }

      p    = (p + 1) % num_pointsets;
      iter = iter + (p == 0);
   }

   /*----------------------------------------------------------
    * Do regular iterations
    *----------------------------------------------------------*/

   while (iter < max_iter)
   {
      pointset = pointset_ranks[p];
      compute_pkg = compute_pkgs[pointset];
      stride = pointset_strides[pointset];

      hypre_StructCopy(x, t);

      for (compute_i = 0; compute_i < 2; compute_i++)
      {
         switch(compute_i)
         {
            case 0:
            {
               xp = hypre_StructVectorData(x);
               hypre_InitializeIndtComputations(compute_pkg, xp, &comm_handle);
               compute_box_aa = hypre_ComputePkgIndtBoxes(compute_pkg);
            }
            break;

            case 1:
            {
               hypre_FinalizeIndtComputations(comm_handle);
               compute_box_aa = hypre_ComputePkgDeptBoxes(compute_pkg);
            }
            break;
         }

         hypre_ForBoxArrayI(i, compute_box_aa)
            {
               compute_box_a = hypre_BoxArrayArrayBoxArray(compute_box_aa, i);

               A_data_box =
                  hypre_BoxArrayBox(hypre_StructMatrixDataSpace(A), i);
               b_data_box =
                  hypre_BoxArrayBox(hypre_StructVectorDataSpace(b), i);
               x_data_box =
                  hypre_BoxArrayBox(hypre_StructVectorDataSpace(x), i);
               t_data_box =
                  hypre_BoxArrayBox(hypre_StructVectorDataSpace(t), i);

               bp = hypre_StructVectorBoxData(b, i);
               xp = hypre_StructVectorBoxData(x, i);
               tp = hypre_StructVectorBoxData(t, i);

               hypre_ForBoxI(j, compute_box_a)
                  {
                     compute_box = hypre_BoxArrayBox(compute_box_a, j);

                     start  = hypre_BoxIMin(compute_box);
                     hypre_BoxGetStrideSize(compute_box, stride, loop_size);

                     hypre_BoxLoop2Begin(loop_size,
                                         b_data_box, start, stride, bi,
                                         t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,bi,ti
#include "hypre_box_smp_forloop.h"
                     hypre_BoxLoop2For(loopi, loopj, loopk, bi, ti)
                        {
                           tp[ti] = bp[bi];
                        }
                     hypre_BoxLoop2End(bi, ti);

                     /* unroll up to depth MAX_DEPTH */
                     for (si = 0; si < stencil_size; si += MAX_DEPTH)
                     {
                        depth = hypre_min(MAX_DEPTH, (stencil_size - si));

                        for (k = 0, sk = si; k < depth; sk++)
                        {
                           if (sk == diag_rank)
                           {
                              depth--;
                           }
                           else
                           {
                              ssi[k] = sk;
                              k++;
                           }
                        }
                           

                        if ( constant_coefficient )
                        {
                           /* >>>> maybe more efficient with the stencil loop inside? >>> */
                           /* >>> TO DO: loop unrolling here as in variable coefficient >>> */
                           /* This constant coefficient code is based on the older, not-unrolled
                              version ... */
                           for (si = 0; si < stencil_size; si++)
                           {

                              if ( si!=diag_rank )
                              {
                                 Ap = hypre_StructMatrixBoxData(A, i, si);
                                 xp = hypre_StructVectorBoxData(x, i) +
                                    hypre_BoxOffsetDistance(x_data_box,
                                                            stencil_shape[si]);
                                 Ai = hypre_CCBoxIndexRank( A_data_box, start );
                                 hypre_BoxLoop2Begin(loop_size,
                                                     x_data_box, start, stride, xi,
                                                     t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,xi,ti
#include "hypre_box_smp_forloop.h"
                                 hypre_BoxLoop2For(loopi, loopj, loopk, xi, ti)
                                    {
                                       tp[ti] -= Ap[Ai] * xp[xi];
                                       /* for debugging >>> */
                                       assert( tp[ti]<1.0e20 );
                                       assert( tp[ti]>-1.0e20 );
                                    }
                                 hypre_BoxLoop2End(xi, ti);
                              }
                           }
                        }
                        else
                        {
                           /* original non-unrolled loop (w/o the #define/#include lines):
                              hypre_BoxLoop3Begin(loop_size,
                              A_data_box, start, stride, Ai,
                              x_data_box, start, stride, xi,
                              t_data_box, start, stride, ti);
                              hypre_BoxLoop3For(loopi, loopj, loopk, Ai, xi, ti)
                              {
                              tp[ti] -= Ap[Ai] * xp[xi];
                              }
                              hypre_BoxLoop3End(Ai, xi, ti);
                           */


                           switch(depth)
                           {
                           case 7:
                              Ap6 = hypre_StructMatrixBoxData(A, i, ssi[6]);
                              xoff6 = hypre_BoxOffsetDistance(
                                 x_data_box, stencil_shape[ssi[6]]);

                           case 6:
                              Ap5 = hypre_StructMatrixBoxData(A, i, ssi[5]);
                              xoff5 = hypre_BoxOffsetDistance(
                                 x_data_box, stencil_shape[ssi[5]]);

                           case 5:
                              Ap4 = hypre_StructMatrixBoxData(A, i, ssi[4]);
                              xoff4 = hypre_BoxOffsetDistance(
                                 x_data_box, stencil_shape[ssi[4]]);

                           case 4:
                              Ap3 = hypre_StructMatrixBoxData(A, i, ssi[3]);
                              xoff3 = hypre_BoxOffsetDistance(
                                 x_data_box, stencil_shape[ssi[3]]);

                           case 3:
                              Ap2 = hypre_StructMatrixBoxData(A, i, ssi[2]);
                              xoff2 = hypre_BoxOffsetDistance(
                                 x_data_box, stencil_shape[ssi[2]]);

                           case 2:
                              Ap1 = hypre_StructMatrixBoxData(A, i, ssi[1]);
                              xoff1 = hypre_BoxOffsetDistance(
                                 x_data_box, stencil_shape[ssi[1]]);

                           case 1:
                              Ap0 = hypre_StructMatrixBoxData(A, i, ssi[0]);
                              xoff0 = hypre_BoxOffsetDistance(
                                 x_data_box, stencil_shape[ssi[0]]);

                           case 0:

                              break;
                           }
                           switch(depth)
                           {
                           case 7:
                              hypre_BoxLoop3Begin(loop_size,
                                                  A_data_box, start, stride, Ai,
                                                  x_data_box, start, stride, xi,
                                                  t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,Ai,xi,ti
#include "hypre_box_smp_forloop.h"
                              hypre_BoxLoop3For(loopi, loopj, loopk, Ai, xi, ti)
                                 {
                                    tp[ti] -=
                                       Ap0[Ai] * xp[xi + xoff0] +
                                       Ap1[Ai] * xp[xi + xoff1] +
                                       Ap2[Ai] * xp[xi + xoff2] +
                                       Ap3[Ai] * xp[xi + xoff3] +
                                       Ap4[Ai] * xp[xi + xoff4] +
                                       Ap5[Ai] * xp[xi + xoff5] +
                                       Ap6[Ai] * xp[xi + xoff6];
                                 }
                              hypre_BoxLoop3End(Ai, xi, ti);
                              break;

                           case 6:
                              hypre_BoxLoop3Begin(loop_size,
                                                  A_data_box, start, stride, Ai,
                                                  x_data_box, start, stride, xi,
                                                  t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,Ai,xi,ti
#include "hypre_box_smp_forloop.h"
                              hypre_BoxLoop3For(loopi, loopj, loopk, Ai, xi, ti)
                                 {
                                    tp[ti] -=
                                       Ap0[Ai] * xp[xi + xoff0] +
                                       Ap1[Ai] * xp[xi + xoff1] +
                                       Ap2[Ai] * xp[xi + xoff2] +
                                       Ap3[Ai] * xp[xi + xoff3] +
                                       Ap4[Ai] * xp[xi + xoff4] +
                                       Ap5[Ai] * xp[xi + xoff5];
                                 }
                              hypre_BoxLoop3End(Ai, xi, ti);
                              break;

                           case 5:
                              hypre_BoxLoop3Begin(loop_size,
                                                  A_data_box, start, stride, Ai,
                                                  x_data_box, start, stride, xi,
                                                  t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,Ai,xi,ti
#include "hypre_box_smp_forloop.h"
                              hypre_BoxLoop3For(loopi, loopj, loopk, Ai, xi, ti)
                                 {
                                    tp[ti] -=
                                       Ap0[Ai] * xp[xi + xoff0] +
                                       Ap1[Ai] * xp[xi + xoff1] +
                                       Ap2[Ai] * xp[xi + xoff2] +
                                       Ap3[Ai] * xp[xi + xoff3] +
                                       Ap4[Ai] * xp[xi + xoff4];
                                 }
                              hypre_BoxLoop3End(Ai, xi, ti);
                              break;

                           case 4:
                              hypre_BoxLoop3Begin(loop_size,
                                                  A_data_box, start, stride, Ai,
                                                  x_data_box, start, stride, xi,
                                                  t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,Ai,xi,ti
#include "hypre_box_smp_forloop.h"
                              hypre_BoxLoop3For(loopi, loopj, loopk, Ai, xi, ti)
                                 {
                                    tp[ti] -=
                                       Ap0[Ai] * xp[xi + xoff0] +
                                       Ap1[Ai] * xp[xi + xoff1] +
                                       Ap2[Ai] * xp[xi + xoff2] +
                                       Ap3[Ai] * xp[xi + xoff3];
                                 }
                              hypre_BoxLoop3End(Ai, xi, ti);
                              break;

                           case 3:
                              hypre_BoxLoop3Begin(loop_size,
                                                  A_data_box, start, stride, Ai,
                                                  x_data_box, start, stride, xi,
                                                  t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,Ai,xi,ti
#include "hypre_box_smp_forloop.h"
                              hypre_BoxLoop3For(loopi, loopj, loopk, Ai, xi, ti)
                                 {
                                    tp[ti] -=
                                       Ap0[Ai] * xp[xi + xoff0] +
                                       Ap1[Ai] * xp[xi + xoff1] +
                                       Ap2[Ai] * xp[xi + xoff2];
                                 }
                              hypre_BoxLoop3End(Ai, xi, ti);
                              break;

                           case 2:
                              hypre_BoxLoop3Begin(loop_size,
                                                  A_data_box, start, stride, Ai,
                                                  x_data_box, start, stride, xi,
                                                  t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,Ai,xi,ti
#include "hypre_box_smp_forloop.h"
                              hypre_BoxLoop3For(loopi, loopj, loopk, Ai, xi, ti)
                                 {
                                    tp[ti] -=
                                       Ap0[Ai] * xp[xi + xoff0] +
                                       Ap1[Ai] * xp[xi + xoff1];
                                 }
                              hypre_BoxLoop3End(Ai, xi, ti);
                              break;

                           case 1:
                              hypre_BoxLoop3Begin(loop_size,
                                                  A_data_box, start, stride, Ai,
                                                  x_data_box, start, stride, xi,
                                                  t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,Ai,xi,ti
#include "hypre_box_smp_forloop.h"
                              hypre_BoxLoop3For(loopi, loopj, loopk, Ai, xi, ti)
                                 {
                                    tp[ti] -=
                                       Ap0[Ai] * xp[xi + xoff0];
                                 }
                              hypre_BoxLoop3End(Ai, xi, ti);
                              break;

                           case 0:
                              break;
                           }
                        }
                     }

                     Ap = hypre_StructMatrixBoxData(A, i, diag_rank);

                     if ( constant_coefficient )
                     {
                        Ai = hypre_CCBoxIndexRank( A_data_box, start );
                        hypre_BoxLoop1Begin(loop_size,
                                            t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,ti
#include "hypre_box_smp_forloop.h"
                        hypre_BoxLoop1For(loopi, loopj, loopk, ti)
                           {
                              assert( Ap[Ai] != 0 );
                              tp[ti] /= Ap[Ai];
                              /* for debugging >>> */
                              assert( tp[ti]<1.0e20 );
                              assert( tp[ti]>-1.0e20 );
                           }
                        hypre_BoxLoop1End(ti);
                     }
                     else
                     {
                        hypre_BoxLoop2Begin(loop_size,
                                            A_data_box, start, stride, Ai,
                                            t_data_box, start, stride, ti);
#define HYPRE_BOX_SMP_PRIVATE loopk,loopi,loopj,Ai,ti
#include "hypre_box_smp_forloop.h"
                        hypre_BoxLoop2For(loopi, loopj, loopk, Ai, ti)
                           {
                              tp[ti] /= Ap[Ai];
                           }
                        hypre_BoxLoop2End(Ai, ti);
                     }
                  }
            }
      }

      if (weight != 1.0)
      {
         hypre_StructScale((1.0 - weight), x);
         hypre_StructAxpy(weight, t, x);
      }
      else
      {
         hypre_StructCopy(t, x);
      }

      p    = (p + 1) % num_pointsets;
      iter = iter + (p == 0);
   }
   
   (relax_data -> num_iterations) = iter;

   /*-----------------------------------------------------------------------
    * Return
    *-----------------------------------------------------------------------*/

   hypre_IncFLOPCount(relax_data -> flops);
   hypre_EndTiming(relax_data -> time_index);

   bdotb = hypre_StructInnerProd(b,b);
   xdotx = hypre_StructInnerProd(x,x);

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelaxSetTol
 *--------------------------------------------------------------------------*/

int
hypre_PointRelaxSetTol( void   *relax_vdata,
                        double  tol         )
{
   hypre_PointRelaxData *relax_data = relax_vdata;
   int                   ierr = 0;

   (relax_data -> tol) = tol;

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelaxSetMaxIter
 *--------------------------------------------------------------------------*/

int
hypre_PointRelaxSetMaxIter( void *relax_vdata,
                            int   max_iter    )
{
   hypre_PointRelaxData *relax_data = relax_vdata;
   int                   ierr = 0;

   (relax_data -> max_iter) = max_iter;

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelaxSetZeroGuess
 *--------------------------------------------------------------------------*/

int
hypre_PointRelaxSetZeroGuess( void *relax_vdata,
                              int   zero_guess  )
{
   hypre_PointRelaxData *relax_data = relax_vdata;
   int                   ierr = 0;

   (relax_data -> zero_guess) = zero_guess;

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelaxSetWeight
 *--------------------------------------------------------------------------*/

int
hypre_PointRelaxSetWeight( void    *relax_vdata,
                           double   weight      )
{
   hypre_PointRelaxData *relax_data = relax_vdata;
   int                   ierr = 0;

   (relax_data -> weight) = weight;

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelaxSetNumPointsets
 *--------------------------------------------------------------------------*/

int
hypre_PointRelaxSetNumPointsets( void *relax_vdata,
                                 int   num_pointsets )
{
   hypre_PointRelaxData *relax_data = relax_vdata;
   int                   i;
   int                   ierr = 0;

   /* free up old pointset memory */
   for (i = 0; i < (relax_data -> num_pointsets); i++)
   {
      hypre_TFree(relax_data -> pointset_indices[i]);
   }
   hypre_TFree(relax_data -> pointset_sizes);
   hypre_TFree(relax_data -> pointset_ranks);
   hypre_TFree(relax_data -> pointset_strides);
   hypre_TFree(relax_data -> pointset_indices);

   /* alloc new pointset memory */
   (relax_data -> num_pointsets)    = num_pointsets;
   (relax_data -> pointset_sizes)   = hypre_TAlloc(int, num_pointsets);
   (relax_data -> pointset_ranks)   = hypre_TAlloc(int, num_pointsets);
   (relax_data -> pointset_strides) = hypre_TAlloc(hypre_Index, num_pointsets);
   (relax_data -> pointset_indices) = hypre_TAlloc(hypre_Index *,
                                                   num_pointsets);
   for (i = 0; i < num_pointsets; i++)
   {
      (relax_data -> pointset_sizes[i]) = 0;
      (relax_data -> pointset_ranks[i]) = i;
      (relax_data -> pointset_indices[i]) = NULL;
   }

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelaxSetPointset
 *--------------------------------------------------------------------------*/

int
hypre_PointRelaxSetPointset( void        *relax_vdata,
                             int          pointset,
                             int          pointset_size,
                             hypre_Index  pointset_stride,
                             hypre_Index *pointset_indices )
{
   hypre_PointRelaxData *relax_data = relax_vdata;
   int                   i;
   int                   ierr = 0;

   /* free up old pointset memory */
   hypre_TFree(relax_data -> pointset_indices[pointset]);

   /* alloc new pointset memory */
   (relax_data -> pointset_indices[pointset]) =
      hypre_TAlloc(hypre_Index, pointset_size);

   (relax_data -> pointset_sizes[pointset]) = pointset_size;
   hypre_CopyIndex(pointset_stride,
                   (relax_data -> pointset_strides[pointset]));
   for (i = 0; i < pointset_size; i++)
   {
      hypre_CopyIndex(pointset_indices[i],
                      (relax_data -> pointset_indices[pointset][i]));
   }

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelaxSetPointsetRank
 *--------------------------------------------------------------------------*/

int
hypre_PointRelaxSetPointsetRank( void *relax_vdata,
                                 int   pointset,
                                 int   pointset_rank )
{
   hypre_PointRelaxData *relax_data = relax_vdata;
   int                   ierr = 0;

   (relax_data -> pointset_ranks[pointset]) = pointset_rank;

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_PointRelaxSetTempVec
 *--------------------------------------------------------------------------*/

int
hypre_PointRelaxSetTempVec( void               *relax_vdata,
                            hypre_StructVector *t           )
{
   hypre_PointRelaxData *relax_data = relax_vdata;
   int                   ierr = 0;

   hypre_StructVectorDestroy(relax_data -> t);
   (relax_data -> t) = hypre_StructVectorRef(t);

   return ierr;
}

