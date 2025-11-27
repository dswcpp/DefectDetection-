#ifndef DATA_GLOBAL_H
#define DATA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DATA_LIBRARY)
#  define DATA_LIBRARY Q_DECL_EXPORT
#else
#  define DATA_LIBRARY Q_DECL_IMPORT
#endif

#endif // DATA_GLOBAL_H
