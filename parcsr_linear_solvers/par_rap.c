/*BHEADER**********************************************************************
 * (c) 1998   The Regents of the University of California
 *
 * See the file COPYRIGHT_and_DISCLAIMER for a complete copyright
 * notice, contact person, and disclaimer.
 *
 * $Revision$
*********************************************************************EHEADER*/

#include "headers.h"

/*--------------------------------------------------------------------------
 * hypre_ParAMGBuildCoarseOperator
 *--------------------------------------------------------------------------*/

int hypre_ParAMGBuildCoarseOperator(    hypre_ParCSRMatrix  *RT,
					hypre_ParCSRMatrix  *A,
					hypre_ParCSRMatrix  *P,
					hypre_ParCSRMatrix **RAP_ptr)

{
   MPI_Comm 	   comm = hypre_ParCSRMatrixComm(A);

   hypre_CSRMatrix *RT_diag = hypre_ParCSRMatrixDiag(RT);
   hypre_CSRMatrix *RT_offd = hypre_ParCSRMatrixOffd(RT);
   int 		   num_cols_offd_RT = hypre_CSRMatrixNumCols(RT_offd);
   int 		   num_rows_offd_RT = hypre_CSRMatrixNumRows(RT_offd);
   hypre_ParCSRCommPkg   *comm_pkg_RT = hypre_ParCSRMatrixCommPkg(RT);
   int		   num_recvs_RT = 0;
   int		   num_sends_RT = 0;
   int		   *send_map_starts_RT;
   int		   *send_map_elmts_RT;

   hypre_CSRMatrix *A_diag = hypre_ParCSRMatrixDiag(A);
   
   double          *A_diag_data = hypre_CSRMatrixData(A_diag);
   int             *A_diag_i = hypre_CSRMatrixI(A_diag);
   int             *A_diag_j = hypre_CSRMatrixJ(A_diag);

   hypre_CSRMatrix *A_offd = hypre_ParCSRMatrixOffd(A);
   
   double          *A_offd_data = hypre_CSRMatrixData(A_offd);
   int             *A_offd_i = hypre_CSRMatrixI(A_offd);
   int             *A_offd_j = hypre_CSRMatrixJ(A_offd);

   int	num_cols_diag_A = hypre_CSRMatrixNumCols(A_diag);
   int	num_cols_offd_A = hypre_CSRMatrixNumCols(A_offd);

   hypre_CSRMatrix *P_diag = hypre_ParCSRMatrixDiag(P);
   
   double          *P_diag_data = hypre_CSRMatrixData(P_diag);
   int             *P_diag_i = hypre_CSRMatrixI(P_diag);
   int             *P_diag_j = hypre_CSRMatrixJ(P_diag);

   hypre_CSRMatrix *P_offd = hypre_ParCSRMatrixOffd(P);
   int		   *col_map_offd_P = hypre_ParCSRMatrixColMapOffd(P);
   
   double          *P_offd_data = hypre_CSRMatrixData(P_offd);
   int             *P_offd_i = hypre_CSRMatrixI(P_offd);
   int             *P_offd_j = hypre_CSRMatrixJ(P_offd);

   int	first_col_diag_P = hypre_ParCSRMatrixFirstColDiag(P);
   int	last_col_diag_P;
   int	num_cols_diag_P = hypre_CSRMatrixNumCols(P_diag);
   int	num_cols_offd_P = hypre_CSRMatrixNumCols(P_offd);
   int *coarse_partitioning = hypre_ParCSRMatrixColStarts(P);

   hypre_ParCSRMatrix *RAP;
   int		      *col_map_offd_RAP;

   hypre_CSRMatrix *RAP_int;

   double          *RAP_int_data;
   int             *RAP_int_i;
   int             *RAP_int_j;

   hypre_CSRMatrix *RAP_ext;

   double          *RAP_ext_data;
   int             *RAP_ext_i;
   int             *RAP_ext_j;

   hypre_CSRMatrix *RAP_diag;

   double          *RAP_diag_data;
   int             *RAP_diag_i;
   int             *RAP_diag_j;

   hypre_CSRMatrix *RAP_offd;

   double          *RAP_offd_data;
   int             *RAP_offd_i;
   int             *RAP_offd_j;

   int              RAP_size;
   int              RAP_ext_size;
   int              RAP_diag_size;
   int              RAP_offd_size;
   int              P_ext_diag_size;
   int              P_ext_offd_size;
   int		    first_col_diag_RAP;
   int		    last_col_diag_RAP;
   int		    num_cols_offd_RAP = 0;
   
   hypre_CSRMatrix *R_diag;
   
   double          *R_diag_data;
   int             *R_diag_i;
   int             *R_diag_j;

   hypre_CSRMatrix *R_offd;
   
   double          *R_offd_data;
   int             *R_offd_i;
   int             *R_offd_j;

   hypre_CSRMatrix *Ps_ext;
   
   double          *Ps_ext_data;
   int             *Ps_ext_i;
   int             *Ps_ext_j;

   double 	   *P_ext_diag_data;
   int             *P_ext_diag_i;
   int             *P_ext_diag_j;

   double 	   *P_ext_offd_data;
   int             *P_ext_offd_i;
   int             *P_ext_offd_j;

   int		   *col_map_offd_Pext;
   int		   *map_P_to_Pext;
   int		   *map_P_to_RAP;
   int		   *map_Pext_to_RAP;

   int		   *P_marker;
   int		   *A_marker;
   int		   *temp;

   int              n_coarse;
   int              n_fine;
   int              num_cols_offd_Pext = 0;
   
   int              ic, i, j, k;
   int              i1, i2, i3;
   int              index, index2, cnt, cnt_offd, cnt_diag, value;
   int              jj1, jj2, jj3, jcol;
   
   int              jj_counter, jj_count_diag, jj_count_offd;
   int              jj_row_begining, jj_row_begin_diag, jj_row_begin_offd;
   int              start_indexing = 0; /* start indexing for RAP_data at 0 */
   int		    num_nz_cols_A;
   int		    count;
   int		    num_procs;

   double           r_entry;
   double           r_a_product;
   double           r_a_p_product;
   
   double           zero = 0.0;

   /*-----------------------------------------------------------------------
    *  Copy ParCSRMatrix RT into CSRMatrix R so that we have row-wise access 
    *  to restriction .
    *-----------------------------------------------------------------------*/

   MPI_Comm_size(comm,&num_procs);

   if (comm_pkg_RT)
   {
	num_recvs_RT = hypre_ParCSRCommPkgNumRecvs(comm_pkg_RT);
      	num_sends_RT = hypre_ParCSRCommPkgNumSends(comm_pkg_RT);
      	send_map_starts_RT =hypre_ParCSRCommPkgSendMapStarts(comm_pkg_RT);
      	send_map_elmts_RT = hypre_ParCSRCommPkgSendMapElmts(comm_pkg_RT);
   }

   hypre_CSRMatrixTranspose(RT_diag,&R_diag); 
   if (num_cols_offd_RT) 
   {
	hypre_CSRMatrixTranspose(RT_offd,&R_offd); 
	R_offd_data = hypre_CSRMatrixData(R_offd);
   	R_offd_i    = hypre_CSRMatrixI(R_offd);
   	R_offd_j    = hypre_CSRMatrixJ(R_offd);
   }

   /*-----------------------------------------------------------------------
    *  Access the CSR vectors for R. Also get sizes of fine and
    *  coarse grids.
    *-----------------------------------------------------------------------*/

   R_diag_data = hypre_CSRMatrixData(R_diag);
   R_diag_i    = hypre_CSRMatrixI(R_diag);
   R_diag_j    = hypre_CSRMatrixJ(R_diag);

   n_fine   = hypre_ParCSRMatrixGlobalNumRows(A);
   n_coarse = hypre_ParCSRMatrixGlobalNumCols(P);
   num_nz_cols_A = num_cols_diag_A + num_cols_offd_A;

   /*-----------------------------------------------------------------------
    *  Generate Ps_ext, i.e. portion of P that is stored on neighbor procs
    *  and needed locally for triple matrix product 
    *-----------------------------------------------------------------------*/

   if (num_procs > 1) 
   {
   	Ps_ext = hypre_ParCSRMatrixExtractBExt(P,A,1);
   	Ps_ext_data = hypre_CSRMatrixData(Ps_ext);
   	Ps_ext_i    = hypre_CSRMatrixI(Ps_ext);
   	Ps_ext_j    = hypre_CSRMatrixJ(Ps_ext);
   }

   P_ext_diag_i = hypre_CTAlloc(int,num_cols_offd_A+1);
   P_ext_offd_i = hypre_CTAlloc(int,num_cols_offd_A+1);
   P_ext_diag_size = 0;
   P_ext_offd_size = 0;
   last_col_diag_P = first_col_diag_P + num_cols_diag_P - 1;

   for (i=0; i < num_cols_offd_A; i++)
   {
      for (j=Ps_ext_i[i]; j < Ps_ext_i[i+1]; j++)
         if (Ps_ext_j[j] < first_col_diag_P || Ps_ext_j[j] > last_col_diag_P)
	    P_ext_offd_size++;
	 else
	    P_ext_diag_size++;
      P_ext_diag_i[i+1] = P_ext_diag_size;
      P_ext_offd_i[i+1] = P_ext_offd_size;
   }
   
   if (P_ext_diag_size)
   {
      P_ext_diag_j = hypre_CTAlloc(int, P_ext_diag_size);
      P_ext_diag_data = hypre_CTAlloc(double, P_ext_diag_size);
   }
   if (P_ext_offd_size)
   {
      P_ext_offd_j = hypre_CTAlloc(int, P_ext_offd_size);
      P_ext_offd_data = hypre_CTAlloc(double, P_ext_offd_size);
   }

   cnt_offd = 0;
   cnt_diag = 0;
   cnt = 0;
   for (i=0; i < num_cols_offd_A; i++)
   {
      for (j=Ps_ext_i[i]; j < Ps_ext_i[i+1]; j++)
         if (Ps_ext_j[j] < first_col_diag_P || Ps_ext_j[j] > last_col_diag_P)
         {
	    P_ext_offd_j[cnt_offd] = Ps_ext_j[j];
	    P_ext_offd_data[cnt_offd++] = Ps_ext_data[j];
         }
	 else
         {
	    P_ext_diag_j[cnt_diag] = Ps_ext_j[j] - first_col_diag_P;
	    P_ext_diag_data[cnt_diag++] = Ps_ext_data[j];
         }
   }
   if (num_procs > 1) hypre_CSRMatrixDestroy(Ps_ext);

   if (P_ext_offd_size || num_cols_offd_P)
   {
      temp = hypre_CTAlloc(int, P_ext_offd_size+num_cols_offd_P);
      for (i=0; i < P_ext_offd_size; i++)
         temp[i] = P_ext_offd_j[i];
      cnt = P_ext_offd_size;
      for (i=0; i < num_cols_offd_P; i++)
         temp[cnt++] = col_map_offd_P[i];
   }
   if (cnt)
   {
      qsort0(temp, 0, cnt-1);

      num_cols_offd_Pext = 1;
      value = temp[0];
      for (i=1; i < cnt; i++)
      {
         if (temp[i] > value)
         {
	    value = temp[i];
	    temp[num_cols_offd_Pext++] = value;
         }
      }
   }
 
   if (num_cols_offd_Pext)
	col_map_offd_Pext = hypre_CTAlloc(int,num_cols_offd_Pext);

   for (i=0; i < num_cols_offd_Pext; i++)
      col_map_offd_Pext[i] = temp[i];

   if (P_ext_offd_size || num_cols_offd_P)
      hypre_TFree(temp);

   for (i=0 ; i < P_ext_offd_size; i++)
      P_ext_offd_j[i] = hypre_BinarySearch(col_map_offd_Pext,
					   P_ext_offd_j[i],
					   num_cols_offd_Pext);
   if (num_cols_offd_P)
   {
      map_P_to_Pext = hypre_CTAlloc(int,num_cols_offd_P);

      cnt = 0;
      for (i=0; i < num_cols_offd_Pext; i++)
         if (col_map_offd_Pext[i] == col_map_offd_P[cnt])
         {
	    map_P_to_Pext[cnt++] = i;
	    if (cnt == num_cols_offd_P) break;
         }
   }

   /*-----------------------------------------------------------------------
    *  Allocate marker arrays.
    *-----------------------------------------------------------------------*/

   if (num_cols_offd_Pext || num_cols_diag_P)
      P_marker = hypre_CTAlloc(int, num_cols_diag_P+num_cols_offd_Pext);
   A_marker = hypre_CTAlloc(int, num_nz_cols_A);

   /*-----------------------------------------------------------------------
    *  First Pass: Determine size of RAP_int and set up RAP_int_i if there 
    *  are more than one processor and nonzero elements in R_offd
    *-----------------------------------------------------------------------*/

  if (num_cols_offd_RT)
  {
   RAP_int_i = hypre_CTAlloc(int, num_cols_offd_RT+1);
   
   /*-----------------------------------------------------------------------
    *  Initialize some stuff.
    *-----------------------------------------------------------------------*/

   jj_counter = start_indexing;
   for (ic = 0; ic < num_cols_diag_P+num_cols_offd_Pext; ic++)
   {      
      P_marker[ic] = -1;
   }
   for (i = 0; i < num_nz_cols_A; i++)
   {      
      A_marker[i] = -1;
   }   

   /*-----------------------------------------------------------------------
    *  Loop over exterior c-points
    *-----------------------------------------------------------------------*/
    
   for (ic = 0; ic < num_cols_offd_RT; ic++)
   {
      
      jj_row_begining = jj_counter;

      /*--------------------------------------------------------------------
       *  Loop over entries in row ic of R_offd.
       *--------------------------------------------------------------------*/
   
      for (jj1 = R_offd_i[ic]; jj1 < R_offd_i[ic+1]; jj1++)
      {
         i1  = R_offd_j[jj1];

         /*-----------------------------------------------------------------
          *  Loop over entries in row i1 of A_offd.
          *-----------------------------------------------------------------*/
         
         for (jj2 = A_offd_i[i1]; jj2 < A_offd_i[i1+1]; jj2++)
         {
            i2 = A_offd_j[jj2];

            /*--------------------------------------------------------------
             *  Check A_marker to see if point i2 has been previously
             *  visited. New entries in RAP only occur from unmarked points.
             *--------------------------------------------------------------*/

            if (A_marker[i2] != ic)
            {

               /*-----------------------------------------------------------
                *  Mark i2 as visited.
                *-----------------------------------------------------------*/

               A_marker[i2] = ic;
               
               /*-----------------------------------------------------------
                *  Loop over entries in row i2 of P_ext.
                *-----------------------------------------------------------*/

               for (jj3 = P_ext_diag_i[i2]; jj3 < P_ext_diag_i[i2+1]; jj3++)
               {
                  i3 = P_ext_diag_j[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, mark it and increment
                   *  counter.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begining)
                  {
                     P_marker[i3] = jj_counter;
                     jj_counter++;
                  }
               }
               for (jj3 = P_ext_offd_i[i2]; jj3 < P_ext_offd_i[i2+1]; jj3++)
               {
                  i3 = P_ext_offd_j[jj3] + num_cols_diag_P;
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, mark it and increment
                   *  counter.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begining)
                  {
                     P_marker[i3] = jj_counter;
                     jj_counter++;
                  }
               }
            }
         }
         /*-----------------------------------------------------------------
          *  Loop over entries in row i1 of A_diag.
          *-----------------------------------------------------------------*/
         
         for (jj2 = A_diag_i[i1]; jj2 < A_diag_i[i1+1]; jj2++)
         {
            i2 = A_diag_j[jj2];

            /*--------------------------------------------------------------
             *  Check A_marker to see if point i2 has been previously
             *  visited. New entries in RAP only occur from unmarked points.
             *--------------------------------------------------------------*/

            if (A_marker[i2+num_cols_offd_A] != ic)
            {

               /*-----------------------------------------------------------
                *  Mark i2 as visited.
                *-----------------------------------------------------------*/

               A_marker[i2+num_cols_offd_A] = ic;
               
               /*-----------------------------------------------------------
                *  Loop over entries in row i2 of P_diag.
                *-----------------------------------------------------------*/

               for (jj3 = P_diag_i[i2]; jj3 < P_diag_i[i2+1]; jj3++)
               {
                  i3 = P_diag_j[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, mark it and increment
                   *  counter.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begining)
                  {
                     P_marker[i3] = jj_counter;
                     jj_counter++;
                  }
               }
               /*-----------------------------------------------------------
                *  Loop over entries in row i2 of P_offd.
                *-----------------------------------------------------------*/

               for (jj3 = P_offd_i[i2]; jj3 < P_offd_i[i2+1]; jj3++)
               {
                  i3 = map_P_to_Pext[P_offd_j[jj3]] + num_cols_diag_P;
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, mark it and increment
                   *  counter.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begining)
                  {
                     P_marker[i3] = jj_counter;
                     jj_counter++;
                  }
               }
            }
         }
      }
            
      /*--------------------------------------------------------------------
       * Set RAP_int_i for this row.
       *--------------------------------------------------------------------*/

      RAP_int_i[ic] = jj_row_begining;
      
   }
  
   RAP_int_i[num_cols_offd_RT] = jj_counter;
 
   /*-----------------------------------------------------------------------
    *  Allocate RAP_int_data and RAP_int_j arrays.
    *-----------------------------------------------------------------------*/

   RAP_size = jj_counter;
   RAP_int_data = hypre_CTAlloc(double, RAP_size);
   RAP_int_j    = hypre_CTAlloc(int, RAP_size);

   /*-----------------------------------------------------------------------
    *  Second Pass: Fill in RAP_int_data and RAP_int_j.
    *-----------------------------------------------------------------------*/

   /*-----------------------------------------------------------------------
    *  Initialize some stuff.
    *-----------------------------------------------------------------------*/

   jj_counter = start_indexing;
   for (ic = 0; ic < num_cols_diag_P+num_cols_offd_Pext; ic++)
   {      
      P_marker[ic] = -1;
   }
   for (i = 0; i < num_nz_cols_A; i++)
   {      
      A_marker[i] = -1;
   }   
   
   /*-----------------------------------------------------------------------
    *  Loop over exterior c-points.
    *-----------------------------------------------------------------------*/
    
   for (ic = 0; ic < num_cols_offd_RT; ic++)
   {
      
      jj_row_begining = jj_counter;

      /*--------------------------------------------------------------------
       *  Loop over entries in row ic of R_offd.
       *--------------------------------------------------------------------*/
   
      for (jj1 = R_offd_i[ic]; jj1 < R_offd_i[ic+1]; jj1++)
      {
         i1  = R_offd_j[jj1];
         r_entry = R_offd_data[jj1];

         /*-----------------------------------------------------------------
          *  Loop over entries in row i1 of A_offd.
          *-----------------------------------------------------------------*/
         
         for (jj2 = A_offd_i[i1]; jj2 < A_offd_i[i1+1]; jj2++)
         {
            i2 = A_offd_j[jj2];
            r_a_product = r_entry * A_offd_data[jj2];
            
            /*--------------------------------------------------------------
             *  Check A_marker to see if point i2 has been previously
             *  visited. New entries in RAP only occur from unmarked points.
             *--------------------------------------------------------------*/

            if (A_marker[i2] != ic)
            {

               /*-----------------------------------------------------------
                *  Mark i2 as visited.
                *-----------------------------------------------------------*/

               A_marker[i2] = ic;
               
               /*-----------------------------------------------------------
                *  Loop over entries in row i2 of P_ext.
                *-----------------------------------------------------------*/

               for (jj3 = P_ext_diag_i[i2]; jj3 < P_ext_diag_i[i2+1]; jj3++)
               {
                  i3 = P_ext_diag_j[jj3];
                  r_a_p_product = r_a_product * P_ext_diag_data[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, create a new entry.
                   *  If it has, add new contribution.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begining)
                  {
                     P_marker[i3] = jj_counter;
                     RAP_int_data[jj_counter] = r_a_p_product;
                     RAP_int_j[jj_counter] = i3 + first_col_diag_P;
                     jj_counter++;
                  }
                  else
                  {
                     RAP_int_data[P_marker[i3]] += r_a_p_product;
                  }
               }

               for (jj3 = P_ext_offd_i[i2]; jj3 < P_ext_offd_i[i2+1]; jj3++)
               {
                  i3 = P_ext_offd_j[jj3] + num_cols_diag_P;
                  r_a_p_product = r_a_product * P_ext_offd_data[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, create a new entry.
                   *  If it has, add new contribution.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begining)
                  {
                     P_marker[i3] = jj_counter;
                     RAP_int_data[jj_counter] = r_a_p_product;
                     RAP_int_j[jj_counter] 
				= col_map_offd_Pext[i3-num_cols_diag_P];
                     jj_counter++;
                  }
                  else
                  {
                     RAP_int_data[P_marker[i3]] += r_a_p_product;
                  }
               }
            }

            /*--------------------------------------------------------------
             *  If i2 is previously visited ( A_marker[12]=ic ) it yields
             *  no new entries in RAP and can just add new contributions.
             *--------------------------------------------------------------*/

            else
            {
               for (jj3 = P_ext_diag_i[i2]; jj3 < P_ext_diag_i[i2+1]; jj3++)
               {
                  i3 = P_ext_diag_j[jj3];
                  r_a_p_product = r_a_product * P_ext_diag_data[jj3];
                  RAP_int_data[P_marker[i3]] += r_a_p_product;
               }
               for (jj3 = P_ext_offd_i[i2]; jj3 < P_ext_offd_i[i2+1]; jj3++)
               {
                  i3 = P_ext_offd_j[jj3] + num_cols_diag_P;
                  r_a_p_product = r_a_product * P_ext_offd_data[jj3];
                  RAP_int_data[P_marker[i3]] += r_a_p_product;
               }
            }
         }

         /*-----------------------------------------------------------------
          *  Loop over entries in row i1 of A_diag.
          *-----------------------------------------------------------------*/
         
         for (jj2 = A_diag_i[i1]; jj2 < A_diag_i[i1+1]; jj2++)
         {
            i2 = A_diag_j[jj2];
            r_a_product = r_entry * A_diag_data[jj2];
            
            /*--------------------------------------------------------------
             *  Check A_marker to see if point i2 has been previously
             *  visited. New entries in RAP only occur from unmarked points.
             *--------------------------------------------------------------*/

            if (A_marker[i2+num_cols_offd_A] != ic)
            {

               /*-----------------------------------------------------------
                *  Mark i2 as visited.
                *-----------------------------------------------------------*/

               A_marker[i2+num_cols_offd_A] = ic;
               
               /*-----------------------------------------------------------
                *  Loop over entries in row i2 of P_diag.
                *-----------------------------------------------------------*/

               for (jj3 = P_diag_i[i2]; jj3 < P_diag_i[i2+1]; jj3++)
               {
                  i3 = P_diag_j[jj3];
                  r_a_p_product = r_a_product * P_diag_data[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, create a new entry.
                   *  If it has, add new contribution.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begining)
                  {
                     P_marker[i3] = jj_counter;
                     RAP_int_data[jj_counter] = r_a_p_product;
                     RAP_int_j[jj_counter] = i3 + first_col_diag_P;
                     jj_counter++;
                  }
                  else
                  {
                     RAP_int_data[P_marker[i3]] += r_a_p_product;
                  }
               }
               for (jj3 = P_offd_i[i2]; jj3 < P_offd_i[i2+1]; jj3++)
               {
                  i3 = map_P_to_Pext[P_offd_j[jj3]] + num_cols_diag_P;
                  r_a_p_product = r_a_product * P_offd_data[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, create a new entry.
                   *  If it has, add new contribution.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begining)
                  {
                     P_marker[i3] = jj_counter;
                     RAP_int_data[jj_counter] = r_a_p_product;
                     RAP_int_j[jj_counter] = 
				col_map_offd_Pext[i3-num_cols_diag_P];
                     jj_counter++;
                  }
                  else
                  {
                     RAP_int_data[P_marker[i3]] += r_a_p_product;
                  }
               }
            }

            /*--------------------------------------------------------------
             *  If i2 is previously visited ( A_marker[12]=ic ) it yields
             *  no new entries in RAP and can just add new contributions.
             *--------------------------------------------------------------*/

            else
            {
               for (jj3 = P_diag_i[i2]; jj3 < P_diag_i[i2+1]; jj3++)
               {
                  i3 = P_diag_j[jj3];
                  r_a_p_product = r_a_product * P_diag_data[jj3];
                  RAP_int_data[P_marker[i3]] += r_a_p_product;
               }
               for (jj3 = P_offd_i[i2]; jj3 < P_offd_i[i2+1]; jj3++)
               {
                  i3 = map_P_to_Pext[P_offd_j[jj3]] + num_cols_diag_P;
                  r_a_p_product = r_a_product * P_offd_data[jj3];
                  RAP_int_data[P_marker[i3]] += r_a_p_product;
               }
            }
         }
      }
   }

   RAP_int = hypre_CSRMatrixCreate(num_cols_offd_RT,num_rows_offd_RT,RAP_size);
   hypre_CSRMatrixI(RAP_int) = RAP_int_i;
   hypre_CSRMatrixJ(RAP_int) = RAP_int_j;
   hypre_CSRMatrixData(RAP_int) = RAP_int_data;
  }

   RAP_ext_size = 0;
   if (num_sends_RT || num_recvs_RT)
   {
	RAP_ext = hypre_ExchangeRAPData(RAP_int,comm_pkg_RT);
   	RAP_ext_i = hypre_CSRMatrixI(RAP_ext);
   	RAP_ext_j = hypre_CSRMatrixJ(RAP_ext);
   	RAP_ext_data = hypre_CSRMatrixData(RAP_ext);
        RAP_ext_size = RAP_ext_i[hypre_CSRMatrixNumRows(RAP_ext)];
   }
   if (num_cols_offd_RT)
   	hypre_CSRMatrixDestroy(RAP_int);
 
   RAP_diag_i = hypre_CTAlloc(int, num_cols_diag_P+1);
   RAP_offd_i = hypre_CTAlloc(int, num_cols_diag_P+1);

   first_col_diag_RAP = first_col_diag_P;
   last_col_diag_RAP = first_col_diag_P + num_cols_diag_P - 1;

   /*-----------------------------------------------------------------------
    *  check for new nonzero columns in RAP_offd generated through RAP_ext
    *-----------------------------------------------------------------------*/

   if (RAP_ext_size || num_cols_offd_Pext)
   {
      temp = hypre_CTAlloc(int,RAP_ext_size+num_cols_offd_Pext);
      cnt = 0;
      for (i=0; i < RAP_ext_size; i++)
         if (RAP_ext_j[i] < first_col_diag_RAP 
			|| RAP_ext_j[i] > last_col_diag_RAP)
            temp[cnt++] = RAP_ext_j[i];
      for (i=0; i < num_cols_offd_Pext; i++)
         temp[cnt++] = col_map_offd_Pext[i];


      if (cnt)
      {
         qsort0(temp,0,cnt-1);
         value = temp[0];
         num_cols_offd_RAP = 1;
         for (i=1; i < cnt; i++)
         {
            if (temp[i] > value)
            {
	       value = temp[i];
	       temp[num_cols_offd_RAP++] = value;
            }
         }
      }

   /* now evaluate col_map_offd_RAP */
      if (num_cols_offd_RAP)
 	 col_map_offd_RAP = hypre_CTAlloc(int, num_cols_offd_RAP);

      for (i=0 ; i < num_cols_offd_RAP; i++)
	 col_map_offd_RAP[i] = temp[i];
  
      hypre_TFree(temp);
   }

   if (num_cols_offd_P)
   {
      map_P_to_RAP = hypre_CTAlloc(int,num_cols_offd_P);

      cnt = 0;
      for (i=0; i < num_cols_offd_RAP; i++)
         if (col_map_offd_RAP[i] == col_map_offd_P[cnt])
         {
	    map_P_to_RAP[cnt++] = i;
	    if (cnt == num_cols_offd_P) break;
         }
   }

   if (num_cols_offd_Pext)
   {
      map_Pext_to_RAP = hypre_CTAlloc(int,num_cols_offd_Pext);

      cnt = 0;
      for (i=0; i < num_cols_offd_RAP; i++)
         if (col_map_offd_RAP[i] == col_map_offd_Pext[cnt])
         {
	    map_Pext_to_RAP[cnt++] = i;
	    if (cnt == num_cols_offd_Pext) break;
         }
   }

