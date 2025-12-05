#ifndef TOAST_H
#define TOAST_H

#include <QWidget>
#include "ui_global.h"

class QLabel;
class QTimer;

class UI_LIBRARY Toast : public QWidget {
    Q_OBJECT
public:
    enum Type {
        Information,
        Success,
        Warning,
        Error
    };

    explicit Toast(QWidget* parent = nullptr);
    ~Toast() = default;

    void setType(Type type);
    void setText(const QString& text);
    void setDuration(int ms);

    // 静态便捷方法
    static void show(QWidget* parent, const QString& text, Type type = Information, int duration = 2000);
    static void info(QWidget* parent, const QString& text, int duration = 2000);
    static void success(QWidget* parent, const QString& text, int duration = 2000);
    static void warning(QWidget* parent, const QString& text, int duration = 2000);
    static void error(QWidget* parent, const QString& text, int duration = 2000);

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();
    void updateStyle();
    void positionToast();

    QLabel* m_iconLabel = nullptr;
    QLabel* m_textLabel = nullptr;
    QTimer* m_timer = nullptr;
    Type m_type = Information;
    int m_duration = 2000;
};

#endif // TOAST_H
