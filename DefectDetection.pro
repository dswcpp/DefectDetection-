# =============================================================================
# 项目名称: 产品缺陷检测系统
# 构建系统: qmake (Qt 6.5+)
# =============================================================================

TEMPLATE = subdirs
CONFIG += ordered

# 按依赖顺序构建
SUBDIRS += \
    src/common \
    src/data \
    src/network \
    src/hal \
    src/algorithm \
    src/ui \
    src/app

# 显式依赖声明（可选，用于并行构建）
src-data.depends = src-common
src-network.depends = src-common
src-hal.depends = src-common
src-algorithm.depends = src-common src-hal
src-ui.depends = src-common src-data src-hal src-algorithm
src-app.depends = src-common src-data src-network src-hal src-algorithm src-ui

# SUBDIRS 名称映射
src-common.subdir = src/common
src-data.subdir = src/data
src-network.subdir = src/network
src-hal.subdir = src/hal
src-algorithm.subdir = src/algorithm
src-ui.subdir = src/ui
src-app.subdir = src/app


