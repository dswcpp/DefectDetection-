#include <cstddef>

// 手动补齐 Qt6EntryPoint 依赖的 argv/argc 导入符号
extern "C" {
int __argc = 0;
char **__argv = nullptr;
int *__imp___argc = &__argc;
char ***__imp___argv = &__argv;
}
