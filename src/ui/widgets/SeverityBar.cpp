#include "SeverityBar.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>

SeverityBar::SeverityBar(QWidget *parent) : QWidget{parent} {
  setMinimumHeight(20);
  setMaximumHeight(32);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void SeverityBar::setValue(double value) {
  value = qBound(0.0, value, m_maxValue);
  if (qFuzzyCompare(m_value, value)) return;
  m_value = value;
  update();
  emit valueChanged(m_value);
}

void SeverityBar::setMaxValue(double max) {
  if (max <= 0) max = 1.0;
  m_maxValue = max;
  m_value = qMin(m_value, m_maxValue);
  update();
}

void SeverityBar::setLevel(const QString& level) {
  if (m_level == level) return;
  m_level = level;
  update();
  emit levelChanged(m_level);
}

void SeverityBar::setDisplayMode(DisplayMode mode) {
  if (m_displayMode == mode) return;
  m_displayMode = mode;
  update();
}

void SeverityBar::setShowText(bool show) {
  if (m_showText == show) return;
  m_showText = show;
  update();
}

QSize SeverityBar::sizeHint() const {
  return QSize(150, 24);
}

QSize SeverityBar::minimumSizeHint() const {
  return QSize(60, 16);
}

void SeverityBar::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event)

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QRect barRect = rect().adjusted(1, 1, -1, -1);
  int radius = barRect.height() / 2;

  // 背景
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(230, 230, 230));
  painter.drawRoundedRect(barRect, radius, radius);

  if (m_displayMode == DisplayMode::Gradient) {
    // 渐变进度条模式
    double ratio = m_maxValue > 0 ? m_value / m_maxValue : 0;
    int fillWidth = static_cast<int>(barRect.width() * ratio);
    
    if (fillWidth > 0) {
      QRect fillRect = barRect;
      fillRect.setWidth(fillWidth);

      QLinearGradient gradient(fillRect.topLeft(), fillRect.topRight());
      gradient.setColorAt(0, QColor(76, 175, 80));    // 绿色
      gradient.setColorAt(0.5, QColor(255, 193, 7));  // 黄色
      gradient.setColorAt(1, QColor(244, 67, 54));    // 红色

      // 根据当前值设置颜色
      QColor fillColor = colorForValue(ratio);
      painter.setBrush(fillColor);
      painter.drawRoundedRect(fillRect, radius, radius);
    }

  } else if (m_displayMode == DisplayMode::Segmented) {
    // 分段模式
    int segmentWidth = barRect.width() / 3;
    QStringList levels = {"Minor", "Major", "Critical"};
    QList<QColor> colors = {
      QColor(76, 175, 80),   // 绿色 - Minor
      QColor(255, 152, 0),   // 橙色 - Major
      QColor(244, 67, 54)    // 红色 - Critical
    };

    int activeIndex = levels.indexOf(m_level);
    
    for (int i = 0; i < 3; ++i) {
      QRect segRect(barRect.x() + i * segmentWidth, barRect.y(),
                    segmentWidth - 2, barRect.height());
      
      if (i == 0) {
        // 左圆角
        QPainterPath path;
        path.addRoundedRect(segRect, radius, radius);
        painter.setClipPath(path);
      } else if (i == 2) {
        // 右圆角
        QPainterPath path;
        path.addRoundedRect(segRect, radius, radius);
        painter.setClipPath(path);
      } else {
        painter.setClipping(false);
      }

      if (i <= activeIndex && activeIndex >= 0) {
        painter.setBrush(colors[i]);
      } else {
        painter.setBrush(QColor(220, 220, 220));
      }
      painter.drawRect(segRect);
    }
    painter.setClipping(false);

  } else if (m_displayMode == DisplayMode::LevelOnly) {
    // 仅等级标签模式
    QColor bgColor = colorForLevel(m_level);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(barRect, radius, radius);
  }

  // 绘制文本
  if (m_showText) {
    QString text = textForDisplay();
    painter.setPen(m_displayMode == DisplayMode::LevelOnly ? Qt::white : QColor(50, 50, 50));
    
    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(true);
    painter.setFont(font);
    
    painter.drawText(barRect, Qt::AlignCenter, text);
  }

  // 边框
  painter.setPen(QPen(QColor(200, 200, 200), 1));
  painter.setBrush(Qt::NoBrush);
  painter.drawRoundedRect(barRect, radius, radius);
}

QColor SeverityBar::colorForValue(double ratio) const {
  // 0.0 = 绿色, 0.5 = 黄色, 1.0 = 红色
  if (ratio <= 0.3) {
    return QColor(76, 175, 80);   // 绿色
  } else if (ratio <= 0.6) {
    // 绿到黄渐变
    double t = (ratio - 0.3) / 0.3;
    int r = 76 + static_cast<int>((255 - 76) * t);
    int g = 175 + static_cast<int>((193 - 175) * t);
    int b = 80 - static_cast<int>(80 * t);
    return QColor(r, g, b);
  } else {
    // 黄到红渐变
    double t = (ratio - 0.6) / 0.4;
    int r = 255 - static_cast<int>((255 - 244) * t);
    int g = 193 - static_cast<int>((193 - 67) * t);
    int b = 7 + static_cast<int>((54 - 7) * t);
    return QColor(r, g, b);
  }
}

QColor SeverityBar::colorForLevel(const QString& level) const {
  if (level == "OK" || level == "None") {
    return QColor(76, 175, 80);    // 绿色
  } else if (level == "Minor") {
    return QColor(255, 193, 7);    // 黄色
  } else if (level == "Major") {
    return QColor(255, 152, 0);    // 橙色
  } else if (level == "Critical") {
    return QColor(244, 67, 54);    // 红色
  }
  return QColor(158, 158, 158);    // 灰色 - 未知
}

QString SeverityBar::textForDisplay() const {
  if (m_displayMode == DisplayMode::LevelOnly) {
    return m_level.isEmpty() ? "-" : m_level;
  } else if (m_displayMode == DisplayMode::Segmented) {
    return m_level.isEmpty() ? "-" : m_level;
  } else {
    // Gradient 模式显示百分比或数值
    if (m_maxValue == 1.0) {
      return QString("%1%").arg(static_cast<int>(m_value * 100));
    } else {
      return QString("%1/%2").arg(m_value, 0, 'f', 1).arg(m_maxValue, 0, 'f', 1);
    }
  }
}
