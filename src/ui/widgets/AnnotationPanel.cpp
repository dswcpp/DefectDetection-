#include "AnnotationPanel.h"
#include "ImageView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QRadioButton>
#include <QListWidget>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QButtonGroup>
#include <QScrollArea>

AnnotationPanel::AnnotationPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
    createConnections();
    updateButtonStates(false);
}

void AnnotationPanel::setImageView(ImageView* view) {
    if (m_imageView) {
        // 断开旧连接
        disconnect(m_imageView, nullptr, this, nullptr);
    }

    m_imageView = view;

    if (m_imageView) {
        // 连接新的ImageView信号
        connect(m_imageView, &ImageView::annotationModeChanged,
                this, &AnnotationPanel::updateButtonStates);

        connect(m_imageView, &ImageView::defectAnnotationAdded,
                this, [this](const DefectAnnotation&) { updateAnnotationList(); });

        connect(m_imageView, &ImageView::defectAnnotationUpdated,
                this, [this](int, const DefectAnnotation&) { updateAnnotationList(); });

        connect(m_imageView, &ImageView::defectAnnotationRemoved,
                this, [this](int) { updateAnnotationList(); });

        // 更新按钮状态
        bool annotating = m_imageView->isAnnotationMode();
        m_annotateBtn->setChecked(annotating);
        updateButtonStates(annotating);
    }
}

