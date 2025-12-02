#ifndef IMAGEPREVIEWDIALOG_H
#define IMAGEPREVIEWDIALOG_H

#include <QDialog>
#include <QImage>
#include "ui_global.h"

class QLabel;
class QScrollArea;

class UI_LIBRARY ImagePreviewDialog : public QDialog {
  Q_OBJECT
public:
  explicit ImagePreviewDialog(QWidget *parent = nullptr);

  void setImage(const QImage& image);
  void setImage(const QString& imagePath);

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void showEvent(QShowEvent* event) override;

private:
  void setupUI();
  void updateZoom();
  void fitToWindow();

  QLabel* m_imageLabel = nullptr;
  QScrollArea* m_scrollArea = nullptr;
  QImage m_originalImage;
  double m_zoomFactor = 1.0;
  bool m_firstShow = true;
};

#endif // IMAGEPREVIEWDIALOG_H
