/*
 * File:          bHYPRE_MPICommunicator_IOR.c
 * Symbol:        bHYPRE.MPICommunicator-v1.0.0
 * Symbol Type:   class
 * Babel Version: 0.10.10
 * Description:   Intermediate Object Representation for bHYPRE.MPICommunicator
 * 
 * WARNING: Automatically generated; changes will be lost
 * 
 * babel-version = 0.10.10
 */

#include "sidl_rmi_InstanceHandle.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "bHYPRE_MPICommunicator_IOR.h"
#ifndef included_sidl_BaseClass_Impl_h
#include "sidl_BaseClass_Impl.h"
#endif
#ifndef included_sidl_BaseClass_h
#include "sidl_BaseClass.h"
#endif
#ifndef included_sidl_ClassInfo_h
#include "sidl_ClassInfo.h"
#endif
#ifndef included_sidl_ClassInfoI_h
#include "sidl_ClassInfoI.h"
#endif

#ifndef NULL
#define NULL 0
#endif

#include "sidl_thread.h"
#ifdef HAVE_PTHREAD
static struct sidl_recursive_mutex_t bHYPRE_MPICommunicator__mutex= SIDL_RECURSIVE_MUTEX_INITIALIZER;
#define LOCK_STATIC_GLOBALS sidl_recursive_mutex_lock( &bHYPRE_MPICommunicator__mutex )
#define UNLOCK_STATIC_GLOBALS sidl_recursive_mutex_unlock( &bHYPRE_MPICommunicator__mutex )
/* #define HAVE_LOCKED_STATIC_GLOBALS (sidl_recursive_mutex_trylock( &bHYPRE_MPICommunicator__mutex )==EDEADLOCK) */
#else
#define LOCK_STATIC_GLOBALS
#define UNLOCK_STATIC_GLOBALS
/* #define HAVE_LOCKED_STATIC_GLOBALS (1) */
#endif

/*
 * Static variables to hold version of IOR
 */

static const int32_t s_IOR_MAJOR_VERSION = 0;
static const int32_t s_IOR_MINOR_VERSION = 9;

/*
 * Static variable to hold shared ClassInfo interface.
 */

static sidl_ClassInfo s_classInfo = NULL;
static int s_classInfo_init = 1;

/*
 * Static variable to make sure _load called no more than once
 */

static int s_load_called = 0;
/*
 * Static variables for managing EPV initialization.
 */

static int s_method_initialized = 0;
static int s_static_initialized = 0;

static struct bHYPRE_MPICommunicator__epv  s_new_epv__bhypre_mpicommunicator;
static struct bHYPRE_MPICommunicator__sepv s_stc_epv__bhypre_mpicommunicator;

static struct sidl_BaseClass__epv  s_new_epv__sidl_baseclass;
static struct sidl_BaseClass__epv* s_old_epv__sidl_baseclass;

static struct sidl_BaseInterface__epv  s_new_epv__sidl_baseinterface;
static struct sidl_BaseInterface__epv* s_old_epv__sidl_baseinterface;

/*
 * Declare EPV routines defined in the skeleton file.
 */

#ifdef __cplusplus
extern "C" {
#endif

extern void bHYPRE_MPICommunicator__set_epv(
  struct bHYPRE_MPICommunicator__epv* epv);
extern void bHYPRE_MPICommunicator__set_sepv(
  struct bHYPRE_MPICommunicator__sepv* sepv);
extern void bHYPRE_MPICommunicator__call_load(void);
#ifdef __cplusplus
}
#endif

static void
bHYPRE_MPICommunicator_addRef__exec(
        struct bHYPRE_MPICommunicator__object* self,
        struct sidl_io_Deserializer__object* inArgs,
        struct sidl_io_Serializer__object* outArgs) {
  /* stack space for arguments */
  /* unpack in and inout argments */

  /* make the call */
  (self->d_epv->f_addRef)(
    self);

  /* pack return value */
  /* pack out and inout argments */

}

static void
bHYPRE_MPICommunicator_deleteRef__exec(
        struct bHYPRE_MPICommunicator__object* self,
        struct sidl_io_Deserializer__object* inArgs,
        struct sidl_io_Serializer__object* outArgs) {
  /* stack space for arguments */
  /* unpack in and inout argments */

  /* make the call */
  (self->d_epv->f_deleteRef)(
    self);

  /* pack return value */
  /* pack out and inout argments */

}

