#ifndef UI_GLOBAL_H
#define UI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UI_LIBRARY)
#  define UI_LIBRARY Q_DECL_EXPORT
#else
#  define UI_LIBRARY Q_DECL_IMPORT
#endif

#endif // UI_GLOBAL_H
