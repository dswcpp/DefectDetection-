#include "FramelessDialog.h"
#include <QMouseEvent>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QStyle>
#include <QGraphicsDropShadowEffect>
#include <QPalette>
#include <QDebug>
#include <cstdio>

static const int TITLE_BAR_HEIGHT = 36;
static const int BUTTON_SIZE = 36;

FramelessDialog::FramelessDialog(QWidget* parent)
    : QDialog(parent)
{
    qDebug() << "[FramelessDialog] Constructor start, parent=" << parent;
    std::fflush(stdout);
    std::fflush(stderr);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    qDebug() << "[FramelessDialog] Window flags set";
    
    setupFramelessUI();
    qDebug() << "[FramelessDialog] UI setup done";
    applyDialogStyle();
    qDebug() << "[FramelessDialog] Style applied";
    
    installEventFilter(this);
    qDebug() << "[FramelessDialog] Constructor end";
}

void FramelessDialog::setupFramelessUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(0);
    
    // 主容器（带阴影效果）
    auto* containerWidget = new QWidget(this);
    containerWidget->setObjectName("framelessDialogContainer");
    containerWidget->setAutoFillBackground(true);
    
    // 设置默认背景色（QSS 会覆盖此设置）
    QPalette pal = containerWidget->palette();
    pal.setColor(QPalette::Window, QColor("#2C2C2E"));
    containerWidget->setPalette(pal);
    
    // 暂时禁用阴影效果（可能导致崩溃）
    // auto* shadow = new QGraphicsDropShadowEffect(this);
    // shadow->setBlurRadius(16);
    // shadow->setXOffset(0);
    // shadow->setYOffset(2);
    // shadow->setColor(QColor(0, 0, 0, 80));
    // containerWidget->setGraphicsEffect(shadow);
    
    auto* containerLayout = new QVBoxLayout(containerWidget);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    
    // 标题栏
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("framelessDialogTitleBar");
    m_titleBar->setFixedHeight(TITLE_BAR_HEIGHT);
    
    auto* titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(12, 0, 8, 0);
    titleLayout->setSpacing(8);
    
    // 图标
    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("framelessDialogIcon");
    m_iconLabel->setFixedSize(20, 20);
    m_iconLabel->setScaledContents(true);
    titleLayout->addWidget(m_iconLabel);
    
    // 标题
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("framelessDialogTitle");
    m_titleLabel->setStyleSheet("color: #FFFFFF; font-size: 14px; font-weight: 500;");
    titleLayout->addWidget(m_titleLabel);
    
    titleLayout->addStretch();
    
    // 最小化按钮 - 使用 SVG 图标，透明背景
    m_minButton = new QPushButton(this);
    m_minButton->setObjectName("framelessDialogMinBtn");
    m_minButton->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_minButton->setCursor(Qt::PointingHandCursor);
    m_minButton->setIcon(QIcon(":/resources/icons/minimize_20.svg"));
    m_minButton->setIconSize(QSize(20, 20));
    m_minButton->setFlat(true);
    m_minButton->setStyleSheet("QPushButton { border: none; background: transparent; }");
    m_minButton->setVisible(m_showMinBtn);
    connect(m_minButton, &QPushButton::clicked, this, &QDialog::showMinimized);
    titleLayout->addWidget(m_minButton);
    
    // 最大化按钮 - 使用 SVG 图标，透明背景
    m_maxButton = new QPushButton(this);
    m_maxButton->setObjectName("framelessDialogMaxBtn");
    m_maxButton->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_maxButton->setCursor(Qt::PointingHandCursor);
    m_maxButton->setIcon(QIcon(":/resources/icons/maximize_20_normal.svg"));
    m_maxButton->setIconSize(QSize(20, 20));
    m_maxButton->setFlat(true);
    m_maxButton->setStyleSheet("QPushButton { border: none; background: transparent; }");
    m_maxButton->setVisible(m_showMaxBtn);
    connect(m_maxButton, &QPushButton::clicked, this, [this]() {
        setWindowState(isMaximized() ? Qt::WindowNoState : Qt::WindowMaximized);
        updateMaximizeButton();
    });
    titleLayout->addWidget(m_maxButton);
    
    // 关闭按钮 - 使用 SVG 图标，透明背景
    m_closeButton = new QPushButton(this);
    m_closeButton->setObjectName("framelessDialogCloseBtn");
    m_closeButton->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setIcon(QIcon(":/resources/icons/close_20.svg"));
    m_closeButton->setIconSize(QSize(20, 20));
    m_closeButton->setFlat(true);
    m_closeButton->setStyleSheet("QPushButton { border: none; background: transparent; }");
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);
    titleLayout->addWidget(m_closeButton);
    
    containerLayout->addWidget(m_titleBar);
    
    // 内容区域
    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName("framelessDialogContent");
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);
    
    containerLayout->addWidget(m_contentWidget, 1);
    
    mainLayout->addWidget(containerWidget);
    
    // 设置可拖动的标题栏组件
    m_titlebarWidgets = {m_titleBar, m_iconLabel, m_titleLabel};
}