static void
bHYPRE_MPICommunicator_isSame__exec(
        struct bHYPRE_MPICommunicator__object* self,
        struct sidl_io_Deserializer__object* inArgs,
        struct sidl_io_Serializer__object* outArgs) {
  /* stack space for arguments */
  struct sidl_BaseInterface__object* iobj;
  sidl_bool _retval;
  sidl_BaseInterface _ex   = NULL;
  sidl_BaseInterface *_ex2 = &_ex;
  /* unpack in and inout argments */

  /* make the call */
  _retval = (self->d_epv->f_isSame)(
    self,
    iobj);

  /* pack return value */
  sidl_io_Serializer_packBool( outArgs, "_retval", _retval, _ex2);

  /* pack out and inout argments */

}

static void
bHYPRE_MPICommunicator_queryInt__exec(
        struct bHYPRE_MPICommunicator__object* self,
        struct sidl_io_Deserializer__object* inArgs,
        struct sidl_io_Serializer__object* outArgs) {
  /* stack space for arguments */
  char* name= NULL;
  struct sidl_BaseInterface__object* _retval;
  sidl_BaseInterface _ex   = NULL;
  sidl_BaseInterface *_ex2 = &_ex;
  /* unpack in and inout argments */
  sidl_io_Deserializer_unpackString( inArgs, "name", &name, _ex2);

  /* make the call */
  _retval = (self->d_epv->f_queryInt)(
    self,
    name);

  /* pack return value */
  /* pack out and inout argments */

}

static void
bHYPRE_MPICommunicator_isType__exec(
        struct bHYPRE_MPICommunicator__object* self,
        struct sidl_io_Deserializer__object* inArgs,
        struct sidl_io_Serializer__object* outArgs) {
  /* stack space for arguments */
  char* name= NULL;
  sidl_bool _retval;
  sidl_BaseInterface _ex   = NULL;
  sidl_BaseInterface *_ex2 = &_ex;
  /* unpack in and inout argments */
  sidl_io_Deserializer_unpackString( inArgs, "name", &name, _ex2);

  /* make the call */
  _retval = (self->d_epv->f_isType)(
    self,
    name);

  /* pack return value */
  sidl_io_Serializer_packBool( outArgs, "_retval", _retval, _ex2);

  /* pack out and inout argments */

}

static void
bHYPRE_MPICommunicator_getClassInfo__exec(
        struct bHYPRE_MPICommunicator__object* self,
        struct sidl_io_Deserializer__object* inArgs,
        struct sidl_io_Serializer__object* outArgs) {
  /* stack space for arguments */
  struct sidl_ClassInfo__object* _retval;
  /* unpack in and inout argments */

  /* make the call */
  _retval = (self->d_epv->f_getClassInfo)(
    self);

  /* pack return value */
  /* pack out and inout argments */

}

static void ior_bHYPRE_MPICommunicator__ensure_load_called(void) {
  /*
   * assert( HAVE_LOCKED_STATIC_GLOBALS );
   */

  if (! s_load_called ) {
    bHYPRE_MPICommunicator__call_load();
    s_load_called=1;
  }
}
/*
 * CAST: dynamic type casting support.
 */

static void* ior_bHYPRE_MPICommunicator__cast(
  struct bHYPRE_MPICommunicator__object* self,
  const char* name)
{
  void* cast = NULL;

  struct bHYPRE_MPICommunicator__object* s0 = self;
  struct sidl_BaseClass__object*         s1 = &s0->d_sidl_baseclass;

  if (!strcmp(name, "bHYPRE.MPICommunicator")) {
    cast = (void*) s0;
  } else if (!strcmp(name, "sidl.BaseClass")) {
    cast = (void*) s1;
  } else if (!strcmp(name, "sidl.BaseInterface")) {
    cast = (void*) &s1->d_sidl_baseinterface;
  }

  return cast;
}

/*
 * DELETE: call destructor and free object memory.
 */

