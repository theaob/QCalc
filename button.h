#ifndef BUTTON_H
#define BUTTON_H

#include <QToolButton>

class Button : public QToolButton
{
    Q_OBJECT
public:
    //explicit Button(QObject *parent = 0);
    Button(const QString &text, QWidget *parent = 0);

    QSize sizeHint() const;

signals:
    
public slots:
    
};

#endif // BUTTON_H
