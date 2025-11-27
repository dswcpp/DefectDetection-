#include "SettingsDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
const auto kDialogPadding = 32;

QFrame* createCard(QWidget* parent, const QString& title)
{
  auto* frame = new QFrame(parent);
  frame->setObjectName(QStringLiteral("SettingsCard"));
  auto* layout = new QVBoxLayout(frame);
  layout->setContentsMargins(16, 12, 16, 16);
  layout->setSpacing(12);

  auto* titleLabel = new QLabel(title, frame);
  titleLabel->setObjectName(QStringLiteral("SettingsCardTitle"));
  layout->addWidget(titleLabel);
  return frame;
}

QWidget* createPathEditor(QWidget* parent, const QString& placeholder)
{
  auto* container = new QWidget(parent);
  auto* layout = new QHBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(8);

  auto* edit = new QLineEdit(container);
  edit->setPlaceholderText(placeholder);
  layout->addWidget(edit, 1);

  auto* browseBtn = new QPushButton(QObject::tr("浏览"), container);
  browseBtn->setCursor(Qt::PointingHandCursor);
  layout->addWidget(browseBtn);
  QObject::connect(browseBtn, &QPushButton::clicked, container, [edit]() {
    const QString dir = QFileDialog::getExistingDirectory(edit, QObject::tr("选择目录"));
    if (!dir.isEmpty()) {
      edit->setText(dir);
    }
  });
  return container;
}
}

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog{parent}
{
  setModal(true);
  setObjectName(QStringLiteral("SettingsDialogOverlay"));
  setWindowFlag(Qt::FramelessWindowHint);
  setupUI();
  loadSettings();
}

void SettingsDialog::loadSettings()
{
}

void SettingsDialog::saveSettings()
{
}

