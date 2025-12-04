#include "FramelessWindow.h"
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>

FramelessWindow::FramelessWindow(QWidget* parent) : QWidget{parent}
{
    setWindowFlags(Qt::WindowStaysOnTopHint);
    // 去除标题栏
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    leftPressed = false;
    leftPressedInTitle = false;

    // // 拖动缩放
    hoverPos = CENTER;
    padding = 4;

    // 默认只允许拖动标题栏，来移动窗口
    onlyMoveByTitlebar = true;

    // 默认允许窗口缩放
    canResize = true;

    this->installEventFilter(this);
}

void FramelessWindow::setTitlebarWidgets(QVector<QWidget*> widgets)
{
    titlebarWidgets = widgets;
}

void FramelessWindow::setOnlyMoveByTitlebar(bool b)
{
    onlyMoveByTitlebar = b;
}

void FramelessWindow::setResizeable(bool b)
{
    canResize = b;
}

void FramelessWindow::setAllWidgetMouseTracking(QWidget* widget)
{
    widget->setMouseTracking(true);

    QObjectList list = widget->children();  // typedef QList<QObject*> QObjectList;
    foreach (QObject* obj, list) {
        //        qDebug() << obj->metaObject()->className();
        if ( obj->metaObject()->className() == QStringLiteral("QWidget") ) {
            QWidget* w = (QWidget*)obj;
            w->setMouseTracking(true);
            setAllWidgetMouseTracking(w);
        }
    }
}

/* 当前窗口绘制时，给窗口中的所有控件设置鼠标追踪。
 * 这样鼠标在移动到边界时，可以追踪得到，从而设置光标为缩放形状
 */
bool FramelessWindow::eventFilter(QObject* target, QEvent* event)
{
    if ( event->type() == QEvent::Paint ) {
        static bool init = false;
        if ( !init ) {
            init = true;
            setAllWidgetMouseTracking(this);
        }
    }

    // 查看源码可知，父类的实现就是 return false，表示让事件接着传递，也就是传递给对应的控件
    return QWidget::eventFilter(target, event);
}

