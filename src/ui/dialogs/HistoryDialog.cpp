#include "HistoryDialog.h"
#include "views/HistoryView.h"
#include "data/DatabaseManager.h"
#include <QVBoxLayout>

HistoryDialog::HistoryDialog(DatabaseManager* dbManager, QWidget *parent)
    : QDialog(parent) {
  setWindowTitle(tr("历史记录"));
  setMinimumSize(1200, 700);
  resize(1400, 800);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  m_historyView = new HistoryView(this);
  m_historyView->setDatabaseManager(dbManager);
  layout->addWidget(m_historyView);

  m_historyView->refresh();
}