void AnnotationPanel::setupUI() {
    // 创建根布局
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 创建滚动区域
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);

    // 创建内容容器
    auto* contentWidget = new QWidget();
    auto* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 标题
    auto* titleLabel = new QLabel(tr("缺陷标注"));
    titleLabel->setObjectName("panelTitle");
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #E0E0E0;");
    mainLayout->addWidget(titleLabel);

    // 标注模式开关
    m_annotateBtn = new QPushButton(tr("开始标注"));
    m_annotateBtn->setCheckable(true);
    m_annotateBtn->setObjectName("primaryButton");
    m_annotateBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #43A047;
        }
        QPushButton:checked {
            background-color: #e53935;
        }
        QPushButton:checked:hover {
            background-color: #d32f2f;
        }
    )");
    mainLayout->addWidget(m_annotateBtn);

    // 分隔线
    auto* separator1 = new QFrame();
    separator1->setFrameShape(QFrame::HLine);
    separator1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator1);

    // 形状选择组
    auto* shapeGroup = new QGroupBox(tr("标注形状"));
    shapeGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #E0E0E0; }");
    auto* shapeLayout = new QVBoxLayout();

    auto* buttonGroup = new QButtonGroup(this);
    m_rectRadio = new QRadioButton(tr("矩形"));
    m_rectRadio->setChecked(true);
    m_circleRadio = new QRadioButton(tr("圆形"));
    m_polyRadio = new QRadioButton(tr("多边形"));

    buttonGroup->addButton(m_rectRadio, 0);
    buttonGroup->addButton(m_circleRadio, 1);
    buttonGroup->addButton(m_polyRadio, 2);

    shapeLayout->addWidget(m_rectRadio);
    shapeLayout->addWidget(m_circleRadio);
    shapeLayout->addWidget(m_polyRadio);
    shapeGroup->setLayout(shapeLayout);
    mainLayout->addWidget(shapeGroup);

    // 缺陷类型选择
    auto* typeLabel = new QLabel(tr("缺陷类型:"));
    typeLabel->setStyleSheet("font-weight: bold; color: #E0E0E0;");
    mainLayout->addWidget(typeLabel);

    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({tr("划痕"), tr("裂纹"), tr("气泡"),
                          tr("异物"), tr("变形"), tr("色差"), tr("其他")});
    m_typeCombo->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #555;
            border-radius: 4px;
            padding: 5px;
            background: #3C3C3E;
            color: #E0E0E0;
        }
        QComboBox:hover {
            border-color: #4CAF50;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox QAbstractItemView {
            background: #3C3C3E;
            color: #E0E0E0;
            selection-background-color: #4CAF50;
        }
    )");
    mainLayout->addWidget(m_typeCombo);

    // 严重等级选择
    auto* severityLabel = new QLabel(tr("严重等级:"));
    severityLabel->setStyleSheet("font-weight: bold; color: #E0E0E0;");
    mainLayout->addWidget(severityLabel);

    m_severityCombo = new QComboBox();
    m_severityCombo->addItems({tr("严重"), tr("重大"), tr("轻微"), tr("信息")});
    m_severityCombo->setCurrentIndex(2); // 默认轻微

    // 为不同严重等级设置不同颜色的样式
    m_severityCombo->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #555;
            border-radius: 4px;
            padding: 5px;
            background: #3C3C3E;
            color: #E0E0E0;
        }
        QComboBox:hover {
            border-color: #4CAF50;
        }
        QComboBox QAbstractItemView {
            background: #3C3C3E;
            color: #E0E0E0;
            selection-background-color: #4CAF50;
        }
    )");
    mainLayout->addWidget(m_severityCombo);

    // 分隔线
    auto* separator2 = new QFrame();
    separator2->setFrameShape(QFrame::HLine);
    separator2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator2);

    // 标注列表
    auto* listLabel = new QLabel(tr("标注列表:"));
    listLabel->setStyleSheet("font-weight: bold; color: #E0E0E0;");
    mainLayout->addWidget(listLabel);

    m_annotationList = new QListWidget();
    m_annotationList->setMaximumHeight(150);
    m_annotationList->setStyleSheet(R"(
        QListWidget {
            border: 1px solid #555;
            border-radius: 4px;
            background: #3C3C3E;
            color: #E0E0E0;
        }
        QListWidget::item {
            padding: 4px;
            border-bottom: 1px solid #555;
        }
        QListWidget::item:selected {
            background-color: #4CAF50;
            color: white;
        }
        QListWidget::item:hover {
            background-color: #48484A;
        }
    )");
    mainLayout->addWidget(m_annotationList);

    // 操作按钮组
    auto* buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(5);

    // 保存/加载按钮（水平排列）
    auto* fileButtonLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton(tr("保存"));
    m_saveBtn->setIcon(QIcon(":/resources/icons/save.svg"));
    m_saveBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: #43A047;
        }
        QPushButton:disabled {
            background-color: #555;
            color: #888;
        }
    )");

    m_loadBtn = new QPushButton(tr("加载"));
    m_loadBtn->setIcon(QIcon(":/resources/icons/open.svg"));
    m_loadBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5C6BC0;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: #3F51B5;
        }
        QPushButton:disabled {
            background-color: #555;
            color: #888;
        }
    )");

    fileButtonLayout->addWidget(m_saveBtn);
    fileButtonLayout->addWidget(m_loadBtn);
    buttonLayout->addLayout(fileButtonLayout);

    // 导出/清除按钮（水平排列）
    auto* actionButtonLayout = new QHBoxLayout();

    m_exportBtn = new QPushButton(tr("导出"));
    m_exportBtn->setIcon(QIcon(":/resources/icons/export.svg"));
    m_exportBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #FF9800;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: #F57C00;
        }
        QPushButton:disabled {
            background-color: #555;
            color: #888;
        }
    )");

    m_clearBtn = new QPushButton(tr("清除"));
    m_clearBtn->setIcon(QIcon(":/resources/icons/delete.svg"));
    m_clearBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #e53935;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: #d32f2f;
        }
        QPushButton:disabled {
            background-color: #555;
            color: #888;
        }
    )");

    actionButtonLayout->addWidget(m_exportBtn);
    actionButtonLayout->addWidget(m_clearBtn);
    buttonLayout->addLayout(actionButtonLayout);

    mainLayout->addLayout(buttonLayout);

    // 提示信息
    auto* hintLabel = new QLabel(tr("提示: ESC取消绘制, Delete删除选中"));
    hintLabel->setStyleSheet("color: #ADADAD; font-size: 11px;");
    hintLabel->setWordWrap(true);
    mainLayout->addWidget(hintLabel);

    mainLayout->addStretch();

    // 将内容容器设置到滚动区域
    scrollArea->setWidget(contentWidget);
    rootLayout->addWidget(scrollArea);

    // 设置整体样式 - 暗黑风格
    setStyleSheet(R"(
        AnnotationPanel {
            background-color: #2C2C2E;
            border: 1px solid #48484A;
            border-radius: 8px;
        }
        QScrollArea {
            background: transparent;
        }
        QScrollArea > QWidget > QWidget {
            background: transparent;
        }
        QRadioButton {
            color: #E0E0E0;
        }
        QRadioButton::indicator {
            width: 14px;
            height: 14px;
        }
        QFrame[frameShape="4"] {
            background-color: #48484A;
        }
    )");
}

