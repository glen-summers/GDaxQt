#ifndef EXPANDBUTTON_H
#define EXPANDBUTTON_H

#include <QToolButton>

class ExpandButton : public QToolButton
{
    QWidget * controlled;
    Q_OBJECT

public:
    explicit ExpandButton(QWidget *parent = nullptr);
    void setControlled(QWidget * widget);

private:
    void Toggle();
};

#endif // EXPANDBUTTON_H
