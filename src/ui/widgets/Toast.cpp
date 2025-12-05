#include "Toast.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QIcon>
#include <QDebug>

Toast::Toast(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    setupUI();
    
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &Toast::close);
}

void Toast::setupUI()
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(10);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(20, 20);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_iconLabel);

    m_textLabel = new QLabel(this);
    m_textLabel->setStyleSheet("color: #FFFFFF; font-size: 14px;");
    layout->addWidget(m_textLabel, 1);

    setMinimumWidth(200);
    setMaximumWidth(400);
}

void Toast::setType(Type type)
{
    m_type = type;
    updateStyle();
}

void Toast::setText(const QString& text)
{
    m_textLabel->setText(text);
    adjustSize();
}

void Toast::setDuration(int ms)
{
    m_duration = ms;
}

void Toast::updateStyle()
{
    QString iconPath;
    QString bgColor;
    QString borderColor;

    switch (m_type) {
        case Success:
            iconPath = ":/resources/icons/check.svg";
            bgColor = "#1B5E20";
            borderColor = "#4CAF50";
            break;
        case Warning:
            iconPath = ":/resources/icons/warn.svg";
            bgColor = "#E65100";
            borderColor = "#FF9800";
            break;
        case Error:
            iconPath = ":/resources/icons/error.svg";
            bgColor = "#B71C1C";
            borderColor = "#F44336";
            break;
        case Information:
        default:
            iconPath = ":/resources/icons/info.svg";
            bgColor = "#0D47A1";
            borderColor = "#2196F3";
            break;
    }

    m_iconLabel->setPixmap(QIcon(iconPath).pixmap(20, 20));
}

void Toast::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    positionToast();
    
    if (m_duration > 0) {
        m_timer->start(m_duration);
    }

    // 淡入动画
    auto* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    auto* animation = new QPropertyAnimation(effect, "opacity", this);
    animation->setDuration(200);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Toast::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QString bgColor, borderColor;
    switch (m_type) {
        case Success:
            bgColor = "#1B5E20";
            borderColor = "#4CAF50";
            break;
        case Warning:
            bgColor = "#E65100";
            borderColor = "#FF9800";
            break;
        case Error:
            bgColor = "#B71C1C";
            borderColor = "#F44336";
            break;
        case Information:
        default:
            bgColor = "#0D47A1";
            borderColor = "#2196F3";
            break;
    }

    painter.setPen(QPen(QColor(borderColor), 1));
    painter.setBrush(QColor(bgColor));
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 6, 6);
}

void Toast::positionToast()
{
    QWidget* p = parentWidget();
    if (p) {
        int x = p->x() + (p->width() - width()) / 2;
        int y = p->y() + 60;
        move(x, y);
    } else {
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenGeom = screen->availableGeometry();
            int x = screenGeom.x() + (screenGeom.width() - width()) / 2;
            int y = screenGeom.y() + 60;
            move(x, y);
        }
    }
}

void Toast::show(QWidget* parent, const QString& text, Type type, int duration)
{
    qDebug() << "[Toast::show] Start, text:" << text;
    
    // 使用 nullptr 作为父窗口避免与 FramelessDialog 冲突
    auto* toast = new Toast(nullptr);
    qDebug() << "[Toast::show] Toast created";
    
    toast->setType(type);
    qDebug() << "[Toast::show] Type set";
    
    toast->setText(text);
    qDebug() << "[Toast::show] Text set";
    
    toast->setDuration(duration);
    qDebug() << "[Toast::show] Duration set";
    
    // 手动定位到 parent 位置
    if (parent) {
        qDebug() << "[Toast::show] Positioning to parent";
        QPoint parentCenter = parent->mapToGlobal(QPoint(parent->width() / 2, 60));
        toast->adjustSize();
        toast->move(parentCenter.x() - toast->width() / 2, parentCenter.y());
        qDebug() << "[Toast::show] Positioned";
    }
    
    qDebug() << "[Toast::show] Showing toast";
    toast->QWidget::show();
    qDebug() << "[Toast::show] Done";
}

void Toast::info(QWidget* parent, const QString& text, int duration)
{
    show(parent, text, Information, duration);
}

void Toast::success(QWidget* parent, const QString& text, int duration)
{
    show(parent, text, Success, duration);
}

void Toast::warning(QWidget* parent, const QString& text, int duration)
{
    show(parent, text, Warning, duration);
}

void Toast::error(QWidget* parent, const QString& text, int duration)
{
    show(parent, text, Error, duration);
}
