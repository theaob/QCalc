#include "button.h"

Button::Button(const QString &text, QWidget *parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setText(text);
}
