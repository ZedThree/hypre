#BHEADER***********************************************************************
# (c) 1998   The Regents of the University of California
#
# See the file COPYRIGHT_and_DISCLAIMER for a complete copyright
# notice, contact person, and disclaimer.
#
# $Revision$
#EHEADER***********************************************************************

# Include all variables defined by configure
include ${HYPRE_SRC_TOP_DIR}/config/Makefile.config


# These are the directories for internal blas, lapack and general utilities
HYPRE_BASIC_DIRS = \
  blas\
  lapack\
  utilities

# These are the directories for the generic Krylov solvers
HYPRE_KRYLOV_DIRS = krylov

#These are the directories for the structured interface
HYPRE_STRUCT_DIRS =\
 struct_mv\
 struct_ls

#These are the directories for the semi-structured interface
HYPRE_SSTRUCT_DIRS =\
 sstruct_mv\
 sstruct_ls

#These are the directories for the IJ interface
HYPRE_IJ_DIRS =\
 seq_mv\
 parcsr_mv\
 distributed_matrix\
 matrix_matrix\
 IJ_mv\
 distributed_ls\
 parcsr_ls

#These are the directories for eigensolvers
HYPRE_EIGEN_DIRS = eigensolvers

#These are the directories for the FEI
HYPRE_FEI_DIRS = FEI_mv

#This is the lib directory
HYPRE_LIBS_DIRS = lib

#This is the documentation directory
HYPRE_DOCS_DIRS = docs

#This is the test-driver directory
HYPRE_TEST_DIRS = test

# These are directories that are officially in HYPRE
HYPRE_DIRS =\
 ${HYPRE_BASIC_DIRS}\
 ${HYPRE_KRYLOV_DIRS}\
 ${HYPRE_STRUCT_DIRS}\
 ${HYPRE_SSTRUCT_DIRS}\
 ${HYPRE_IJ_DIRS}\
 ${HYPRE_EIGEN_DIRS}\
 ${HYPRE_FEI_DIRS}\
 ${HYPRE_LIBS_DIRS}

# These are directories that are not yet officially in HYPRE
HYPRE_EXTRA_DIRS =\
 ${HYPRE_DOCS_DIRS}\
 ${HYPRE_TEST_DIRS}\
 seq_ls 

#################################################################
# Targets
#################################################################

all:
	@ \
	mkdir -p ${HYPRE_BUILD_DIR}/include; \
	mkdir -p ${HYPRE_BUILD_DIR}/lib; \
	cp -fp HYPRE_config.h ${HYPRE_BUILD_DIR}/include/.; \
	cp -fp $(srcdir)/HYPRE.h ${HYPRE_BUILD_DIR}/include/.; \
	for i in ${HYPRE_DIRS}; \
	do \
	  echo "Making $$i ..."; \
	  (cd $$i && $(MAKE) $@); \
	  echo ""; \
	done

help:
	@echo "     "
	@echo "************************************************************"
	@echo " HYPRE Make System Targets"
	@echo "   (using GNU-standards)"
	@echo "     "
	@echo "all:"
	@echo "     default target in all directories"
	@echo "     compile the entire program"
	@echo "     does not rebuild documentation"
	@echo "     "
	@echo "help:"
	@echo "     prints details of each target"
	@echo "     "
	@echo "install:"
	@echo "     compile the program and copy executables, libraries, etc"
	@echo "        to the file names where they reside for actual use"
	@echo "     executes mkinstalldirs script to create directories needed"
	@echo "     "
	@echo "uninstall:"
	@echo "     deletes all files that the install target creates"
	@echo "     "
	@echo "clean:"
	@echo "     deletes all files from the current directory that are normally"
	@echo "        created by building the program"
	@echo "     "
	@echo "distclean:"
	@echo "     deletes all files from the current directory that are"
	@echo "        created by configuring or building the program"
	@echo "     "
	@echo "tags:"
	@echo "     runs etags to create tags table"
	@echo "     file is named TAGS and is saved in current directory"
	@echo "     "
	@echo "test:"
	@echo "     depends on the all target to be completed"
	@echo "     removes existing temporary installation sub-directory"
	@echo "     creates a temporary installation sub-directory"
	@echo "     copies all libHYPRE* and *.h files to the temporary locations"
	@echo "     builds the test drivers; linking to the temporary installation"
	@echo "        directories to simulate how application codes will link to HYPRE"
	@echo "     "
	@echo "************************************************************"

test: all
	@ \
	echo "Making test drivers ..."; \
	(cd test; $(MAKE) distclean; $(MAKE) all); \

install: all
	@echo "Installing hypre ..."
	@echo "lib-install: ${HYPRE_LIB_INSTALL}"
	${HYPRE_SRC_TOP_DIR}/config/mkinstalldirs ${HYPRE_LIB_INSTALL} ${HYPRE_INC_INSTALL}
	cp -fr ${HYPRE_BUILD_DIR}/lib/* ${HYPRE_LIB_INSTALL}/.
	cp -fr ${HYPRE_BUILD_DIR}/include/* ${HYPRE_INC_INSTALL}/.
	chgrp -fR hypre ${HYPRE_LIB_INSTALL}
	chgrp -fR hypre ${HYPRE_INC_INSTALL}
	chmod -R a+rX,u+w,go-w ${HYPRE_LIB_INSTALL}
	chmod -R a+rX,u+w,go-w ${HYPRE_INC_INSTALL}

uninstall:
	@echo "Un-installing hypre ..."
	rm -rf ${HYPRE_LIB_INSTALL}
	rm -rf ${HYPRE_INC_INSTALL}

clean:
	@ \
	for i in ${HYPRE_DIRS} ${HYPRE_EXTRA_DIRS}; \
	do \
	  if [ -d $$i ]; \
	  then \
	    echo "Cleaning $$i ..."; \
	    (cd $$i && $(MAKE) $@); \
	  fi; \
	done

distclean:
	@ \
	rm -Rf hypre; \
	for i in ${HYPRE_DIRS} ${HYPRE_EXTRA_DIRS}; \
	do \
	  if [ -d $$i ]; \
	  then \
	    echo "Dist-cleaning $$i ..."; \
	    (cd $$i &&  $(MAKE) $@); \
	  fi; \
	done
	rm -rf ./config/Makefile.config
	rm -rf ./TAGS

tags:
	etags `find . -name "*.c" -or -name "*.C" -or -name "*.h" -or\
	-name "*.c??" -or -name "*.h??" -or -name "*.f"`