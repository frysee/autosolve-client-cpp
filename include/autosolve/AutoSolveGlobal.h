#pragma once

#ifdef AUTOSOLVE_SHARED
#  ifdef AUTOSOLVE_BUILD
#    define AUTOSOLVE_EXPORT Q_DECL_EXPORT
#  else
#    define AUTOSOLVE_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define AUTOSOLVE_EXPORT
#endif