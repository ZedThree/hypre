# Copyright 1998-2019 Lawrence Livermore National Security, LLC and other
# HYPRE Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

set(HDRS
  par_csr_block_matrix.h
  csr_block_matrix.h
)

set(SRCS
  csr_block_matrix.c
  csr_block_matvec.c
  par_csr_block_matrix.c
  par_csr_block_matvec.c
  par_csr_block_comm.c
  par_csr_block_rap.c
  par_csr_block_rap_communication.c
  par_csr_block_interp.c
  par_csr_block_relax.c
  par_block_nodal_systems.c
)
  
convert_filenames_to_full_paths(HDRS)
convert_filenames_to_full_paths(SRCS)

set(HYPRE_HEADERS ${HYPRE_HEADERS} ${HDRS} PARENT_SCOPE)
set(HYPRE_SOURCES ${HYPRE_SOURCES} ${SRCS} PARENT_SCOPE)


