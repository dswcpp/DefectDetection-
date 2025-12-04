#ifndef FRAMELESSDIALOG_H
#define FRAMELESSDIALOG_H

#include <QDialog>
#include <QVector>
#include "ui_global.h"

class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

enum DialogLocation { 
    DLG_TOP, DLG_BOTTOM, DLG_LEFT, DLG_RIGHT, 
    DLG_TOP_LEFT, DLG_TOP_RIGHT, DLG_BOTTOM_LEFT, DLG_BOTTOM_RIGHT, DLG_CENTER 
};

class UI_LIBRARY FramelessDialog : public QDialog {
    Q_OBJECT
public:
    explicit FramelessDialog(QWidget* parent = nullptr);
    virtual ~FramelessDialog() = default;

    void setDialogTitle(const QString& title);
    void setDialogIcon(const QIcon& icon);
    void setResizeable(bool resizable);
    void setShowMinButton(bool show);
    void setShowMaxButton(bool show);

protected:
    QWidget* contentWidget() const { return m_contentWidget; }
    QVBoxLayout* contentLayout() const { return m_contentLayout; }
    
    void setTitlebarWidgets(const QVector<QWidget*>& widgets);
    void setAllWidgetMouseTracking(QWidget* widget);
    
    bool eventFilter(QObject* target, QEvent* event) override;
    void setCursorShape(const QPoint& point);

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    void setupFramelessUI();
    void applyDialogStyle();
    void updateMaximizeButton();

private:
    // 标题栏组件
    QWidget* m_titleBar = nullptr;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    QPushButton* m_minButton = nullptr;
    QPushButton* m_maxButton = nullptr;
    QPushButton* m_closeButton = nullptr;
    
    // 内容区域
    QWidget* m_contentWidget = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;

    // 拖动和缩放状态
    bool m_leftPressed = false;
    bool m_leftPressedInTitle = false;
    QVector<QWidget*> m_titlebarWidgets;
    QPoint m_pressPos;
    QPoint m_wndPos;
    
    int m_padding = 5;
    DialogLocation m_hoverPos = DLG_CENTER;
    bool m_canResize = true;
    bool m_showMinBtn = false;
    bool m_showMaxBtn = false;
};

#endif // FRAMELESSDIALOG_H
