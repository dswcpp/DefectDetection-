#ifndef IMAGEVIEWCONTROLS_H
#define IMAGEVIEWCONTROLS_H

#include <QWidget>

class QButtonGroup;
class QRadioButton;
class QCheckBox;
class QPushButton;
class QLabel;

class ImageViewControls : public QWidget {
  Q_OBJECT
public:
  explicit ImageViewControls(QWidget* parent = nullptr);

signals:
  // 显示模式改变信号
  void displayModeChanged(int mode);  // 0=原图, 1=标注图

  // ROI显示状态改变
  void showROIChanged(bool show);

  // 缩放控制
  void zoomInRequested();
  void zoomOutRequested();
  void zoomFitRequested();
  void zoomActualRequested();

public slots:
  void setZoomLevel(double zoom);

private:
  void setupUI();

  QButtonGroup* m_displayModeGroup;
  QRadioButton* m_originalButton;
  QRadioButton* m_annotatedButton;
  QCheckBox* m_showROICheck;
  QPushButton* m_zoomInButton;
  QPushButton* m_zoomOutButton;
  QPushButton* m_zoomFitButton;
  QPushButton* m_zoom100Button;
  QLabel* m_zoomLabel;
};

#endif // IMAGEVIEWCONTROLS_H