#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QWidget>

class LoginDialog : public QWidget {
  Q_OBJECT
public:
  explicit LoginDialog(QWidget *parent = nullptr);

signals:
};

#endif // LOGINDIALOG_H