static void ior_bHYPRE_MPICommunicator__delete(
  struct bHYPRE_MPICommunicator__object* self)
{
  bHYPRE_MPICommunicator__fini(self);
  memset((void*)self, 0, sizeof(struct bHYPRE_MPICommunicator__object));
  free((void*) self);
}

static char*
ior_bHYPRE_MPICommunicator__getURL(
    struct bHYPRE_MPICommunicator__object* self) {
  sidl_rmi_InstanceHandle conn = (sidl_rmi_InstanceHandle)self->d_data;
  sidl_BaseInterface _ex = NULL;
  /* TODO: Make this work for local object! */
  return NULL;
}
struct bHYPRE_MPICommunicator__method {
  const char *d_name;
  void (*d_func)(struct bHYPRE_MPICommunicator__object*,
    struct sidl_io_Deserializer__object *,
    struct sidl_io_Serializer__object *);
};

static void
ior_bHYPRE_MPICommunicator__exec(
    struct bHYPRE_MPICommunicator__object* self,
    const char* methodName,
    struct sidl_io_Deserializer__object* inArgs,
    struct sidl_io_Serializer__object* outArgs ) { 
  static const struct bHYPRE_MPICommunicator__method  s_methods[] = {
    { "addRef", bHYPRE_MPICommunicator_addRef__exec },
    { "deleteRef", bHYPRE_MPICommunicator_deleteRef__exec },
    { "getClassInfo", bHYPRE_MPICommunicator_getClassInfo__exec },
    { "isSame", bHYPRE_MPICommunicator_isSame__exec },
    { "isType", bHYPRE_MPICommunicator_isType__exec },
    { "queryInt", bHYPRE_MPICommunicator_queryInt__exec }
  };
  int i, cmp, l = 0;
  int u = sizeof(s_methods)/sizeof(struct bHYPRE_MPICommunicator__method);
  if (methodName) {
    /* Use binary search to locate method */
    while (l < u) {
      i = (l + u) >> 1;
      if (!(cmp=strcmp(methodName, s_methods[i].d_name))) {
        (s_methods[i].d_func)(self, inArgs, outArgs);
        return;
      }
      else if (cmp < 0) u = i;
      else l = i + 1;
    }
  }
  /* TODO: add code for method not found */
}
/*
 * EPV: create method entry point vector (EPV) structure.
 */

static void bHYPRE_MPICommunicator__init_epv(
  struct bHYPRE_MPICommunicator__object* self)
{
/*
 * assert( HAVE_LOCKED_STATIC_GLOBALS );
 */

  struct bHYPRE_MPICommunicator__object* s0 = self;
  struct sidl_BaseClass__object*         s1 = &s0->d_sidl_baseclass;

  struct bHYPRE_MPICommunicator__epv*  epv  = 
    &s_new_epv__bhypre_mpicommunicator;
  struct sidl_BaseClass__epv*          e0   = &s_new_epv__sidl_baseclass;
  struct sidl_BaseInterface__epv*      e1   = &s_new_epv__sidl_baseinterface;

  s_old_epv__sidl_baseinterface = s1->d_sidl_baseinterface.d_epv;
  s_old_epv__sidl_baseclass     = s1->d_epv;

  epv->f__cast                    = ior_bHYPRE_MPICommunicator__cast;
  epv->f__delete                  = ior_bHYPRE_MPICommunicator__delete;
  epv->f__exec                    = ior_bHYPRE_MPICommunicator__exec;
  epv->f__getURL                  = ior_bHYPRE_MPICommunicator__getURL;
  epv->f__ctor                    = NULL;
  epv->f__dtor                    = NULL;
  epv->f_addRef                   = (void (*)(struct 
    bHYPRE_MPICommunicator__object*)) s1->d_epv->f_addRef;
  epv->f_deleteRef                = (void (*)(struct 
    bHYPRE_MPICommunicator__object*)) s1->d_epv->f_deleteRef;
  epv->f_isSame                   = (sidl_bool (*)(struct 
    bHYPRE_MPICommunicator__object*,
    struct sidl_BaseInterface__object*)) s1->d_epv->f_isSame;
  epv->f_queryInt                 = (struct sidl_BaseInterface__object* 
    (*)(struct bHYPRE_MPICommunicator__object*,
    const char*)) s1->d_epv->f_queryInt;
  epv->f_isType                   = (sidl_bool (*)(struct 
    bHYPRE_MPICommunicator__object*,const char*)) s1->d_epv->f_isType;
  epv->f_getClassInfo             = (struct sidl_ClassInfo__object* (*)(struct 
    bHYPRE_MPICommunicator__object*)) s1->d_epv->f_getClassInfo;