void AnnotationPanel::createConnections() {
    // 标注模式切换
    connect(m_annotateBtn, &QPushButton::toggled,
            this, &AnnotationPanel::onAnnotationModeToggled);

    // 形状选择
    connect(m_rectRadio, &QRadioButton::clicked,
            this, &AnnotationPanel::onShapeSelectionChanged);
    connect(m_circleRadio, &QRadioButton::clicked,
            this, &AnnotationPanel::onShapeSelectionChanged);
    connect(m_polyRadio, &QRadioButton::clicked,
            this, &AnnotationPanel::onShapeSelectionChanged);

    // 类型和等级选择
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AnnotationPanel::onDefectTypeChanged);
    connect(m_severityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AnnotationPanel::onSeverityChanged);

    // 操作按钮
    connect(m_saveBtn, &QPushButton::clicked,
            this, &AnnotationPanel::onSaveAnnotations);
    connect(m_loadBtn, &QPushButton::clicked,
            this, &AnnotationPanel::onLoadAnnotations);
    connect(m_clearBtn, &QPushButton::clicked,
            this, &AnnotationPanel::onClearAnnotations);
    connect(m_exportBtn, &QPushButton::clicked,
            this, &AnnotationPanel::onExportImage);

    // 标注列表
    connect(m_annotationList, &QListWidget::itemClicked,
            this, &AnnotationPanel::onAnnotationItemClicked);
}

void AnnotationPanel::onAnnotationModeToggled(bool checked) {
    if (!m_imageView) return;

    m_isAnnotating = checked;
    m_imageView->setAnnotationMode(checked);
    m_annotateBtn->setText(checked ? tr("结束标注") : tr("开始标注"));
    updateButtonStates(checked);

    emit annotationModeChanged(checked);
}

void AnnotationPanel::onShapeSelectionChanged() {
    if (!m_imageView) return;

    AnnotationShape shape = AnnotationShape::Rectangle;
    if (m_circleRadio->isChecked()) {
        shape = AnnotationShape::Circle;
    } else if (m_polyRadio->isChecked()) {
        shape = AnnotationShape::Polygon;
    }

    m_imageView->setCurrentAnnotationShape(shape);
}

void AnnotationPanel::onDefectTypeChanged(int index) {
    if (!m_imageView) return;
    m_imageView->setCurrentDefectType(static_cast<DefectType>(index));
}

void AnnotationPanel::onSeverityChanged(int index) {
    if (!m_imageView) return;
    m_imageView->setCurrentDefectSeverity(static_cast<DefectSeverity>(index));
}

void AnnotationPanel::onSaveAnnotations() {
    if (!m_imageView) return;

    QString filename = QFileDialog::getSaveFileName(this,
        tr("保存标注"),
        QString("annotations_%1.json").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        tr("JSON文件 (*.json)"));

    if (!filename.isEmpty()) {
        if (m_imageView->saveAnnotations(filename)) {
            QMessageBox::information(this, tr("成功"),
                tr("标注已保存到: %1").arg(filename));
            emit annotationsSaved(filename);
        } else {
            QMessageBox::warning(this, tr("错误"),
                tr("保存标注失败"));
        }
    }
}

void AnnotationPanel::onLoadAnnotations() {
    if (!m_imageView) return;

    QString filename = QFileDialog::getOpenFileName(this,
        tr("加载标注"), "", tr("JSON文件 (*.json)"));

    if (!filename.isEmpty()) {
        if (m_imageView->loadAnnotations(filename)) {
            updateAnnotationList();
            QMessageBox::information(this, tr("成功"),
                tr("标注已加载"));
            emit annotationsLoaded(filename);
        } else {
            QMessageBox::warning(this, tr("错误"),
                tr("加载标注失败"));
        }
    }
}

