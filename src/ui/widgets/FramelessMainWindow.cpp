#include "FramelessMainWindow.h"
#include <QMouseEvent>
#include <QShowEvent>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QIcon>
#include <QDebug>

static const int TITLE_BAR_HEIGHT = 32;
static const int BUTTON_SIZE = 32;

FramelessMainWindow::FramelessMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    
    setupTitleBar();
    installEventFilter(this);
}

void FramelessMainWindow::setupTitleBar()
{
    // 创建标题栏容器
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("framelessMainWindowTitleBar");
    m_titleBar->setFixedHeight(TITLE_BAR_HEIGHT);
    m_titleBar->setStyleSheet(R"(
        #framelessMainWindowTitleBar {
            background-color: #1C1C1E;
            border-bottom: 1px solid #3C3C3E;
        }
    )");
    
    auto* titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(8, 0, 0, 0);
    titleLayout->setSpacing(0);
    
    // 图标
    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("framelessMainWindowIcon");
    m_iconLabel->setFixedSize(20, 20);
    m_iconLabel->setScaledContents(true);
    titleLayout->addWidget(m_iconLabel);
    
    titleLayout->addSpacing(8);
    
    // 标题
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("framelessMainWindowTitle");
    m_titleLabel->setStyleSheet("color: #E0E0E0; font-size: 13px; font-weight: 500;");
    titleLayout->addWidget(m_titleLabel);
    
    titleLayout->addStretch();
    
    // 最小化按钮
    m_minButton = new QPushButton(this);
    m_minButton->setObjectName("framelessMainWindowMinBtn");
    m_minButton->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_minButton->setCursor(Qt::PointingHandCursor);
    m_minButton->setIcon(QIcon(":/resources/icons/minimize_20.svg"));
    m_minButton->setIconSize(QSize(16, 16));
    m_minButton->setFlat(true);
    m_minButton->setStyleSheet("QPushButton { border: none; background: transparent; }");
    connect(m_minButton, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    titleLayout->addWidget(m_minButton);
    
    // 最大化按钮
    m_maxButton = new QPushButton(this);
    m_maxButton->setObjectName("framelessMainWindowMaxBtn");
    m_maxButton->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_maxButton->setCursor(Qt::PointingHandCursor);
    m_maxButton->setIcon(QIcon(":/resources/icons/maximize_20_normal.svg"));
    m_maxButton->setIconSize(QSize(16, 16));
    m_maxButton->setFlat(true);
    m_maxButton->setStyleSheet("QPushButton { border: none; background: transparent; }");
    connect(m_maxButton, &QPushButton::clicked, this, [this]() {
        setWindowState(isMaximized() ? Qt::WindowNoState : Qt::WindowMaximized);
    });
    titleLayout->addWidget(m_maxButton);
    
    // 关闭按钮
    m_closeButton = new QPushButton(this);
    m_closeButton->setObjectName("framelessMainWindowCloseBtn");
    m_closeButton->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setIcon(QIcon(":/resources/icons/close_20.svg"));
    m_closeButton->setIconSize(QSize(16, 16));
    m_closeButton->setFlat(true);
    m_closeButton->setStyleSheet("QPushButton { border: none; background: transparent; }");
    connect(m_closeButton, &QPushButton::clicked, this, &QMainWindow::close);
    titleLayout->addWidget(m_closeButton);
    
    // 使用 setMenuWidget 将标题栏放在菜单栏位置上方
    // 但这会替换掉菜单栏，所以我们改用其他方法
    
    // 设置可拖动的标题栏组件
    m_titlebarWidgets = {m_titleBar, m_iconLabel, m_titleLabel};
}

void FramelessMainWindow::setWindowTitle(const QString& title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
    QMainWindow::setWindowTitle(title);
}

void FramelessMainWindow::setWindowIcon(const QIcon& icon)
{
    if (m_iconLabel) {
        m_iconLabel->setPixmap(icon.pixmap(20, 20));
    }
    QMainWindow::setWindowIcon(icon);
}

void FramelessMainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    
    // 在首次显示时，将标题栏添加到布局顶部
    static bool initialized = false;
    if (!initialized && m_titleBar) {
        initialized = true;
        
        // 获取中央部件的父级，将标题栏插入到顶部
        if (auto* centralWidget = this->centralWidget()) {
            // 创建一个新的容器来包含标题栏和原有内容
            auto* container = new QWidget(this);
            auto* containerLayout = new QVBoxLayout(container);
            containerLayout->setContentsMargins(0, 0, 0, 0);
            containerLayout->setSpacing(0);
            
            // 先移除原中央部件
            centralWidget->setParent(nullptr);
            
            // 添加标题栏
            containerLayout->addWidget(m_titleBar);
            
            // 添加菜单栏（如果有）
            if (auto* mb = menuBar()) {
                mb->setParent(nullptr);
                containerLayout->addWidget(mb);
            }
            
            // 添加原中央部件
            containerLayout->addWidget(centralWidget, 1);
            
            // 设置新容器为中央部件
            setCentralWidget(container);
        }
    }
}

void FramelessMainWindow::updateMaximizeButton()
{
    if (isMaximized()) {
        m_maxButton->setIcon(QIcon(":/resources/icons/maximize_20_max.svg"));
        m_maxButton->setToolTip(tr("还原"));
    } else {
        m_maxButton->setIcon(QIcon(":/resources/icons/maximize_20_normal.svg"));
        m_maxButton->setToolTip(tr("最大化"));
    }
}

void FramelessMainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        updateMaximizeButton();
        emit windowStateChanged();
    }
    QMainWindow::changeEvent(event);
}

void FramelessMainWindow::setResizeable(bool resizable)
{
    m_canResize = resizable;
}

void FramelessMainWindow::setTitlebarWidgets(const QVector<QWidget*>& widgets)
{
    m_titlebarWidgets = widgets;
}

void FramelessMainWindow::setAllWidgetMouseTracking(QWidget* widget)
{
    widget->setMouseTracking(true);
    for (QObject* obj : widget->children()) {
        if (auto* w = qobject_cast<QWidget*>(obj)) {
            w->setMouseTracking(true);
            setAllWidgetMouseTracking(w);
        }
    }
}

bool FramelessMainWindow::eventFilter(QObject* target, QEvent* event)
{
    if (event->type() == QEvent::Paint) {
        static bool init = false;
        if (!init) {
            init = true;
            setAllWidgetMouseTracking(this);
        }
    }
    return QMainWindow::eventFilter(target, event);
}

void FramelessMainWindow::setCursorShape(const QPoint& point)
{
    if (!m_canResize || isMaximized()) {
        setCursor(Qt::ArrowCursor);
        m_hoverPos = MW_CENTER;
        return;
    }
    
    QRect rect = this->rect();
    QPoint topLeft = mapToGlobal(rect.topLeft());
    QPoint bottomRight = mapToGlobal(rect.bottomRight());

    int x = point.x();
    int y = point.y();

    if (x >= topLeft.x() && x <= topLeft.x() + m_padding && 
        y >= topLeft.y() && y <= topLeft.y() + m_padding) {
        m_hoverPos = MW_TOP_LEFT;
        setCursor(Qt::SizeFDiagCursor);
    } else if (x <= bottomRight.x() && x >= bottomRight.x() - m_padding && 
               y <= bottomRight.y() && y >= bottomRight.y() - m_padding) {
        m_hoverPos = MW_BOTTOM_RIGHT;
        setCursor(Qt::SizeFDiagCursor);
    } else if (x >= topLeft.x() && x <= topLeft.x() + m_padding && 
               y <= bottomRight.y() && y >= bottomRight.y() - m_padding) {
        m_hoverPos = MW_BOTTOM_LEFT;
        setCursor(Qt::SizeBDiagCursor);
    } else if (x <= bottomRight.x() && x >= bottomRight.x() - m_padding && 
               y >= topLeft.y() && y <= topLeft.y() + m_padding) {
        m_hoverPos = MW_TOP_RIGHT;
        setCursor(Qt::SizeBDiagCursor);
    } else if (x >= topLeft.x() && x <= topLeft.x() + m_padding) {
        m_hoverPos = MW_LEFT;
        setCursor(Qt::SizeHorCursor);
    } else if (x <= bottomRight.x() && x >= bottomRight.x() - m_padding) {
        m_hoverPos = MW_RIGHT;
        setCursor(Qt::SizeHorCursor);
    } else if (y >= topLeft.y() && y <= topLeft.y() + m_padding) {
        m_hoverPos = MW_TOP;
        setCursor(Qt::SizeVerCursor);
    } else if (y <= bottomRight.y() && y >= bottomRight.y() - m_padding) {
        m_hoverPos = MW_BOTTOM;
        setCursor(Qt::SizeVerCursor);
    } else {
        m_hoverPos = MW_CENTER;
        setCursor(Qt::ArrowCursor);
    }
}

void FramelessMainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QMainWindow::mousePressEvent(event);
        return;
    }

    m_leftPressed = true;
    m_wndPos = pos();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_pressPos = event->globalPosition().toPoint();
    QWidget* pressedWidget = QApplication::widgetAt(event->globalPosition().toPoint());
#else
    m_pressPos = event->globalPos();
    QWidget* pressedWidget = QApplication::widgetAt(event->globalPos());
#endif

    m_leftPressedInTitle = false;
    if (pressedWidget) {
        for (QWidget* widget : m_titlebarWidgets) {
            if (pressedWidget == widget) {
                m_leftPressedInTitle = true;
                break;
            }
        }
    }
    
    QMainWindow::mousePressEvent(event);
}

void FramelessMainWindow::mouseMoveEvent(QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPoint globalPos = event->globalPosition().toPoint();
#else
    QPoint globalPos = event->globalPos();
#endif

    if (!m_leftPressed) {
        if (windowState().testFlag(Qt::WindowNoState) && m_canResize) {
            setCursorShape(globalPos);
        }
        QMainWindow::mouseMoveEvent(event);
        return;
    }

    if (m_hoverPos != MW_CENTER && m_canResize && !isMaximized()) {
        QRect rect = this->rect();
        QPoint topLeft = mapToGlobal(rect.topLeft());
        QPoint bottomRight = mapToGlobal(rect.bottomRight());
        QRect rMove(topLeft, bottomRight);

        switch (m_hoverPos) {
            case MW_TOP:
                if (bottomRight.y() - globalPos.y() > minimumHeight())
                    rMove.setY(globalPos.y());
                break;
            case MW_BOTTOM:
                rMove.setHeight(globalPos.y() - topLeft.y());
                break;
            case MW_LEFT:
                if (bottomRight.x() - globalPos.x() > minimumWidth())
                    rMove.setX(globalPos.x());
                break;
            case MW_RIGHT:
                rMove.setWidth(globalPos.x() - topLeft.x());
                break;
            case MW_TOP_LEFT:
                if (bottomRight.y() - globalPos.y() > minimumHeight())
                    rMove.setY(globalPos.y());
                if (bottomRight.x() - globalPos.x() > minimumWidth())
                    rMove.setX(globalPos.x());
                break;
            case MW_TOP_RIGHT:
                if (bottomRight.y() - globalPos.y() > minimumHeight())
                    rMove.setY(globalPos.y());
                rMove.setWidth(globalPos.x() - topLeft.x());
                break;
            case MW_BOTTOM_LEFT:
                rMove.setHeight(globalPos.y() - topLeft.y());
                if (bottomRight.x() - globalPos.x() > minimumWidth())
                    rMove.setX(globalPos.x());
                break;
            case MW_BOTTOM_RIGHT:
                rMove.setHeight(globalPos.y() - topLeft.y());
                rMove.setWidth(globalPos.x() - topLeft.x());
                break;
            default:
                break;
        }
        setGeometry(rMove);
    } else if (m_leftPressedInTitle) {
        if (isMaximized()) {
            // 从最大化状态拖动：恢复窗口并保持鼠标相对位置
            float widthRatio = float(globalPos.x()) / float(width());
            setWindowState(Qt::WindowNoState);
            int offset = int(width() * widthRatio);
            m_wndPos.setX(globalPos.x() - offset);
            m_wndPos.setY(0);
        }
        move(m_wndPos + (globalPos - m_pressPos));
    }
    
    QMainWindow::mouseMoveEvent(event);
}

void FramelessMainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    m_leftPressed = false;
    m_leftPressedInTitle = false;
    QMainWindow::mouseReleaseEvent(event);
}

void FramelessMainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!m_canResize) {
        QMainWindow::mouseDoubleClickEvent(event);
        return;
    }
    
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QWidget* clickedWidget = QApplication::widgetAt(event->globalPosition().toPoint());
#else
    QWidget* clickedWidget = QApplication::widgetAt(event->globalPos());
#endif

    if (clickedWidget) {
        for (QWidget* widget : m_titlebarWidgets) {
            if (clickedWidget == widget) {
                setWindowState(isMaximized() ? Qt::WindowNoState : Qt::WindowMaximized);
                break;
            }
        }
    }
    
    QMainWindow::mouseDoubleClickEvent(event);
}
