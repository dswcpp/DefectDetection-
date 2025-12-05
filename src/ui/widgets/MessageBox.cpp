#include "MessageBox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>

DroidMessageBox::DroidMessageBox(QWidget* parent)
    : FramelessDialog(parent)
{
    setShowMinButton(false);
    setShowMaxButton(false);
    setResizeable(false);
    setupUI();
}

void DroidMessageBox::setupUI()
{
    setFixedWidth(400);

    auto* layout = contentLayout();
    layout->setSpacing(16);
    layout->setContentsMargins(24, 16, 24, 24);

    // 图标和文本区域
    auto* topLayout = new QHBoxLayout();
    topLayout->setSpacing(16);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(48, 48);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet("font-size: 32px;");
    topLayout->addWidget(m_iconLabel, 0, Qt::AlignTop);

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(8);

    m_textLabel = new QLabel(this);
    m_textLabel->setWordWrap(true);
    m_textLabel->setStyleSheet("color: #FFFFFF; font-size: 14px; font-weight: 500;");
    textLayout->addWidget(m_textLabel);

    m_informativeLabel = new QLabel(this);
    m_informativeLabel->setWordWrap(true);
    m_informativeLabel->setStyleSheet("color: #ADADAD; font-size: 13px;");
    m_informativeLabel->hide();
    textLayout->addWidget(m_informativeLabel);

    topLayout->addLayout(textLayout, 1);
    layout->addLayout(topLayout);

    // 按钮区域
    m_buttonContainer = new QWidget(this);
    auto* buttonLayout = new QHBoxLayout(m_buttonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(12);
    buttonLayout->addStretch();
    layout->addWidget(m_buttonContainer);
}

void DroidMessageBox::setIcon(Icon icon)
{
    QString iconPath;
    switch (icon) {
        case Information:
            iconPath = ":/resources/icons/info.svg";
            break;
        case Warning:
            iconPath = ":/resources/icons/warn.svg";
            break;
        case Critical:
            iconPath = ":/resources/icons/error.svg";
            break;
        case Question:
            iconPath = ":/resources/icons/info.svg";
            break;
        default:
            break;
    }
    if (!iconPath.isEmpty()) {
        m_iconLabel->setPixmap(QIcon(iconPath).pixmap(32, 32));
        m_iconLabel->setVisible(true);
    } else {
        m_iconLabel->setVisible(false);
    }
}

void DroidMessageBox::setText(const QString& text)
{
    m_textLabel->setText(text);
}

void DroidMessageBox::setInformativeText(const QString& text)
{
    m_informativeLabel->setText(text);
    m_informativeLabel->setVisible(!text.isEmpty());
}

void DroidMessageBox::setStandardButtons(StandardButtons buttons)
{
    // 清除现有按钮
    auto* layout = qobject_cast<QHBoxLayout*>(m_buttonContainer->layout());
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    layout->addStretch();

    // 添加新按钮
    if (buttons & No) addButton(No);
    if (buttons & Cancel) addButton(Cancel);
    if (buttons & Yes) addButton(Yes);
    if (buttons & Ok) addButton(Ok);
}

void DroidMessageBox::addButton(StandardButton button)
{
    auto* btn = new QPushButton(buttonText(button), this);
    btn->setMinimumSize(80, 32);
    btn->setCursor(Qt::PointingHandCursor);

    bool isPrimary = (button == Ok || button == Yes);
    if (isPrimary) {
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #007AFF;
                border: none;
                border-radius: 4px;
                color: white;
                font-weight: 500;
                padding: 6px 16px;
            }
            QPushButton:hover {
                background-color: #0066DD;
            }
            QPushButton:pressed {
                background-color: #0055CC;
            }
        )");
    } else {
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #3C3C3E;
                border: 1px solid #48484A;
                border-radius: 4px;
                color: #E0E0E0;
                padding: 6px 16px;
            }
            QPushButton:hover {
                background-color: #48484A;
            }
            QPushButton:pressed {
                background-color: #2C2C2E;
            }
        )");
    }

    connect(btn, &QPushButton::clicked, this, [this, button]() {
        m_clickedButton = button;
        accept();
    });

    auto* layout = qobject_cast<QHBoxLayout*>(m_buttonContainer->layout());
    layout->addWidget(btn);
}

QString DroidMessageBox::buttonText(StandardButton button) const
{
    switch (button) {
        case Ok: return tr("确定");
        case Cancel: return tr("取消");
        case Yes: return tr("是");
        case No: return tr("否");
        default: return QString();
    }
}

DroidMessageBox::StandardButton DroidMessageBox::information(QWidget* parent, const QString& title,
                                                    const QString& text, StandardButtons buttons)
{
    // 使用顶层窗口作为父窗口，避免与 FramelessDialog 冲突
    QWidget* topLevel = parent ? parent->window() : nullptr;
    DroidMessageBox box(topLevel);
    box.setDialogTitle(title);
    box.setIcon(Information);
    box.setText(text);
    box.setStandardButtons(buttons);
    box.exec();
    return box.clickedButton();
}

DroidMessageBox::StandardButton DroidMessageBox::warning(QWidget* parent, const QString& title,
                                                const QString& text, StandardButtons buttons)
{
    QWidget* topLevel = parent ? parent->window() : nullptr;
    DroidMessageBox box(topLevel);
    box.setDialogTitle(title);
    box.setIcon(Warning);
    box.setText(text);
    box.setStandardButtons(buttons);
    box.exec();
    return box.clickedButton();
}

DroidMessageBox::StandardButton DroidMessageBox::critical(QWidget* parent, const QString& title,
                                                 const QString& text, StandardButtons buttons)
{
    QWidget* topLevel = parent ? parent->window() : nullptr;
    DroidMessageBox box(topLevel);
    box.setDialogTitle(title);
    box.setIcon(Critical);
    box.setText(text);
    box.setStandardButtons(buttons);
    box.exec();
    return box.clickedButton();
}

DroidMessageBox::StandardButton DroidMessageBox::question(QWidget* parent, const QString& title,
                                                 const QString& text, StandardButtons buttons)
{
    QWidget* topLevel = parent ? parent->window() : nullptr;
    DroidMessageBox box(topLevel);
    box.setDialogTitle(title);
    box.setIcon(Question);
    box.setText(text);
    box.setStandardButtons(buttons);
    box.exec();
    return box.clickedButton();
}