void FramelessDialog::applyDialogStyle()
{
    // 样式由 dark-theme.qss 统一管理
    // 按钮使用 SVG 图标，在 setupFramelessUI 中设置
}

void FramelessDialog::updateMaximizeButton()
{
    if (isMaximized()) {
        m_maxButton->setIcon(QIcon(":/resources/icons/maximize_20_max.svg"));
        m_maxButton->setToolTip(tr("还原"));
    } else {
        m_maxButton->setIcon(QIcon(":/resources/icons/maximize_20_normal.svg"));
        m_maxButton->setToolTip(tr("最大化"));
    }
}

void FramelessDialog::setDialogTitle(const QString& title)
{
    m_titleLabel->setText(title);
    setWindowTitle(title);
}

void FramelessDialog::setDialogIcon(const QIcon& icon)
{
    m_iconLabel->setPixmap(icon.pixmap(20, 20));
    setWindowIcon(icon);
}

void FramelessDialog::setResizeable(bool resizable)
{
    m_canResize = resizable;
}

void FramelessDialog::setShowMinButton(bool show)
{
    m_showMinBtn = show;
    if (m_minButton) m_minButton->setVisible(show);
}

void FramelessDialog::setShowMaxButton(bool show)
{
    m_showMaxBtn = show;
    if (m_maxButton) m_maxButton->setVisible(show);
}

void FramelessDialog::setTitlebarWidgets(const QVector<QWidget*>& widgets)
{
    m_titlebarWidgets = widgets;
}

void FramelessDialog::setAllWidgetMouseTracking(QWidget* widget)
{
    widget->setMouseTracking(true);
    for (QObject* obj : widget->children()) {
        if (auto* w = qobject_cast<QWidget*>(obj)) {
            w->setMouseTracking(true);
            setAllWidgetMouseTracking(w);
        }
    }
}

bool FramelessDialog::eventFilter(QObject* target, QEvent* event)
{
    if (event->type() == QEvent::Paint) {
        static bool init = false;
        if (!init) {
            init = true;
            setAllWidgetMouseTracking(this);
        }
    }
    return QDialog::eventFilter(target, event);
}