/*   need to allocate new P_marker etc. and make further changes */
   /*-----------------------------------------------------------------------
    *  Initialize some stuff.
    *-----------------------------------------------------------------------*/
   if (num_cols_offd_Pext || num_cols_diag_P)
      hypre_TFree(P_marker);
   P_marker = hypre_CTAlloc(int, num_cols_diag_P+num_cols_offd_RAP);

   jj_count_diag = start_indexing;
   jj_count_offd = start_indexing;
   for (ic = 0; ic < num_cols_diag_P+num_cols_offd_RAP; ic++)
   {      
      P_marker[ic] = -1;
   }
   for (i = 0; i < num_nz_cols_A; i++)
   {      
      A_marker[i] = -1;
   }   

   /*-----------------------------------------------------------------------
    *  Loop over interior c-points.
    *-----------------------------------------------------------------------*/
   
   for (ic = 0; ic < num_cols_diag_P; ic++)
   {
      
      /*--------------------------------------------------------------------
       *  Set marker for diagonal entry, RAP_{ic,ic}. and for all points
       *  being added to row ic of RAP_diag and RAP_offd through RAP_ext
       *--------------------------------------------------------------------*/
 
      P_marker[ic] = jj_count_diag;
      jj_row_begin_diag = jj_count_diag;
      jj_row_begin_offd = jj_count_offd;
      jj_count_diag++;

      for (i=0; i < num_sends_RT; i++)
	for (j = send_map_starts_RT[i]; j < send_map_starts_RT[i+1]; j++)
	    if (send_map_elmts_RT[j] == ic)
	    {
		for (k=RAP_ext_i[j]; k < RAP_ext_i[j+1]; k++)
		{
		   jcol = RAP_ext_j[k];
		   if (jcol < first_col_diag_RAP || jcol > last_col_diag_RAP)
		   {
		   	jcol = num_cols_diag_P 
				+ hypre_BinarySearch(col_map_offd_RAP,
						jcol,num_cols_offd_RAP);
		   	if (P_marker[jcol] < jj_row_begin_offd)
			{
			 	P_marker[jcol] = jj_count_offd;
				jj_count_offd++;
			}
		   }
		   else
		   {
		   	jcol -= first_col_diag_P;
		   	if (P_marker[jcol] < jj_row_begin_diag)
			{
			 	P_marker[jcol] = jj_count_diag;
				jj_count_diag++;
			}
		   }
	    	}
		break;
	    }
 
      /*--------------------------------------------------------------------
       *  Loop over entries in row ic of R_diag.
       *--------------------------------------------------------------------*/
   
      for (jj1 = R_diag_i[ic]; jj1 < R_diag_i[ic+1]; jj1++)
      {
         i1  = R_diag_j[jj1];
 
         /*-----------------------------------------------------------------
          *  Loop over entries in row i1 of A_offd.
          *-----------------------------------------------------------------*/
         
	 if (num_cols_offd_A)
	 {
           for (jj2 = A_offd_i[i1]; jj2 < A_offd_i[i1+1]; jj2++)
           {
            i2 = A_offd_j[jj2];
 
            /*--------------------------------------------------------------
             *  Check A_marker to see if point i2 has been previously
             *  visited. New entries in RAP only occur from unmarked points.
             *--------------------------------------------------------------*/
 
            if (A_marker[i2] != ic)
            {
 
               /*-----------------------------------------------------------
                *  Mark i2 as visited.
                *-----------------------------------------------------------*/
 
               A_marker[i2] = ic;
               
               /*-----------------------------------------------------------
                *  Loop over entries in row i2 of P_ext.
                *-----------------------------------------------------------*/
 
               for (jj3 = P_ext_diag_i[i2]; jj3 < P_ext_diag_i[i2+1]; jj3++)
               {
                  i3 = P_ext_diag_j[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, mark it and increment
                   *  counter.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begin_diag)
                  {
                     P_marker[i3] = jj_count_diag;
                     jj_count_diag++;
                  }
               }
               for (jj3 = P_ext_offd_i[i2]; jj3 < P_ext_offd_i[i2+1]; jj3++)
               {
                  i3 = map_Pext_to_RAP[P_ext_offd_j[jj3]]+num_cols_diag_P;
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, mark it and increment
                   *  counter.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begin_offd)
                  {
                     P_marker[i3] = jj_count_offd;
                     jj_count_offd++;
                  }
               }
            }
           }
         }
         /*-----------------------------------------------------------------
          *  Loop over entries in row i1 of A_diag.
          *-----------------------------------------------------------------*/
         
         for (jj2 = A_diag_i[i1]; jj2 < A_diag_i[i1+1]; jj2++)
         {
            i2 = A_diag_j[jj2];
 
            /*--------------------------------------------------------------
             *  Check A_marker to see if point i2 has been previously
             *  visited. New entries in RAP only occur from unmarked points.
             *--------------------------------------------------------------*/
 
            if (A_marker[i2+num_cols_offd_A] != ic)
            {
 
               /*-----------------------------------------------------------
                *  Mark i2 as visited.
                *-----------------------------------------------------------*/
 
               A_marker[i2+num_cols_offd_A] = ic;
               
               /*-----------------------------------------------------------
                *  Loop over entries in row i2 of P_diag.
                *-----------------------------------------------------------*/
 
               for (jj3 = P_diag_i[i2]; jj3 < P_diag_i[i2+1]; jj3++)
               {
                  i3 = P_diag_j[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, mark it and increment
                   *  counter.
                   *--------------------------------------------------------*/
 
                  if (P_marker[i3] < jj_row_begin_diag)
                  {
                     P_marker[i3] = jj_count_diag;
                     jj_count_diag++;
                  }
               }
               /*-----------------------------------------------------------
                *  Loop over entries in row i2 of P_offd.
                *-----------------------------------------------------------*/

	       if (num_cols_offd_P)
	       { 
                 for (jj3 = P_offd_i[i2]; jj3 < P_offd_i[i2+1]; jj3++)
                 {
                  i3 = map_P_to_RAP[P_offd_j[jj3]] + num_cols_diag_P;
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, mark it and increment
                   *  counter.
                   *--------------------------------------------------------*/
 
                  if (P_marker[i3] < jj_row_begin_offd)
                  {
                     P_marker[i3] = jj_count_offd;
                     jj_count_offd++;
                  }
                 }
	       } 
            }
         }
      }
            
      /*--------------------------------------------------------------------
       * Set RAP_diag_i and RAP_offd_i for this row.
       *--------------------------------------------------------------------*/
 
      RAP_diag_i[ic] = jj_row_begin_diag;
      RAP_offd_i[ic] = jj_row_begin_offd;
      
   }
  
   RAP_diag_i[num_cols_diag_P] = jj_count_diag;
   RAP_offd_i[num_cols_diag_P] = jj_count_offd;
 
   /*-----------------------------------------------------------------------
    *  Allocate RAP_diag_data and RAP_diag_j arrays.
    *  Allocate RAP_offd_data and RAP_offd_j arrays.
    *-----------------------------------------------------------------------*/
 
   RAP_diag_size = jj_count_diag;
   if (RAP_diag_size)
   { 
      RAP_diag_data = hypre_CTAlloc(double, RAP_diag_size);
      RAP_diag_j    = hypre_CTAlloc(int, RAP_diag_size);
   } 
 
   RAP_offd_size = jj_count_offd;
   if (RAP_offd_size)
   { 
   	RAP_offd_data = hypre_CTAlloc(double, RAP_offd_size);
   	RAP_offd_j    = hypre_CTAlloc(int, RAP_offd_size);
   } 

   /*-----------------------------------------------------------------------
    *  Second Pass: Fill in RAP_diag_data and RAP_diag_j.
    *  Second Pass: Fill in RAP_offd_data and RAP_offd_j.
    *-----------------------------------------------------------------------*/

   /*-----------------------------------------------------------------------
    *  Initialize some stuff.
    *-----------------------------------------------------------------------*/

   jj_count_diag = start_indexing;
   jj_count_offd = start_indexing;
   for (ic = 0; ic < num_cols_diag_P+num_cols_offd_RAP; ic++)
   {      
      P_marker[ic] = -1;
   }
   for (i = 0; i < num_nz_cols_A ; i++)
   {      
      A_marker[i] = -1;
   }   
   
   /*-----------------------------------------------------------------------
    *  Loop over interior c-points.
    *-----------------------------------------------------------------------*/
    
   for (ic = 0; ic < num_cols_diag_P; ic++)
   {
      
      /*--------------------------------------------------------------------
       *  Create diagonal entry, RAP_{ic,ic} and add entries of RAP_ext 
       *--------------------------------------------------------------------*/

      P_marker[ic] = jj_count_diag;
      jj_row_begin_diag = jj_count_diag;
      jj_row_begin_offd = jj_count_offd;
      RAP_diag_data[jj_count_diag] = zero;
      RAP_diag_j[jj_count_diag] = ic;
      jj_count_diag++;

      for (i=0; i < num_sends_RT; i++)
	for (j = send_map_starts_RT[i]; j < send_map_starts_RT[i+1]; j++)
	    if (send_map_elmts_RT[j] == ic)
	    {
		for (k=RAP_ext_i[j]; k < RAP_ext_i[j+1]; k++)
		{
		   jcol = RAP_ext_j[k];
		   if (jcol < first_col_diag_RAP || jcol > last_col_diag_RAP)
		   {
		   	jcol = num_cols_diag_P 
				+ hypre_BinarySearch(col_map_offd_RAP,
						jcol,num_cols_offd_RAP);
		   	if (P_marker[jcol] < jj_row_begin_offd)
			{
			 	P_marker[jcol] = jj_count_offd;
				RAP_offd_data[jj_count_offd] 
					= RAP_ext_data[k];
				RAP_offd_j[jj_count_offd] 
					= jcol-num_cols_diag_P;
				jj_count_offd++;
			}
			else
				RAP_offd_data[P_marker[jcol]]
					+= RAP_ext_data[k];
		   }
		   else
		   {
		   	jcol -= first_col_diag_P; 
		   	if (P_marker[jcol] < jj_row_begin_diag)
			{
			 	P_marker[jcol] = jj_count_diag;
				RAP_diag_data[jj_count_diag] 
					= RAP_ext_data[k];
				RAP_diag_j[jj_count_diag] = jcol;
				jj_count_diag++;
			}
			else
				RAP_diag_data[P_marker[jcol]]
					+= RAP_ext_data[k];
		   }
	    	}
		break;
	    }
 
      /*--------------------------------------------------------------------
       *  Loop over entries in row ic of R_diag.
       *--------------------------------------------------------------------*/

      for (jj1 = R_diag_i[ic]; jj1 < R_diag_i[ic+1]; jj1++)
      {
         i1  = R_diag_j[jj1];
         r_entry = R_diag_data[jj1];

         /*-----------------------------------------------------------------
          *  Loop over entries in row i1 of A_offd.
          *-----------------------------------------------------------------*/
         
	 if (num_cols_offd_A)
	 {
	  for (jj2 = A_offd_i[i1]; jj2 < A_offd_i[i1+1]; jj2++)
          {
            i2 = A_offd_j[jj2];
            r_a_product = r_entry * A_offd_data[jj2];
            
            /*--------------------------------------------------------------
             *  Check A_marker to see if point i2 has been previously
             *  visited. New entries in RAP only occur from unmarked points.
             *--------------------------------------------------------------*/

            if (A_marker[i2] != ic)
            {

               /*-----------------------------------------------------------
                *  Mark i2 as visited.
                *-----------------------------------------------------------*/

               A_marker[i2] = ic;
               
               /*-----------------------------------------------------------
                *  Loop over entries in row i2 of P_ext.
                *-----------------------------------------------------------*/

               for (jj3 = P_ext_diag_i[i2]; jj3 < P_ext_diag_i[i2+1]; jj3++)
               {
                  i3 = P_ext_diag_j[jj3];
                  r_a_p_product = r_a_product * P_ext_diag_data[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, create a new entry.
                   *  If it has, add new contribution.
                   *--------------------------------------------------------*/
                  if (P_marker[i3] < jj_row_begin_diag)
                  {
                    	P_marker[i3] = jj_count_diag;
                     	RAP_diag_data[jj_count_diag] = r_a_p_product;
                     	RAP_diag_j[jj_count_diag] = i3;
                     	jj_count_diag++;
		  }
		  else
                     	RAP_diag_data[P_marker[i3]] += r_a_p_product;
               }
               for (jj3 = P_ext_offd_i[i2]; jj3 < P_ext_offd_i[i2+1]; jj3++)
               {
                  i3 = map_Pext_to_RAP[P_ext_offd_j[jj3]] + num_cols_diag_P;
                  r_a_p_product = r_a_product * P_ext_offd_data[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, create a new entry.
                   *  If it has, add new contribution.
                   *--------------------------------------------------------*/
                  if (P_marker[i3] < jj_row_begin_offd)
                  {
                     	P_marker[i3] = jj_count_offd;
                     	RAP_offd_data[jj_count_offd] = r_a_p_product;
                     	RAP_offd_j[jj_count_offd] = i3 - num_cols_diag_P;
                     	jj_count_offd++;
	 	  }
		  else
                     	RAP_offd_data[P_marker[i3]] += r_a_p_product;
               }
            }

            /*--------------------------------------------------------------
             *  If i2 is previously visited ( A_marker[12]=ic ) it yields
             *  no new entries in RAP and can just add new contributions.
             *--------------------------------------------------------------*/
            else
            {
               for (jj3 = P_ext_diag_i[i2]; jj3 < P_ext_diag_i[i2+1]; jj3++)
               {
                  i3 = P_ext_diag_j[jj3];
                  r_a_p_product = r_a_product * P_ext_diag_data[jj3];
                  RAP_diag_data[P_marker[i3]] += r_a_p_product;
               }
               for (jj3 = P_ext_offd_i[i2]; jj3 < P_ext_offd_i[i2+1]; jj3++)
               {
                  i3 = map_Pext_to_RAP[P_ext_offd_j[jj3]] + num_cols_diag_P;
                  r_a_p_product = r_a_product * P_ext_offd_data[jj3];
                  RAP_offd_data[P_marker[i3]] += r_a_p_product;
               }
            }
          }
         }

         /*-----------------------------------------------------------------
          *  Loop over entries in row i1 of A_diag.
          *-----------------------------------------------------------------*/
         
         for (jj2 = A_diag_i[i1]; jj2 < A_diag_i[i1+1]; jj2++)
         {
            i2 = A_diag_j[jj2];
            r_a_product = r_entry * A_diag_data[jj2];
            
            /*--------------------------------------------------------------
             *  Check A_marker to see if point i2 has been previously
             *  visited. New entries in RAP only occur from unmarked points.
             *--------------------------------------------------------------*/

            if (A_marker[i2+num_cols_offd_A] != ic)
            {

               /*-----------------------------------------------------------
                *  Mark i2 as visited.
                *-----------------------------------------------------------*/

               A_marker[i2+num_cols_offd_A] = ic;
               
               /*-----------------------------------------------------------
                *  Loop over entries in row i2 of P_diag.
                *-----------------------------------------------------------*/

               for (jj3 = P_diag_i[i2]; jj3 < P_diag_i[i2+1]; jj3++)
               {
                  i3 = P_diag_j[jj3];
                  r_a_p_product = r_a_product * P_diag_data[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, create a new entry.
                   *  If it has, add new contribution.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begin_diag)
                  {
                     P_marker[i3] = jj_count_diag;
                     RAP_diag_data[jj_count_diag] = r_a_p_product;
                     RAP_diag_j[jj_count_diag] = P_diag_j[jj3];
                     jj_count_diag++;
                  }
                  else
                  {
                     RAP_diag_data[P_marker[i3]] += r_a_p_product;
                  }
               }
               if (num_cols_offd_P)
	       {
		for (jj3 = P_offd_i[i2]; jj3 < P_offd_i[i2+1]; jj3++)
                {
                  i3 = map_P_to_RAP[P_offd_j[jj3]] + num_cols_diag_P;
                  r_a_p_product = r_a_product * P_offd_data[jj3];
                  
                  /*--------------------------------------------------------
                   *  Check P_marker to see that RAP_{ic,i3} has not already
                   *  been accounted for. If it has not, create a new entry.
                   *  If it has, add new contribution.
                   *--------------------------------------------------------*/

                  if (P_marker[i3] < jj_row_begin_offd)
                  {
                     P_marker[i3] = jj_count_offd;
                     RAP_offd_data[jj_count_offd] = r_a_p_product;
                     RAP_offd_j[jj_count_offd] = i3 - num_cols_diag_P;
                     jj_count_offd++;
                  }
                  else
                  {
                     RAP_offd_data[P_marker[i3]] += r_a_p_product;
                  }
                }
               }
            }

            /*--------------------------------------------------------------
             *  If i2 is previously visited ( A_marker[12]=ic ) it yields
             *  no new entries in RAP and can just add new contributions.
             *--------------------------------------------------------------*/

            else
            {
               for (jj3 = P_diag_i[i2]; jj3 < P_diag_i[i2+1]; jj3++)
               {
                  i3 = P_diag_j[jj3];
                  r_a_p_product = r_a_product * P_diag_data[jj3];
                  RAP_diag_data[P_marker[i3]] += r_a_p_product;
               }
	       if (num_cols_offd_P)
	       {
                for (jj3 = P_offd_i[i2]; jj3 < P_offd_i[i2+1]; jj3++)
                {
                  i3 = map_P_to_RAP[P_offd_j[jj3]] + num_cols_diag_P;
                  r_a_p_product = r_a_product * P_offd_data[jj3];
                  RAP_offd_data[P_marker[i3]] += r_a_p_product;
                }
               }
            }
         }
      }
   }

   RAP = hypre_ParCSRMatrixCreate(comm, n_coarse, n_coarse, 
	coarse_partitioning, coarse_partitioning,
	num_cols_offd_RAP, RAP_diag_size, RAP_offd_size);

/* Have RAP own coarse_partitioning instead of P */
   hypre_ParCSRMatrixSetColStartsOwner(P,0);

   RAP_diag = hypre_ParCSRMatrixDiag(RAP);
   hypre_CSRMatrixData(RAP_diag) = RAP_diag_data; 
   hypre_CSRMatrixI(RAP_diag) = RAP_diag_i; 
   hypre_CSRMatrixJ(RAP_diag) = RAP_diag_j; 

   RAP_offd = hypre_ParCSRMatrixOffd(RAP);
   hypre_CSRMatrixI(RAP_offd) = RAP_offd_i; 
   if (num_cols_offd_RAP)
   {
	hypre_CSRMatrixData(RAP_offd) = RAP_offd_data; 
   	hypre_CSRMatrixJ(RAP_offd) = RAP_offd_j; 
   	hypre_ParCSRMatrixColMapOffd(RAP) = col_map_offd_RAP;
   }
   if (num_procs > 1)
   {
   	/* hypre_GenerateRAPCommPkg(RAP, A); */
   	hypre_MatvecCommPkgCreate(RAP); 
   }

   *RAP_ptr = RAP;

   /*-----------------------------------------------------------------------
    *  Free R, P_ext and marker arrays.
    *-----------------------------------------------------------------------*/

   hypre_CSRMatrixDestroy(R_diag);
   if (num_cols_offd_RT) 
	hypre_CSRMatrixDestroy(R_offd);

   if (num_sends_RT || num_recvs_RT) 
	hypre_CSRMatrixDestroy(RAP_ext);

   hypre_TFree(P_marker);   
   hypre_TFree(A_marker);
   hypre_TFree(P_ext_diag_i);
   hypre_TFree(P_ext_offd_i);
   if (num_cols_offd_P)
   {
      hypre_TFree(map_P_to_Pext);
      hypre_TFree(map_P_to_RAP);
   }
   if (num_cols_offd_Pext)
   {
      hypre_TFree(col_map_offd_Pext);
      hypre_TFree(map_Pext_to_RAP);
   }
   if (P_ext_diag_size)
   {
      hypre_TFree(P_ext_diag_data);
      hypre_TFree(P_ext_diag_j);
   }
   if (P_ext_offd_size)
   {
      hypre_TFree(P_ext_offd_data);
      hypre_TFree(P_ext_offd_j);
   }
   return(0);
   
}            




             
/*--------------------------------------------------------------------------
 * OLD NOTES:
 * Sketch of John's code to build RAP
 *
 * Uses two integer arrays icg and ifg as marker arrays
 *
 *  icg needs to be of size n_fine; size of ia.
 *     A negative value of icg(i) indicates i is a f-point, otherwise
 *     icg(i) is the converts from fine to coarse grid orderings. 
 *     Note that I belive the code assumes that if i<j and both are
 *     c-points, then icg(i) < icg(j).
 *  ifg needs to be of size n_coarse; size of irap
 *     I don't think it has meaning as either input or output.
 *
 * In the code, both the interpolation and restriction operator
 * are stored row-wise in the array b. If i is a f-point,
 * ib(i) points the row of the interpolation operator for point
 * i. If i is a c-point, ib(i) points the row of the restriction
 * operator for point i.
 *
 * In the CSR storage for rap, its guaranteed that the rows will
 * be ordered ( i.e. ic<jc -> irap(ic) < irap(jc)) but I don't
 * think there is a guarantee that the entries within a row will
 * be ordered in any way except that the diagonal entry comes first.
 *
 * As structured now, the code requires that the size of rap be
 * predicted up front. To avoid this, one could execute the code
 * twice, the first time would only keep track of icg ,ifg and ka.
 * Then you would know how much memory to allocate for rap and jrap.
 * The second time would fill in these arrays. Actually you might
 * be able to include the filling in of jrap into the first pass;
 * just overestimate its size (its an integer array) and cut it
 * back before the second time through. This would avoid some if tests
 * in the second pass.
 *
 * Questions
 *            1) parallel (PetSc) version?
 *            2) what if we don't store R row-wise and don't
 *               even want to store a copy of it in this form
 *               temporarily? 
 *--------------------------------------------------------------------------*/
         
hypre_CSRMatrix *
hypre_ExchangeRAPData( 	hypre_CSRMatrix *RAP_int,
			hypre_ParCSRCommPkg *comm_pkg_RT)
{
   int     *RAP_int_i;
   int     *RAP_int_j;
   double  *RAP_int_data;
   int     num_cols = 0;

   MPI_Comm comm = hypre_ParCSRCommPkgComm(comm_pkg_RT);
   int num_recvs = hypre_ParCSRCommPkgNumRecvs(comm_pkg_RT);
   int *recv_procs = hypre_ParCSRCommPkgRecvProcs(comm_pkg_RT);
   int *recv_vec_starts = hypre_ParCSRCommPkgRecvVecStarts(comm_pkg_RT);
   int num_sends = hypre_ParCSRCommPkgNumSends(comm_pkg_RT);
   int *send_procs = hypre_ParCSRCommPkgSendProcs(comm_pkg_RT);
   int *send_map_starts = hypre_ParCSRCommPkgSendMapStarts(comm_pkg_RT);

   hypre_CSRMatrix *RAP_ext;

   int	   *RAP_ext_i;
   int	   *RAP_ext_j;
   double  *RAP_ext_data;

   MPI_Datatype *recv_matrix_types;
   MPI_Datatype *send_matrix_types;
   hypre_ParCSRCommHandle *comm_handle;
   hypre_ParCSRCommPkg *tmp_comm_pkg;

   int num_rows;
   int num_nonzeros;
   int start_index;
   int i, j;
   int num_procs, my_id;

   MPI_Comm_size(comm,&num_procs);
   MPI_Comm_rank(comm,&my_id);

   send_matrix_types = hypre_CTAlloc(MPI_Datatype, num_sends);
   recv_matrix_types = hypre_CTAlloc(MPI_Datatype, num_recvs);
 
   RAP_ext_i = hypre_CTAlloc(int, send_map_starts[num_sends]+1);
 
/*--------------------------------------------------------------------------
 * recompute RAP_int_i so that RAP_int_i[j+1] contains the number of
 * elements of row j (to be determined through send_map_elmnts on the
 * receiving end0
 *--------------------------------------------------------------------------*/

   if (num_recvs)
   {
    	RAP_int_i = hypre_CSRMatrixI(RAP_int);
     	RAP_int_j = hypre_CSRMatrixJ(RAP_int);
   	RAP_int_data = hypre_CSRMatrixData(RAP_int);
   	num_cols = hypre_CSRMatrixNumCols(RAP_int);
   }

   for (i=0; i < num_recvs; i++)
   {
	start_index = RAP_int_i[recv_vec_starts[i]];
	num_nonzeros = RAP_int_i[recv_vec_starts[i+1]]-start_index;
	hypre_BuildCSRJDataType(num_nonzeros, 
			  &RAP_int_data[start_index], 
			  &RAP_int_j[start_index], 
			  &recv_matrix_types[i]);	
   }
 
   for (i=num_recvs; i > 0; i--)
	for (j = recv_vec_starts[i]; j > recv_vec_starts[i-1]; j--)
		RAP_int_i[j] -= RAP_int_i[j-1];

/*--------------------------------------------------------------------------
 * initialize communication 
 *--------------------------------------------------------------------------*/
   comm_handle = hypre_InitializeCommunication(12,comm_pkg_RT,
		&RAP_int_i[1], &RAP_ext_i[1]);

   tmp_comm_pkg = hypre_CTAlloc(hypre_ParCSRCommPkg, 1);
   hypre_ParCSRCommPkgComm(tmp_comm_pkg) = comm;
   hypre_ParCSRCommPkgNumSends(tmp_comm_pkg) = num_recvs;
   hypre_ParCSRCommPkgNumRecvs(tmp_comm_pkg) = num_sends;
   hypre_ParCSRCommPkgSendProcs(tmp_comm_pkg) = recv_procs;
   hypre_ParCSRCommPkgRecvProcs(tmp_comm_pkg) = send_procs;
   hypre_ParCSRCommPkgSendMPITypes(tmp_comm_pkg) = recv_matrix_types;	

   hypre_FinalizeCommunication(comm_handle);

/*--------------------------------------------------------------------------
 * compute num_nonzeros for RAP_ext
 *--------------------------------------------------------------------------*/

   for (i=0; i < num_sends; i++)
	for (j = send_map_starts[i]; j < send_map_starts[i+1]; j++)
		RAP_ext_i[j+1] += RAP_ext_i[j];

   num_rows = send_map_starts[num_sends];
   num_nonzeros = RAP_ext_i[num_rows];
   if (num_nonzeros)
   {
      RAP_ext_j = hypre_CTAlloc(int, num_nonzeros);
      RAP_ext_data = hypre_CTAlloc(double, num_nonzeros);
   }

   for (i=0; i < num_sends; i++)
   {
	start_index = RAP_ext_i[send_map_starts[i]];
	num_nonzeros = RAP_ext_i[send_map_starts[i+1]]-start_index;
	hypre_BuildCSRJDataType(num_nonzeros, 
			  &RAP_ext_data[start_index], 
			  &RAP_ext_j[start_index], 
			  &send_matrix_types[i]);	
   }

   hypre_ParCSRCommPkgRecvMPITypes(tmp_comm_pkg) = send_matrix_types;	

   comm_handle = hypre_InitializeCommunication(0,tmp_comm_pkg,NULL,NULL);

   RAP_ext = hypre_CSRMatrixCreate(num_rows,num_cols,num_nonzeros);

   hypre_CSRMatrixI(RAP_ext) = RAP_ext_i;
   if (num_nonzeros)
   {
      hypre_CSRMatrixJ(RAP_ext) = RAP_ext_j;
      hypre_CSRMatrixData(RAP_ext) = RAP_ext_data;
   }

   hypre_FinalizeCommunication(comm_handle); 

   for (i=0; i < num_sends; i++)
   {
	MPI_Type_free(&send_matrix_types[i]);
   }

   for (i=0; i < num_recvs; i++)
   {
	MPI_Type_free(&recv_matrix_types[i]);
   }

   hypre_TFree(tmp_comm_pkg);
   hypre_TFree(recv_matrix_types);
   hypre_TFree(send_matrix_types);

   return RAP_ext;
}