void AnnotationPanel::onClearAnnotations() {
    if (!m_imageView) return;

    auto reply = QMessageBox::question(this, tr("确认"),
        tr("确定要清除所有标注吗？"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_imageView->clearAnnotations();
        updateAnnotationList();
    }
}

void AnnotationPanel::onExportImage() {
    if (!m_imageView) return;

    QString filename = QFileDialog::getSaveFileName(this,
        tr("导出标注图像"),
        QString("annotated_%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        tr("图像文件 (*.png *.jpg *.jpeg)"));

    if (!filename.isEmpty()) {
        QImage annotatedImage = m_imageView->exportAnnotatedImage();
        if (!annotatedImage.isNull()) {
            if (annotatedImage.save(filename)) {
                QMessageBox::information(this, tr("成功"),
                    tr("标注图像已导出到: %1").arg(filename));
            } else {
                QMessageBox::warning(this, tr("错误"),
                    tr("导出图像失败"));
            }
        } else {
            QMessageBox::warning(this, tr("错误"),
                tr("没有图像可导出"));
        }
    }
}

void AnnotationPanel::updateAnnotationList() {
    if (!m_imageView) return;

    m_annotationList->clear();

    QVector<DefectAnnotation> annotations = m_imageView->getDefectAnnotations();

    for (const auto& annotation : annotations) {
        QString severityStr;
        QColor severityColor;

        switch (annotation.severity) {
            case DefectSeverity::Critical:
                severityStr = tr("严重");
                severityColor = QColor("#e53e3e");
                break;
            case DefectSeverity::Major:
                severityStr = tr("重大");
                severityColor = QColor("#dd6b20");
                break;
            case DefectSeverity::Minor:
                severityStr = tr("轻微");
                severityColor = QColor("#d69e2e");
                break;
            case DefectSeverity::Info:
                severityStr = tr("信息");
                severityColor = QColor("#3182ce");
                break;
        }

        QString typeStr;
        switch (annotation.type) {
            case DefectType::Scratch: typeStr = tr("划痕"); break;
            case DefectType::Crack: typeStr = tr("裂纹"); break;
            case DefectType::Bubble: typeStr = tr("气泡"); break;
            case DefectType::ForeignObject: typeStr = tr("异物"); break;
            case DefectType::Deformation: typeStr = tr("变形"); break;
            case DefectType::ColorDefect: typeStr = tr("色差"); break;
            case DefectType::Other: typeStr = tr("其他"); break;
        }

        QString itemText = QString("#%1 [%2] %3").arg(annotation.id)
                                                  .arg(severityStr)
                                                  .arg(typeStr);

        if (!annotation.description.isEmpty()) {
            itemText += " - " + annotation.description;
        }

        if (annotation.isManual) {
            itemText += " (手动)";
        } else if (annotation.confidence > 0) {
            itemText += QString(" (%1%)").arg(static_cast<int>(annotation.confidence * 100));
        }

        auto* item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, annotation.id);

        // 设置项目颜色
        item->setForeground(severityColor);

        m_annotationList->addItem(item);
    }
}

void AnnotationPanel::onAnnotationItemClicked() {
    // 可以添加选中标注的功能
    auto* item = m_annotationList->currentItem();
    if (item) {
        // int id = item->data(Qt::UserRole).toInt();
        // 可以高亮显示选中的标注
        // m_imageView->selectAnnotation(id);
        Q_UNUSED(item)
    }
}

void AnnotationPanel::updateButtonStates(bool annotating) {
    m_isAnnotating = annotating;

    // 标注时禁用某些按钮
    m_loadBtn->setEnabled(!annotating);

    // 更新标注按钮文本和样式
    if (annotating) {
        m_annotateBtn->setText(tr("结束标注"));
    } else {
        m_annotateBtn->setText(tr("开始标注"));
    }
}
