#include \"SettingsDialog.h\"
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDoubleSpinBox>

auto createCard(QWidget* parent, const QString& title) {
  auto frame = new QFrame(parent);
  frame->setObjectName(QStringLiteral("SettingsCard"));
  auto layout = new QVBoxLayout(frame);
  layout->setContentsMargins(16, 12, 16, 16);
  layout->setSpacing(12);
  auto titleLabel = new QLabel(title, frame);
  titleLabel->setObjectName(QStringLiteral("SettingsCardTitle"));
  layout->addWidget(titleLabel);
  return frame;
}
