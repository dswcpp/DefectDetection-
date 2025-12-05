#ifndef DROIDMESSAGEBOX_H
#define DROIDMESSAGEBOX_H

#include "FramelessDialog.h"
#include "ui_global.h"

class QLabel;
class QPushButton;

class UI_LIBRARY DroidMessageBox : public FramelessDialog {
    Q_OBJECT
public:
    enum Icon {
        NoIcon,
        Information,
        Warning,
        Critical,
        Question
    };

    enum StandardButton {
        NoButton = 0x00000000,
        Ok = 0x00000400,
        Cancel = 0x00400000,
        Yes = 0x00004000,
        No = 0x00010000
    };
    Q_DECLARE_FLAGS(StandardButtons, StandardButton)

    explicit DroidMessageBox(QWidget* parent = nullptr);
    ~DroidMessageBox() = default;

    void setIcon(Icon icon);
    void setText(const QString& text);
    void setInformativeText(const QString& text);
    void setStandardButtons(StandardButtons buttons);

    StandardButton clickedButton() const { return m_clickedButton; }

    // 静态便捷方法
    static StandardButton information(QWidget* parent, const QString& title, const QString& text,
                                       StandardButtons buttons = StandardButtons(Ok));
    static StandardButton warning(QWidget* parent, const QString& title, const QString& text,
                                   StandardButtons buttons = StandardButtons(Ok));
    static StandardButton critical(QWidget* parent, const QString& title, const QString& text,
                                    StandardButtons buttons = StandardButtons(Ok));
    static StandardButton question(QWidget* parent, const QString& title, const QString& text,
                                    StandardButtons buttons = StandardButtons(Yes | No));

private:
    void setupUI();
    void addButton(StandardButton button);
    QString buttonText(StandardButton button) const;

    QLabel* m_iconLabel = nullptr;
    QLabel* m_textLabel = nullptr;
    QLabel* m_informativeLabel = nullptr;
    QWidget* m_buttonContainer = nullptr;
    StandardButton m_clickedButton = NoButton;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DroidMessageBox::StandardButtons)

#endif // DROIDMESSAGEBOX_H
