#ifndef FRAMELESSMAINWINDOW_H
#define FRAMELESSMAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "ui_global.h"

class QLabel;
class QPushButton;
class QIcon;

enum MainWindowLocation { 
    MW_TOP, MW_BOTTOM, MW_LEFT, MW_RIGHT, 
    MW_TOP_LEFT, MW_TOP_RIGHT, MW_BOTTOM_LEFT, MW_BOTTOM_RIGHT, MW_CENTER 
};

class UI_LIBRARY FramelessMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit FramelessMainWindow(QWidget* parent = nullptr);
    virtual ~FramelessMainWindow() = default;

    void setWindowTitle(const QString& title);
    void setWindowIcon(const QIcon& icon);
    void setResizeable(bool resizable);

signals:
    void windowStateChanged();

protected:
    void setTitlebarWidgets(const QVector<QWidget*>& widgets);
    void setAllWidgetMouseTracking(QWidget* widget);
    
    bool eventFilter(QObject* target, QEvent* event) override;
    void setCursorShape(const QPoint& point);

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void setupTitleBar();
    void updateMaximizeButton();

private:
    // 标题栏组件
    QWidget* m_titleBarWidget = nullptr;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    QPushButton* m_minButton = nullptr;
    QPushButton* m_maxButton = nullptr;
    QPushButton* m_closeButton = nullptr;

    // 初始化标志
    bool m_initialized = false;

    // 拖动和缩放状态
    bool m_leftPressed = false;
    bool m_leftPressedInTitle = false;
    QVector<QWidget*> m_titlebarWidgets;
    QPoint m_pressPos;
    QPoint m_wndPos;
    
    int m_padding = 5;
    MainWindowLocation m_hoverPos = MW_CENTER;
    bool m_canResize = true;
};

#endif // FRAMELESSMAINWINDOW_H
