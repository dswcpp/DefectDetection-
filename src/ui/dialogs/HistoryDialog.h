#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include "ui_global.h"

class HistoryView;
class DatabaseManager;

class UI_LIBRARY HistoryDialog : public QDialog {
  Q_OBJECT
public:
  explicit HistoryDialog(DatabaseManager* dbManager, QWidget *parent = nullptr);

private:
  HistoryView* m_historyView = nullptr;
};

#endif // HISTORYDIALOG_H