  bHYPRE_MPICommunicator__set_epv(epv);

  e0->f__cast               = (void* (*)(struct sidl_BaseClass__object*,
    const char*)) epv->f__cast;
  e0->f__delete             = (void (*)(struct sidl_BaseClass__object*)) 
    epv->f__delete;
  e0->f__exec               = (void (*)(struct sidl_BaseClass__object*,
    const char*,struct sidl_io_Deserializer__object*,
    struct sidl_io_Serializer__object*)) epv->f__exec;
  e0->f_addRef              = (void (*)(struct sidl_BaseClass__object*)) 
    epv->f_addRef;
  e0->f_deleteRef           = (void (*)(struct sidl_BaseClass__object*)) 
    epv->f_deleteRef;
  e0->f_isSame              = (sidl_bool (*)(struct sidl_BaseClass__object*,
    struct sidl_BaseInterface__object*)) epv->f_isSame;
  e0->f_queryInt            = (struct sidl_BaseInterface__object* (*)(struct 
    sidl_BaseClass__object*,const char*)) epv->f_queryInt;
  e0->f_isType              = (sidl_bool (*)(struct sidl_BaseClass__object*,
    const char*)) epv->f_isType;
  e0->f_getClassInfo        = (struct sidl_ClassInfo__object* (*)(struct 
    sidl_BaseClass__object*)) epv->f_getClassInfo;

  e1->f__cast               = (void* (*)(void*,const char*)) epv->f__cast;
  e1->f__delete             = (void (*)(void*)) epv->f__delete;
  e1->f__exec               = (void (*)(void*,const char*,
    struct sidl_io_Deserializer__object*,
    struct sidl_io_Serializer__object*)) epv->f__exec;
  e1->f_addRef              = (void (*)(void*)) epv->f_addRef;
  e1->f_deleteRef           = (void (*)(void*)) epv->f_deleteRef;
  e1->f_isSame              = (sidl_bool (*)(void*,
    struct sidl_BaseInterface__object*)) epv->f_isSame;
  e1->f_queryInt            = (struct sidl_BaseInterface__object* (*)(void*,
    const char*)) epv->f_queryInt;
  e1->f_isType              = (sidl_bool (*)(void*,const char*)) epv->f_isType;
  e1->f_getClassInfo        = (struct sidl_ClassInfo__object* (*)(void*)) 
    epv->f_getClassInfo;

  s_method_initialized = 1;
  ior_bHYPRE_MPICommunicator__ensure_load_called();
}

/*
 * SEPV: create the static entry point vector (SEPV).
 */

static void bHYPRE_MPICommunicator__init_sepv(void)
{
  /*
   * assert( HAVE_LOCKED_STATIC_GLOBALS );
   */

  struct bHYPRE_MPICommunicator__sepv*  s = &s_stc_epv__bhypre_mpicommunicator;

  s->f_CreateC         = NULL;
  s->f_CreateF         = NULL;

  bHYPRE_MPICommunicator__set_sepv(s);

  s_static_initialized = 1;
  ior_bHYPRE_MPICommunicator__ensure_load_called();
}

/*
 * STATIC: return pointer to static EPV structure.
 */

struct bHYPRE_MPICommunicator__sepv*
bHYPRE_MPICommunicator__statics(void)
{
  LOCK_STATIC_GLOBALS;
  if (!s_static_initialized) {
    bHYPRE_MPICommunicator__init_sepv();
  }
  UNLOCK_STATIC_GLOBALS;
  return &s_stc_epv__bhypre_mpicommunicator;
}

/*
 * SUPER: return's parent's non-overrided EPV
 */

static struct sidl_BaseClass__epv* bHYPRE_MPICommunicator__super(void) {
  return s_old_epv__sidl_baseclass;
}

/*
 * initClassInfo: create a ClassInfo interface if necessary.
 */

