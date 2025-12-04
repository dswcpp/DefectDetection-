#include "HistoryDialog.h"
#include "views/HistoryView.h"
#include "data/DatabaseManager.h"
#include <QVBoxLayout>

HistoryDialog::HistoryDialog(DatabaseManager* dbManager, QWidget *parent)
    : FramelessDialog(parent) {
  setDialogTitle(tr("历史记录"));
  setShowMaxButton(true);
  setMinimumSize(1200, 750);
  resize(1400, 850);

  auto* layout = contentLayout();
  layout->setContentsMargins(0, 0, 0, 0);

  m_historyView = new HistoryView(this);
  m_historyView->setDatabaseManager(dbManager);
  layout->addWidget(m_historyView);

  m_historyView->refresh();
}
