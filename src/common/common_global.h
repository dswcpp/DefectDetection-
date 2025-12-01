#ifndef COMMON_GLOBAL_H
#define COMMON_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(COMMON_LIBRARY_BUILD)
#  define COMMON_LIBRARY Q_DECL_EXPORT
#else
#  define COMMON_LIBRARY Q_DECL_IMPORT
#endif

#endif // COMMON_GLOBAL_H
