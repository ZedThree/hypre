#ifndef GSELIM_H
#define GSELIM_H

#define hypre_gselim_inline(A,x,n)\
{				  \
   HYPRE_Int    j,k,m;		  \
   HYPRE_Real factor;		  \
   HYPRE_Real divA;		  \
   if (n==1)  /* A is 1x1 */	  \
   {				  \
      if (A[0] != 0.0)		  \
      {				  \
	 x[0] = x[0]/A[0];	  \
      }				  \
   }				  \
   else/* A is nxn. Forward elimination */\
   {					  \
      for (k = 0; k < n-1; k++)		  \
      {  				  \
  	 if (A[k*n+k] != 0.0)		  \
	 {				  \
            divA = 1.0/A[k*n+k];	  \
	    for (j = k+1; j < n; j++)	  \
            {				  \
               if (A[j*n+k] != 0.0)	  \
	       {	       		  \
		  factor = A[j*n+k]*divA; \
		  for (m = k+1; m < n; m++)	\
                  {				\
		    A[j*n+m]  -= factor * A[k*n+m];	\
		  }				   \
		  x[j] -= factor * x[k];	   \
	       }				   \
	    }					   \
	 }					   \
      }						  \
      /* Back Substitution  */\
      for (k = n-1; k > 0; --k)			\
      {					\
	 if (A[k*n+k] != 0.0)		\
	 {				\
            x[k] /= A[k*n+k];		\
	    for (j = 0; j < k; j++)	\
            {				\
	       if (A[j*n+k] != 0.0)		\
               {				\
		  x[j] -= x[k] * A[j*n+k];	\
	       }				\
	    }					\
	 }					\
      }						\
      if (A[0] != 0.0) x[0] /= A[0];		\
   }						\
}

#endif