void FramelessDialog::setCursorShape(const QPoint& point)
{
    if (!m_canResize) {
        setCursor(Qt::ArrowCursor);
        return;
    }
    
    QRect rect = this->rect();
    QPoint topLeft = mapToGlobal(rect.topLeft());
    QPoint bottomRight = mapToGlobal(rect.bottomRight());

    int x = point.x();
    int y = point.y();

    if (x >= topLeft.x() && x <= topLeft.x() + m_padding && 
        y >= topLeft.y() && y <= topLeft.y() + m_padding) {
        m_hoverPos = DLG_TOP_LEFT;
        setCursor(Qt::SizeFDiagCursor);
    } else if (x <= bottomRight.x() && x >= bottomRight.x() - m_padding && 
               y <= bottomRight.y() && y >= bottomRight.y() - m_padding) {
        m_hoverPos = DLG_BOTTOM_RIGHT;
        setCursor(Qt::SizeFDiagCursor);
    } else if (x >= topLeft.x() && x <= topLeft.x() + m_padding && 
               y <= bottomRight.y() && y >= bottomRight.y() - m_padding) {
        m_hoverPos = DLG_BOTTOM_LEFT;
        setCursor(Qt::SizeBDiagCursor);
    } else if (x <= bottomRight.x() && x >= bottomRight.x() - m_padding && 
               y >= topLeft.y() && y <= topLeft.y() + m_padding) {
        m_hoverPos = DLG_TOP_RIGHT;
        setCursor(Qt::SizeBDiagCursor);
    } else if (x >= topLeft.x() && x <= topLeft.x() + m_padding) {
        m_hoverPos = DLG_LEFT;
        setCursor(Qt::SizeHorCursor);
    } else if (x <= bottomRight.x() && x >= bottomRight.x() - m_padding) {
        m_hoverPos = DLG_RIGHT;
        setCursor(Qt::SizeHorCursor);
    } else if (y >= topLeft.y() && y <= topLeft.y() + m_padding) {
        m_hoverPos = DLG_TOP;
        setCursor(Qt::SizeVerCursor);
    } else if (y <= bottomRight.y() && y >= bottomRight.y() - m_padding) {
        m_hoverPos = DLG_BOTTOM;
        setCursor(Qt::SizeVerCursor);
    } else {
        m_hoverPos = DLG_CENTER;
        setCursor(Qt::ArrowCursor);
    }
}

void FramelessDialog::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
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

    if (pressedWidget) {
        for (QWidget* widget : m_titlebarWidgets) {
            if (pressedWidget == widget) {
                m_leftPressedInTitle = true;
                break;
            }
        }
    }
}

void FramelessDialog::mouseMoveEvent(QMouseEvent* event)
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
        return;
    }

    if (m_hoverPos != DLG_CENTER) {
        QRect rect = this->rect();
        QPoint topLeft = mapToGlobal(rect.topLeft());
        QPoint bottomRight = mapToGlobal(rect.bottomRight());
        QRect rMove(topLeft, bottomRight);

        switch (m_hoverPos) {
            case DLG_TOP:
                if (bottomRight.y() - globalPos.y() > minimumHeight())
                    rMove.setY(globalPos.y());
                break;
            case DLG_BOTTOM:
                rMove.setHeight(globalPos.y() - topLeft.y());
                break;
            case DLG_LEFT:
                if (bottomRight.x() - globalPos.x() > minimumWidth())
                    rMove.setX(globalPos.x());
                break;
            case DLG_RIGHT:
                rMove.setWidth(globalPos.x() - topLeft.x());
                break;
            case DLG_TOP_LEFT:
                if (bottomRight.y() - globalPos.y() > minimumHeight())
                    rMove.setY(globalPos.y());
                if (bottomRight.x() - globalPos.x() > minimumWidth())
                    rMove.setX(globalPos.x());
                break;
            case DLG_TOP_RIGHT:
                if (bottomRight.y() - globalPos.y() > minimumHeight())
                    rMove.setY(globalPos.y());
                rMove.setWidth(globalPos.x() - topLeft.x());
                break;
            case DLG_BOTTOM_LEFT:
                rMove.setHeight(globalPos.y() - topLeft.y());
                if (bottomRight.x() - globalPos.x() > minimumWidth())
                    rMove.setX(globalPos.x());
                break;
            case DLG_BOTTOM_RIGHT:
                rMove.setHeight(globalPos.y() - topLeft.y());
                rMove.setWidth(globalPos.x() - topLeft.x());
                break;
            default:
                break;
        }
        setGeometry(rMove);
    } else {
        if (m_leftPressedInTitle && !isMaximized()) {
            move(m_wndPos + (globalPos - m_pressPos));
        }
    }
}

void FramelessDialog::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    m_leftPressed = false;
    m_leftPressedInTitle = false;
}

void FramelessDialog::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!m_canResize || !m_showMaxBtn) return;
    
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
}