static void
initClassInfo(sidl_ClassInfo *info)
{
  LOCK_STATIC_GLOBALS;
  if (s_classInfo_init) {
    sidl_ClassInfoI impl;
    s_classInfo_init = 0;
    impl = sidl_ClassInfoI__create();
    s_classInfo = sidl_ClassInfo__cast(impl);
    if (impl) {
      sidl_ClassInfoI_setName(impl, "bHYPRE.MPICommunicator");
      sidl_ClassInfoI_setIORVersion(impl, s_IOR_MAJOR_VERSION,
        s_IOR_MINOR_VERSION);
    }
  }
  if (s_classInfo) {
    if (*info) {
      sidl_ClassInfo_deleteRef(*info);
    }
    *info = s_classInfo;
    sidl_ClassInfo_addRef(*info);
  }
UNLOCK_STATIC_GLOBALS;
}

/*
 * initMetadata: store IOR version & class in sidl.BaseClass's data
 */

static void
initMetadata(struct bHYPRE_MPICommunicator__object* self)
{
  if (self) {
    struct sidl_BaseClass__data *data = 
      sidl_BaseClass__get_data(sidl_BaseClass__cast(self));
    if (data) {
      data->d_IOR_major_version = s_IOR_MAJOR_VERSION;
      data->d_IOR_minor_version = s_IOR_MINOR_VERSION;
      initClassInfo(&(data->d_classinfo));
    }
  }
}

/*
 * NEW: allocate object and initialize it.
 */

struct bHYPRE_MPICommunicator__object*
bHYPRE_MPICommunicator__new(void)
{
  struct bHYPRE_MPICommunicator__object* self =
    (struct bHYPRE_MPICommunicator__object*) malloc(
      sizeof(struct bHYPRE_MPICommunicator__object));
  bHYPRE_MPICommunicator__init(self);
  initMetadata(self);
  return self;
}

/*
 * INIT: initialize a new instance of the class object.
 */

void bHYPRE_MPICommunicator__init(
  struct bHYPRE_MPICommunicator__object* self)
{
  struct bHYPRE_MPICommunicator__object* s0 = self;
  struct sidl_BaseClass__object*         s1 = &s0->d_sidl_baseclass;

  sidl_BaseClass__init(s1);

  LOCK_STATIC_GLOBALS;
  if (!s_method_initialized) {
    bHYPRE_MPICommunicator__init_epv(s0);
  }
  UNLOCK_STATIC_GLOBALS;

  s1->d_sidl_baseinterface.d_epv = &s_new_epv__sidl_baseinterface;
  s1->d_epv                      = &s_new_epv__sidl_baseclass;

  s0->d_epv    = &s_new_epv__bhypre_mpicommunicator;

  s0->d_data = NULL;


  (*(self->d_epv->f__ctor))(self);
}

/*
 * FINI: deallocate a class instance (destructor).
 */

void bHYPRE_MPICommunicator__fini(
  struct bHYPRE_MPICommunicator__object* self)
{
  struct bHYPRE_MPICommunicator__object* s0 = self;
  struct sidl_BaseClass__object*         s1 = &s0->d_sidl_baseclass;

  (*(s0->d_epv->f__dtor))(s0);

  s1->d_sidl_baseinterface.d_epv = s_old_epv__sidl_baseinterface;
  s1->d_epv                      = s_old_epv__sidl_baseclass;

  sidl_BaseClass__fini(s1);
}

/*
 * VERSION: Return the version of the IOR used to generate this IOR.
 */

void
bHYPRE_MPICommunicator__IOR_version(int32_t *major, int32_t *minor)
{
  *major = s_IOR_MAJOR_VERSION;
  *minor = s_IOR_MINOR_VERSION;
}

static const struct bHYPRE_MPICommunicator__external
s_externalEntryPoints = {
  bHYPRE_MPICommunicator__new,
  bHYPRE_MPICommunicator__statics,
  bHYPRE_MPICommunicator__super
};

/*
 * This function returns a pointer to a static structure of
 * pointers to function entry points.  Its purpose is to provide
 * one-stop shopping for loading DLLs.
 */

const struct bHYPRE_MPICommunicator__external*
bHYPRE_MPICommunicator__externals(void)
{
  return &s_externalEntryPoints;
}
