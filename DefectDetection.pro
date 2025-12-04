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