void FramelessWindow::setCursorShape(const QPoint& point)
{
    QRect rect = this->rect();
#if 0
    // topLeft= QPoint(0,0) , bottomRight= QPoint(599,399)
    qDebug() << " topLeft=" << rect.topLeft() << ", bottomRight=" << rect.bottomRight();
#endif
    QPoint topLeft = mapToGlobal(rect.topLeft());
    QPoint bottomRight = mapToGlobal(rect.bottomRight());

    int x = point.x();
    int y = point.y();

    if ( x >= topLeft.x() && x <= topLeft.x() + padding && y >= topLeft.y() && y <= topLeft.y() + padding ) {
        // 左上角
        hoverPos = TOP_LEFT;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if ( x <= bottomRight.x() && x >= bottomRight.x() - padding && y <= bottomRight.y() && y >= bottomRight.y() - padding ) {
        // 右下角
        hoverPos = BOTTOM_RIGHT;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if ( x >= topLeft.x() && x <= topLeft.x() + padding && y <= bottomRight.y() && y >= bottomRight.y() - padding ) {
        // 左下角
        hoverPos = BOTTOM_LEFT;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if ( x <= bottomRight.x() && x >= bottomRight.x() - padding && y >= topLeft.y() && y <= topLeft.y() + padding ) {
        // 右上角
        hoverPos = TOP_RIGHT;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if ( x >= topLeft.x() && x <= topLeft.x() + padding ) {
        // 左边
        hoverPos = LEFT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    } else if ( x <= bottomRight.x() && x >= bottomRight.x() - padding ) {
        // 右边
        hoverPos = RIGHT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    } else if ( y >= topLeft.y() && y <= topLeft.y() + padding ) {
        // 上边
        hoverPos = TOP;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    } else if ( y <= bottomRight.y() && y >= bottomRight.y() - padding ) {
        // 下边
        hoverPos = BOTTOM;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    } else {
        // 中间
        hoverPos = CENTER;
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}

void FramelessWindow::mousePressEvent(QMouseEvent* event)
{
    // 如果不是左键，直接返回
    if ( event->button() != Qt::LeftButton ) {
        return;
    }

    leftPressed = true;

    wndPos = this->pos();           // 记录当前窗体的位置，也就是窗体左上角的坐标
    pressPos = event->globalPosition().toPoint();  // 记录鼠标按下的位置

    // 判断左键按下的位置是否在标题栏中
    QWidget* pressedWidget = QApplication::widgetAt(event->globalPosition().toPoint());
    //    qDebug() << "pressedWidget: " << pressedWidget << ", " << event->globalPos();
    if ( pressedWidget ) {
        foreach (QWidget* widget, titlebarWidgets) {
            if ( pressedWidget == widget ) {
                leftPressedInTitle = true;
                break;
            }
        }
    }
}

void FramelessWindow::mouseMoveEvent(QMouseEvent* event)
{
    QPoint globalPos = event->globalPosition().toPoint();

    // 1. 左键未按下
    if ( !leftPressed ) {
        // 窗口不是最大化状态 && 窗口可缩放，则光标移动到边界时，要变成缩放的形状（窗口处于最大化状态时，就无须改变光标形状了）
        if ( this->windowState().testFlag(Qt::WindowNoState) && canResize ) {
            setCursorShape(globalPos);
        }
        return;
    }

    // 2. 左键按下时
    if ( hoverPos != CENTER ) {
        // 2.1 在边界处按下
        QRect rect = this->rect();
        QPoint topLeft = mapToGlobal(rect.topLeft());
        QPoint bottomRight = mapToGlobal(rect.bottomRight());

        QRect rMove(topLeft, bottomRight);

        switch ( hoverPos ) {
            case TOP:
                // 如果不加if判断，则窗口高度达到最小高度后，会被鼠标 "向下推走"
                if ( bottomRight.y() - globalPos.y() > this->minimumHeight() ) {
                    rMove.setY(globalPos.y());
                }
                break;
            case BOTTOM:
                rMove.setHeight(globalPos.y() - topLeft.y());
                break;
            case LEFT:
                // 如果不加if判断，则窗口高度达到最小宽度后，会被鼠标 "向右推走"
                if ( bottomRight.x() - globalPos.x() > this->minimumWidth() ) {
                    rMove.setX(globalPos.x());
                }
                break;
            case RIGHT:
                rMove.setWidth(globalPos.x() - topLeft.x());
                break;
            case TOP_LEFT:
                if ( bottomRight.y() - globalPos.y() > this->minimumHeight() ) {
                    rMove.setY(globalPos.y());
                }
                if ( bottomRight.x() - globalPos.x() > this->minimumWidth() ) {
                    rMove.setX(globalPos.x());
                }
                break;
            case TOP_RIGHT:
                if ( bottomRight.y() - globalPos.y() > this->minimumHeight() ) {
                    rMove.setY(globalPos.y());
                }
                rMove.setWidth(globalPos.x() - topLeft.x());
                break;
            case BOTTOM_LEFT:
                rMove.setHeight(globalPos.y() - topLeft.y());
                if ( bottomRight.x() - globalPos.x() > this->minimumWidth() ) {
                    rMove.setX(globalPos.x());
                }
                break;
            case BOTTOM_RIGHT:
                rMove.setHeight(globalPos.y() - topLeft.y());
                rMove.setWidth(globalPos.x() - topLeft.x());
                break;
            default:
                break;
        }
        this->setGeometry(rMove);
    } else {
        // 2.2 在非边界处按下
        if ( leftPressedInTitle ) {
            // 2.2.1 在标题栏内按下
            if ( this->isMaximized() ) {
                // 窗口最大化时鼠标拖动标题栏需要完成两个操作：
                // a. 窗口恢复
                // b. 鼠标相对窗口的相对位置不变
                //    相对位置不变指的是：鼠标点击拖动窗口1/4处进行拖动，复原时鼠标依然位于窗口1/4处
                // 达到此效果，仅需更改窗口的位置即可

                // 计算全屏时，鼠标在 x 轴上，相对于屏幕宽度的百分比
                float width_ratio = float(event->globalPosition().toPoint().x()) / float(this->width());
                qDebug() << "ratio=" << width_ratio;
                qDebug() << "before width = " << this->width();  // 1920
                // a. 窗口恢复
                this->setWindowState(Qt::WindowNoState);
                qDebug() << "after width = " << this->width();  // 600

                // b. 鼠标相对窗口的相对位置不变
                // 和双击时一样，默认会回到双击效果时的位置，所以要修改窗口的位置
                int offset = this->width() * width_ratio;  // 当前窗口相对于鼠标位置的偏移

                wndPos.setX(event->globalPosition().toPoint().x() - offset);
                wndPos.setY(0);
            } else {
                this->move(wndPos + (event->globalPosition().toPoint() - pressPos));
            }
        } else {
            // 2.2.2 在主体内按下
            if ( !onlyMoveByTitlebar && !this->isMaximized() ) {
                this->move(wndPos + (event->globalPosition().toPoint() - pressPos));
            }
        }
    }
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    leftPressed = false;
    leftPressedInTitle = false;
}

void FramelessWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    if ( canResize ) {
        QWidget* clickedWidget = QApplication::widgetAt(event->globalPosition().toPoint());
        //        qDebug() << "clickedWidget: " << clickedWidget << ", " << event->globalPos();
        if ( clickedWidget ) {
            bool inTitle = false;
            foreach (QWidget* widget, titlebarWidgets) {
                if ( clickedWidget == widget ) {
                    inTitle = true;
                    break;
                }
            }

            if ( inTitle ) {
                this->setWindowState(isMaximized() ? Qt::WindowNoState : Qt::WindowMaximized);
            }
        }
    }
}
