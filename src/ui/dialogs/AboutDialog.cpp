#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QApplication>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setupUI();
}

void AboutDialog::setupUI() {
    setWindowTitle(tr("关于"));
    setFixedSize(450, 400);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // 主布局
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 20);
    mainLayout->setSpacing(10);

    // Logo（可以用应用图标或者文字代替）
    auto* logoLabel = new QLabel();
    logoLabel->setAlignment(Qt::AlignCenter);
    // 如果有logo图片，可以设置：
    // logoLabel->setPixmap(QPixmap(":/icons/logo.svg").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    // 暂时用文字代替
    logoLabel->setText("🔍");
    logoLabel->setStyleSheet("font-size: 48px;");
    mainLayout->addWidget(logoLabel);

    mainLayout->addSpacing(10);

    // 应用名称
    auto* titleLabel = new QLabel(tr("缺陷检测系统"));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 20px;
        font-weight: bold;
        color: #333;
    )");
    mainLayout->addWidget(titleLabel);

    // 版本号
    auto* versionLabel = new QLabel(tr("版本 1.0.0"));
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(R"(
        font-size: 14px;
        color: #666;
    )");
    mainLayout->addWidget(versionLabel);

    mainLayout->addSpacing(20);

    // 描述
    auto* descLabel = new QLabel(tr("基于机器视觉的工业缺陷检测系统\n提供高精度、实时的产品质量检测解决方案"));
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(R"(
        font-size: 13px;
        color: #555;
        line-height: 1.5;
    )");
    mainLayout->addWidget(descLabel);

    mainLayout->addSpacing(20);

    // 技术栈
    auto* techLabel = new QLabel(tr("技术栈：Qt 6.8.1 | OpenCV 4.6.0 | C++17"));
    techLabel->setAlignment(Qt::AlignCenter);
    techLabel->setStyleSheet(R"(
        font-size: 12px;
        color: #777;
        background-color: #f8f9fa;
        padding: 8px;
        border-radius: 4px;
    )");
    mainLayout->addWidget(techLabel);

    mainLayout->addStretch();

    // 版权信息
    auto* copyrightLabel = new QLabel(tr("© 2025 All Rights Reserved"));
    copyrightLabel->setAlignment(Qt::AlignCenter);
    copyrightLabel->setStyleSheet(R"(
        font-size: 11px;
        color: #999;
    )");
    mainLayout->addWidget(copyrightLabel);

    mainLayout->addSpacing(20);

    // 关闭按钮
    auto* closeBtn = new QPushButton(tr("关闭"));
    closeBtn->setFixedWidth(100);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 16px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QPushButton:pressed {
            background-color: #004085;
        }
    )");

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
}
