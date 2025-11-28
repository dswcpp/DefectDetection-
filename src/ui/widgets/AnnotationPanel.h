#ifndef ANNOTATIONPANEL_H
#define ANNOTATIONPANEL_H

#include <QWidget>
#include "ui_global.h"

class ImageView;
class QPushButton;
class QComboBox;
class QRadioButton;
class QListWidget;

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