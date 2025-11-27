#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QWidget>

class AboutDialog : public QWidget {
  Q_OBJECT
public:
  explicit AboutDialog(QWidget *parent = nullptr);

signals:
};

#endif // ABOUTDIALOG_H
