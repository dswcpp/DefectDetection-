/*
 * Copyright (c) 2025.12
 * All rights reserved.
 *
 * AnnotationPanel.h
 *
 * 初始版本：1.0
 * 作者：Vere
 * 创建日期：2025年12月03日
 * 摘要：标注面板控件接口定义
 * 描述：缺陷标注工具面板，提供标注形状选择、缺陷类型选择、
 *       严重等级选择、标注列表管理等功能
 *
 * 当前版本：1.0
 */

#ifndef ANNOTATIONPANEL_H
#define ANNOTATIONPANEL_H

#include <QWidget>
#include "ui_global.h"

class ImageView;
class QPushButton;
class QComboBox;
class QRadioButton;
class QListWidget;
class QScrollArea;

class UI_LIBRARY AnnotationPanel : public QWidget {
    Q_OBJECT

public:
    explicit AnnotationPanel(QWidget* parent = nullptr);
    ~AnnotationPanel() = default;

    // 设置关联的ImageView
    void setImageView(ImageView* view);

signals:
    void annotationModeChanged(bool enabled);
    void annotationsSaved(const QString& filename);
    void annotationsLoaded(const QString& filename);

private slots:
    void onAnnotationModeToggled(bool checked);
    void onShapeSelectionChanged();
    void onDefectTypeChanged(int index);
    void onSeverityChanged(int index);
    void onSaveAnnotations();
    void onLoadAnnotations();
    void onClearAnnotations();
    void onExportImage();
    void updateAnnotationList();
    void onAnnotationItemClicked();

private:
    void setupUI();
    void createConnections();
    void updateButtonStates(bool annotating);

    // UI组件
    ImageView* m_imageView = nullptr;
    QPushButton* m_annotateBtn;
    QRadioButton* m_rectRadio;
    QRadioButton* m_circleRadio;
    QRadioButton* m_polyRadio;
    QComboBox* m_typeCombo;
    QComboBox* m_severityCombo;
    QPushButton* m_saveBtn;
    QPushButton* m_loadBtn;
    QPushButton* m_clearBtn;
    QPushButton* m_exportBtn;
    QListWidget* m_annotationList;

    // 状态
    bool m_isAnnotating = false;
};

#endif // ANNOTATIONPANEL_H