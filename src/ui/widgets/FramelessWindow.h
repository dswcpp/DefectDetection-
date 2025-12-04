#ifndef FRAMELESSWINDOW_H
#define FRAMELESSWINDOW_H

#include <QWidget>

enum Location { TOP, BOTTOM, LEFT, RIGHT, TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, CENTER };

class FramelessWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FramelessWindow(QWidget* parent = nullptr);

signals:

protected:
    void setTitlebarWidgets(QVector<QWidget*> widgets);

    void setOnlyMoveByTitlebar(bool b);
    void setResizeable(bool b);

    // 用于设置鼠标追踪
    void setAllWidgetMouseTracking(QWidget* widget);
    bool eventFilter(QObject* target, QEvent* event);
    void setCursorShape(const QPoint& point);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    // 双击标题栏放大
    void mouseDoubleClickEvent(QMouseEvent* event);

private:
    bool leftPressed;
    bool leftPressedInTitle;
    QVector<QWidget*> titlebarWidgets;

    QPoint pressPos;
    QPoint wndPos;  // 当前窗体的位置，也就是窗口左上角的坐标

    // 拖动缩放
    int padding;
    Location hoverPos;

    // 只允许标题栏拖动窗口
    bool onlyMoveByTitlebar;

    // 窗口可缩放
    bool canResize;
};
#endif // FRAMELESSWINDOW_H