void SettingsDialog::setupUI()
{
  auto* overlayLayout = new QVBoxLayout(this);
  overlayLayout->setContentsMargins(kDialogPadding, kDialogPadding, kDialogPadding, kDialogPadding);

  auto* container = new QWidget(this);
  container->setObjectName(QStringLiteral("SettingsDialogContainer"));
  container->setMinimumSize(960, 640);
  auto* containerLayout = new QVBoxLayout(container);
  containerLayout->setContentsMargins(0, 0, 0, 0);
  overlayLayout->addWidget(container, 0, Qt::AlignCenter);

  // Header
  auto* header = new QWidget(container);
  header->setObjectName(QStringLiteral("SettingsDialogHeader"));
  auto* headerLayout = new QHBoxLayout(header);
  headerLayout->setContentsMargins(24, 16, 24, 16);

  auto* titleLabel = new QLabel(tr("系统设置"), header);
  titleLabel->setObjectName(QStringLiteral("SettingsDialogTitle"));
  headerLayout->addWidget(titleLabel);
  headerLayout->addStretch();

  auto* closeButton = new QPushButton(header);
  closeButton->setObjectName(QStringLiteral("SettingsDialogCloseButton"));
  closeButton->setText(QStringLiteral("X"));
  closeButton->setFlat(true);
  closeButton->setCursor(Qt::PointingHandCursor);
  headerLayout->addWidget(closeButton);
  containerLayout->addWidget(header);

  // Main body
  auto* body = new QWidget(container);
  body->setObjectName(QStringLiteral("SettingsDialogBody"));
  auto* bodyLayout = new QHBoxLayout(body);
  bodyLayout->setContentsMargins(0, 0, 0, 0);
  bodyLayout->setSpacing(0);
  containerLayout->addWidget(body, 1);

  // Left navigation
  auto* navPanel = new QWidget(body);
  navPanel->setObjectName(QStringLiteral("SettingsNavPanel"));
  navPanel->setMinimumWidth(220);
  auto* navLayout = new QVBoxLayout(navPanel);
  navLayout->setContentsMargins(16, 16, 16, 16);

  auto* navLabel = new QLabel(tr("配置列表"), navPanel);
  navLabel->setObjectName(QStringLiteral("SettingsNavLabel"));
  navLayout->addWidget(navLabel);

  auto* divider = new QFrame(navPanel);
  divider->setFrameShape(QFrame::HLine);
  divider->setObjectName(QStringLiteral("SettingsNavDivider"));
  navLayout->addWidget(divider);

  auto* navButtonsContainer = new QWidget(navPanel);
  auto* navButtonsLayout = new QVBoxLayout(navButtonsContainer);
  navButtonsLayout->setContentsMargins(0, 12, 0, 12);
  navButtonsLayout->setSpacing(8);
  navLayout->addWidget(navButtonsContainer, 1);
  navLayout->addStretch();

  // Right content
  auto* contentPanel = new QWidget(body);
  contentPanel->setObjectName(QStringLiteral("SettingsContentPanel"));
  auto* contentLayout = new QVBoxLayout(contentPanel);
  contentLayout->setContentsMargins(0, 0, 0, 0);

  auto* contentHeader = new QWidget(contentPanel);
  contentHeader->setObjectName(QStringLiteral("SettingsContentHeader"));
  auto* contentHeaderLayout = new QHBoxLayout(contentHeader);
  contentHeaderLayout->setContentsMargins(24, 16, 24, 16);
  contentHeaderLayout->setSpacing(12);

  m_sectionIconLabel = new QLabel(contentHeader);
  m_sectionIconLabel->setObjectName(QStringLiteral("SettingsSectionIcon"));
  m_sectionIconLabel->setVisible(false);
  contentHeaderLayout->addWidget(m_sectionIconLabel);

  m_sectionTitleLabel = new QLabel(contentHeader);
  m_sectionTitleLabel->setObjectName(QStringLiteral("SettingsSectionTitle"));
  contentHeaderLayout->addWidget(m_sectionTitleLabel);
  contentHeaderLayout->addStretch();

  contentLayout->addWidget(contentHeader);

  m_stackedWidget = new QStackedWidget(contentPanel);
  m_stackedWidget->setObjectName(QStringLiteral("SettingsStack"));
  contentLayout->addWidget(m_stackedWidget, 1);

  // Footer
  auto* footer = new QWidget(contentPanel);
  footer->setObjectName(QStringLiteral("SettingsFooter"));
  auto* footerLayout = new QHBoxLayout(footer);
  footerLayout->setContentsMargins(24, 16, 24, 16);

  footerLayout->addStretch();

  auto* restoreBtn = new QPushButton(tr("恢复默认"), footer);
  restoreBtn->setObjectName(QStringLiteral("SettingsRestoreButton"));
  connect(restoreBtn, &QPushButton::clicked, this, &SettingsDialog::onRestoreDefaultClicked);
  footerLayout->addWidget(restoreBtn);

  auto* applyBtn = new QPushButton(tr("应用"), footer);
  applyBtn->setObjectName(QStringLiteral("SettingsApplyButton"));
  connect(applyBtn, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
  footerLayout->addWidget(applyBtn);

  auto* cancelBtn = new QPushButton(tr("取消"), footer);
  cancelBtn->setObjectName(QStringLiteral("SettingsCancelButton"));
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
  footerLayout->addWidget(cancelBtn);

  auto* okBtn = new QPushButton(tr("确认"), footer);
  okBtn->setObjectName(QStringLiteral("SettingsOkButton"));
  okBtn->setDefault(true);
  connect(okBtn, &QPushButton::clicked, this, [this] {
    saveSettings();
    accept();
  });
  footerLayout->addWidget(okBtn);

  contentLayout->addWidget(footer);

  bodyLayout->addWidget(navPanel);
  bodyLayout->addWidget(contentPanel, 1);

  buildSections();
  for (int i = 0; i < m_sections.size(); ++i) {
    auto& section = m_sections[i];
    section.page = createSectionWidget(section.id);
    m_stackedWidget->addWidget(section.page);
    section.button = createNavButton(i);
    navButtonsLayout->addWidget(section.button);
  }

  setActiveSection(0);

  connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::buildSections()
{
  m_sections = {
      {QStringLiteral("camera"), tr("相机设置"), QStringLiteral(":/resources/icons/camera.svg")},
      {QStringLiteral("light"), tr("光源控制"), QStringLiteral(":/resources/icons/light.svg")},
      {QStringLiteral("plc"), tr("PLC 通信"), QStringLiteral(":/resources/icons/plc.svg")},
      {QStringLiteral("storage"), tr("存储策略"), QStringLiteral(":/resources/icons/storage.svg")},
      {QStringLiteral("detection"), tr("检测参数"), QStringLiteral(":/resources/icons/detect.svg")},
      {QStringLiteral("user"), tr("用户权限"), QStringLiteral(":/resources/icons/user.svg")}};
}

QWidget* SettingsDialog::createSectionWidget(const QString& id)
{
  if (id == QLatin1String("camera"))
    return createCameraPage();
  if (id == QLatin1String("light"))
    return createLightPage();
  if (id == QLatin1String("plc"))
    return createPLCPage();
  if (id == QLatin1String("storage"))
    return createStoragePage();
  if (id == QLatin1String("detection"))
    return createDetectorPage();
  if (id == QLatin1String("user"))
    return createUserPage();
  auto* page = new QWidget();
  return page;
}

QPushButton* SettingsDialog::createNavButton(int index)
{
  auto& section = m_sections[index];
  auto* button = new QPushButton(section.title);
  button->setObjectName(QStringLiteral("SettingsNavButton"));
  button->setCheckable(true);
  button->setCursor(Qt::PointingHandCursor);
  button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  if (!section.icon.isEmpty()) {
    button->setIcon(QIcon(section.icon));
  }
  connect(button, &QPushButton::clicked, this, [this, index]() { setActiveSection(index); });
  return button;
}

void SettingsDialog::setActiveSection(int index)
{
  if (index < 0 || index >= m_sections.size())
    return;
  if (m_activeSection == index)
    return;

  m_activeSection = index;
  m_stackedWidget->setCurrentIndex(index);

  const auto& section = m_sections[index];
  m_sectionTitleLabel->setText(section.title);
  if (!section.icon.isEmpty()) {
    m_sectionIconLabel->setPixmap(QIcon(section.icon).pixmap(20, 20));
    m_sectionIconLabel->setVisible(true);
  } else {
    m_sectionIconLabel->setVisible(false);
  }

  for (int i = 0; i < m_sections.size(); ++i) {
    if (m_sections[i].button) {
      m_sections[i].button->setChecked(i == index);
    }
  }
}

void SettingsDialog::onRestoreDefaultClicked()
{
  const auto reply = QMessageBox::question(this, tr("提示"), tr("确认恢复默认设置？"));
  if (reply == QMessageBox::Yes) {
    QMessageBox::information(this, tr("提示"), tr("已恢复默认设置"));
  }
}

void SettingsDialog::onApplyClicked()
{
  QMessageBox::information(this, tr("提示"), tr("设置已应用"));
  emit settingsChanged();
}

QWidget* SettingsDialog::createCameraPage()
{
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(24, 24, 24, 24);
  layout->setSpacing(16);

  auto* configCard = createCard(page, tr("相机参数"));
  auto* grid = new QGridLayout();
  grid->setHorizontalSpacing(12);
  grid->setVerticalSpacing(10);
  configCard->layout()->addItem(grid);

  auto addLabel = [&](int row, const QString& text) {
    auto* label = new QLabel(text, configCard);
    grid->addWidget(label, row, 0);
  };

  addLabel(0, tr("相机类型:"));
  auto* typeCombo = new QComboBox(configCard);
  typeCombo->addItems({tr("GigE Vision"), tr("USB3 Vision"), tr("海康 SDK"), tr("大恒 SDK"), tr("离线调试")});
  grid->addWidget(typeCombo, 0, 1, 1, 2);

  addLabel(1, tr("相机 IP:"));
  auto* ipContainer = new QWidget(configCard);
  auto* ipLayout = new QHBoxLayout(ipContainer);
  ipLayout->setContentsMargins(0, 0, 0, 0);
  ipLayout->setSpacing(6);
  auto* ipEdit = new QLineEdit(configCard);
  ipEdit->setPlaceholderText(QStringLiteral("192.168.1.100"));
  ipLayout->addWidget(ipEdit, 1);
  auto* scanBtn = new QPushButton(tr("扫描"), configCard);
  scanBtn->setCursor(Qt::PointingHandCursor);
  ipLayout->addWidget(scanBtn);
  grid->addWidget(ipContainer, 1, 1, 1, 2);

  addLabel(2, tr("曝光时间:"));
  auto* exposureSlider = new QSlider(Qt::Horizontal, configCard);
  exposureSlider->setRange(100, 100000);
  exposureSlider->setValue(5000);
  grid->addWidget(exposureSlider, 2, 1);
  auto* exposureSpin = new QSpinBox(configCard);
  exposureSpin->setRange(100, 100000);
  exposureSpin->setValue(5000);
  exposureSpin->setSuffix(tr(" μs"));
  grid->addWidget(exposureSpin, 2, 2);
  connect(exposureSlider, &QSlider::valueChanged, exposureSpin, &QSpinBox::setValue);
  connect(exposureSpin, QOverload<int>::of(&QSpinBox::valueChanged), exposureSlider, &QSlider::setValue);

  addLabel(3, tr("增益:"));
  auto* gainSlider = new QSlider(Qt::Horizontal, configCard);
  gainSlider->setRange(0, 24);
  grid->addWidget(gainSlider, 3, 1);
  auto* gainSpin = new QSpinBox(configCard);
  gainSpin->setRange(0, 24);
  gainSpin->setSuffix(tr(" dB"));
  grid->addWidget(gainSpin, 3, 2);
  connect(gainSlider, &QSlider::valueChanged, gainSpin, &QSpinBox::setValue);
  connect(gainSpin, QOverload<int>::of(&QSpinBox::valueChanged), gainSlider, &QSlider::setValue);

  layout->addWidget(configCard);

  auto* actionCard = createCard(page, tr("连接与测试"));
  auto* actionLayout = new QHBoxLayout();
  actionLayout->setSpacing(12);
  actionCard->layout()->addItem(actionLayout);
  for (const QString& text : {tr("连接相机"), tr("测试相机"), tr("单帧采集")}) {
    auto* btn = new QPushButton(text, actionCard);
    btn->setCursor(Qt::PointingHandCursor);
    actionLayout->addWidget(btn);
  }
  actionLayout->addStretch();
  layout->addWidget(actionCard);

  auto* previewCard = createCard(page, tr("图像预览"));
  auto* previewFrame = new QFrame(previewCard);
  previewFrame->setObjectName(QStringLiteral("SettingsPreviewBox"));
  previewFrame->setMinimumHeight(220);
  previewCard->layout()->addWidget(previewFrame);
  layout->addWidget(previewCard);

  layout->addStretch();
  return page;
}

QWidget* SettingsDialog::createLightPage()
{
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(24, 24, 24, 24);
  layout->setSpacing(16);

  auto* card = createCard(page, tr("光源控制"));
  auto* form = new QFormLayout();
  form->setSpacing(12);
  card->layout()->addItem(form);

  auto* brightnessSlider = new QSlider(Qt::Horizontal, card);
  brightnessSlider->setRange(0, 100);
  form->addRow(tr("亮度"), brightnessSlider);

  auto* temperatureSlider = new QSlider(Qt::Horizontal, card);
  temperatureSlider->setRange(2700, 6000);
  form->addRow(tr("色温"), temperatureSlider);

  auto* pulseSpin = new QSpinBox(card);
  pulseSpin->setRange(10, 1000);
  pulseSpin->setSuffix(tr(" μs"));
  form->addRow(tr("脉冲宽度"), pulseSpin);

  layout->addWidget(card);

  auto* testCard = createCard(page, tr("快速测试"));
  auto* btnLayout = new QHBoxLayout();
  btnLayout->setSpacing(12);
  testCard->layout()->addItem(btnLayout);
  for (const QString& text : {tr("打开光源"), tr("关闭光源")}) {
    auto* btn = new QPushButton(text, testCard);
    btn->setCursor(Qt::PointingHandCursor);
    btnLayout->addWidget(btn);
  }
  btnLayout->addStretch();
  layout->addWidget(testCard);

  layout->addStretch();
  return page;
}

QWidget* SettingsDialog::createPLCPage()
{
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(24, 24, 24, 24);
  layout->setSpacing(16);

  auto* card = createCard(page, tr("PLC 通信"));
  auto* form = new QFormLayout();
  form->setSpacing(12);
  card->layout()->addItem(form);

  auto* protocolCombo = new QComboBox(card);
  protocolCombo->addItems({tr("Modbus TCP"), tr("Modbus RTU"), tr("西门子 S7"), tr("三菱 MC"), tr("欧姆龙 FINS")});
  form->addRow(tr("通讯协议"), protocolCombo);

  auto* ipEdit = new QLineEdit(card);
  ipEdit->setPlaceholderText(QStringLiteral("192.168.1.10"));
  form->addRow(tr("IP 地址"), ipEdit);

  auto* portSpin = new QSpinBox(card);
  portSpin->setRange(1, 65535);
  portSpin->setValue(502);
  form->addRow(tr("端口"), portSpin);

  auto* slaveSpin = new QSpinBox(card);
  slaveSpin->setRange(1, 255);
  slaveSpin->setValue(1);
  form->addRow(tr("站号"), slaveSpin);

  auto* timeoutSpin = new QSpinBox(card);
  timeoutSpin->setRange(100, 10000);
  timeoutSpin->setValue(1000);
  timeoutSpin->setSuffix(tr(" ms"));
  form->addRow(tr("超时"), timeoutSpin);

  layout->addWidget(card);

  auto* diagnoseCard = createCard(page, tr("诊断工具"));
  auto* diagLayout = new QHBoxLayout();
  diagLayout->setSpacing(12);
  diagnoseCard->layout()->addItem(diagLayout);
  for (const QString& text : {tr("连接测试"), tr("读寄存器"), tr("写寄存器")}) {
    auto* btn = new QPushButton(text, diagnoseCard);
    btn->setCursor(Qt::PointingHandCursor);
    diagLayout->addWidget(btn);
  }
  diagLayout->addStretch();
  layout->addWidget(diagnoseCard);

  layout->addStretch();
  return page;
}

QWidget* SettingsDialog::createStoragePage()
{
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(24, 24, 24, 24);
  layout->setSpacing(16);

  auto* card = createCard(page, tr("存储策略"));
  auto* form = new QFormLayout();
  form->setSpacing(12);
  card->layout()->addItem(form);

  form->addRow(tr("图像目录"), createPathEditor(card, tr("./data/images")));

  auto* dbCombo = new QComboBox(card);
  dbCombo->addItems({tr("SQLite"), tr("MySQL"), tr("PostgreSQL")});
  form->addRow(tr("数据库"), dbCombo);

  auto* reserveSpin = new QSpinBox(card);
  reserveSpin->setRange(1, 365);
  reserveSpin->setValue(30);
  reserveSpin->setSuffix(tr(" 天"));
  form->addRow(tr("保留天数"), reserveSpin);

  auto* autoClean = new QCheckBox(tr("启用自动清理"), card);
  autoClean->setChecked(true);
  form->addRow(QString(), autoClean);

  layout->addWidget(card);
  layout->addStretch();
  return page;
}

QWidget* SettingsDialog::createDetectorPage()
{
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(24, 24, 24, 24);
  layout->setSpacing(16);

  auto* card = createCard(page, tr("检测器配置"));
  auto* vLayout = new QVBoxLayout();
  vLayout->setSpacing(8);
  card->layout()->addItem(vLayout);

  const QStringList detectors = {tr("划痕检测"), tr("裂纹检测"), tr("异物检测"), tr("尺寸测量")};
  for (const auto& det : detectors) {
    auto* check = new QCheckBox(det, card);
    check->setChecked(true);
    vLayout->addWidget(check);
  }

  auto* thresholdLabel = new QLabel(tr("严重度阈值"), card);
  vLayout->addWidget(thresholdLabel);
  auto* thresholdSlider = new QSlider(Qt::Horizontal, card);
  thresholdSlider->setRange(0, 100);
  thresholdSlider->setValue(75);
  vLayout->addWidget(thresholdSlider);

  layout->addWidget(card);
  layout->addStretch();
  return page;
}

QWidget* SettingsDialog::createUserPage()
{
  auto* page = new QWidget();
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(24, 24, 24, 24);
  layout->setSpacing(16);

  auto* currentCard = createCard(page, tr("当前用户"));
  auto* currentLayout = new QHBoxLayout();
  currentLayout->setSpacing(12);
  currentCard->layout()->addItem(currentLayout);
  auto* avatar = new QLabel(tr("A"), currentCard);
  avatar->setObjectName(QStringLiteral("UserAvatarBadge"));
  avatar->setAlignment(Qt::AlignCenter);
  avatar->setFixedSize(48, 48);
  currentLayout->addWidget(avatar);
  auto* infoLayout = new QVBoxLayout();
  infoLayout->addWidget(new QLabel(tr("admin"), currentCard));
  infoLayout->addWidget(new QLabel(tr("管理员 - 全部权限"), currentCard));
  currentLayout->addLayout(infoLayout);
  layout->addWidget(currentCard);

  auto* tableCard = createCard(page, tr("用户列表"));
  auto* tableLayout = new QVBoxLayout();
  tableLayout->setSpacing(8);
  tableCard->layout()->addItem(tableLayout);
  auto* addBtn = new QPushButton(tr("新增用户"), tableCard);
  addBtn->setCursor(Qt::PointingHandCursor);
  tableLayout->addWidget(addBtn, 0, Qt::AlignRight);
  auto* table = new QTableWidget(3, 4, tableCard);
  table->setHorizontalHeaderLabels({tr("用户名"), tr("角色"), tr("状态"), tr("操作")});
  table->verticalHeader()->setVisible(false);
  table->horizontalHeader()->setStretchLastSection(true);
  tableLayout->addWidget(table);

  const QStringList names = {QStringLiteral("admin"), QStringLiteral("operator1"), QStringLiteral("engineer")};
  const QStringList roles = {tr("管理员"), tr("操作员"), tr("工程师")};
  const QStringList status = {tr("在线"), tr("在线"), tr("离线")};
  for (int row = 0; row < table->rowCount(); ++row) {
    table->setItem(row, 0, new QTableWidgetItem(names[row]));
    table->setItem(row, 1, new QTableWidgetItem(roles[row]));
    table->setItem(row, 2, new QTableWidgetItem(status[row]));
    auto* actions = new QWidget(table);
    auto* actionLayout = new QHBoxLayout(actions);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(4);
    auto* editBtn = new QPushButton(tr("编辑"), table);
    auto* delBtn = new QPushButton(tr("删除"), table);
    editBtn->setCursor(Qt::PointingHandCursor);
    delBtn->setCursor(Qt::PointingHandCursor);
    actionLayout->addWidget(editBtn);
    actionLayout->addWidget(delBtn);
    table->setCellWidget(row, 3, actions);
  }
  layout->addWidget(tableCard);

  auto* roleCard = createCard(page, tr("角色权限"));
  auto* roleLayout = new QGridLayout();
  roleLayout->setHorizontalSpacing(12);
  roleLayout->setVerticalSpacing(8);
  const QStringList perms = {tr("查看报表"), tr("修改参数"), tr("导出数据"),
                             tr("远程维护"), tr("模型管理"), tr("用户管理")};
  for (int i = 0; i < perms.size(); ++i) {
    auto* check = new QCheckBox(perms[i], roleCard);
    check->setChecked(i < 4);
    roleLayout->addWidget(check, i / 2, i % 2);
  }
  roleCard->layout()->addItem(roleLayout);
  layout->addWidget(roleCard);

  layout->addStretch();
  return page;
}
