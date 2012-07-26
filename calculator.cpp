#include <QtGui>
#include <math.h>

#include "calculator.h"

#include "button.h"

Calculator::Calculator(QWidget *parent) :
    QWidget(parent)
{
    sumInMemory = 0.0;
    sumSoFar = 0.0;
    factorSoFar = 0.0;
    waitingForOperand = true;

    display = new QLineEdit("0");
    display->setReadOnly(true);
    display->setAlignment(Qt::AlignRight);
    display->setMaxLength(15);

    QFont font = display->font();
    font.setPointSize(font.pointSize() + 8);
    display->setFont(font);

    for(int i = 0; i < NumDigitButtons; i++)
    {
        digitButtons[i] = createButton(QString::number(i),SLOT(digitClicked()));
    }

    Button *pointButton = createButton(".", SLOT(pointClicked()));
    Button *changeSignButton = createButton("\261", SLOT(changeSignClicked()));

    Button *backspaceButton = createButton("Backspace", SLOT(backspaceClicked()));
    Button *clearButton = createButton("Clear", SLOT(clearClicked()));
    Button *clearAllButton = createButton("Clear All", SLOT(clearAllClicked()));

    Button *clearMemoryButton = createButton("MC", SLOT(clearMemory()));
    Button *readMemoryButton = createButton("MR", SLOT(readMemory()));
    Button *setMemoryButton = createButton("MS", SLOT(setMemory()));
    Button *addToMemoryButton = createButton("M+", SLOT(addToMemory()));

    Button *divisionButton = createButton("\367", SLOT(multiplicativeOperatorClicked()));
    Button *timesButton = createButton("\327", SLOT(multiplicativeOperatorClicked()));
    Button *minusButton = createButton("-", SLOT(additiveOperatorClicked()));
    Button *plusButton = createButton("+", SLOT(additiveOperatorClicked()));

    Button *squareRootButton = createButton("sqrt", SLOT(unaryOperatorClicked()));
    Button *powerButton = createButton("x\262", SLOT(unaryOperatorClicked()));
    Button *reciprocalButton = createButton("1/x", SLOT(unaryOperatorClicked()));
    Button *equalButton = createButton("=", SLOT(equalClicked()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    mainLayout->addWidget(display,0,0,1,6);
    mainLayout->addWidget(backspaceButton,1,0,1,2);
    mainLayout->addWidget(clearButton,1,2,1,2);
    mainLayout->addWidget(clearAllButton,1,4,1,2);

    mainLayout->addWidget(clearMemoryButton,2,0);
    mainLayout->addWidget(readMemoryButton,3,0);
    mainLayout->addWidget(setMemoryButton,4,0);
    mainLayout->addWidget(addToMemoryButton,4,0);

    for(int i = 1; i < NumDigitButtons; ++i)
    {
        int row = ((9-i)/3)+2;
        int column = ((i-1)%3) + 1;
        mainLayout->addWidget(digitButtons[i],row,column);
    }

    mainLayout->addWidget(digitButtons[0],5,1);
    mainLayout->addWidget(pointButton, 5, 2);
    mainLayout->addWidget(changeSignButton, 5, 3);

    mainLayout->addWidget(divisionButton, 2, 4);
    mainLayout->addWidget(timesButton, 3, 4);
    mainLayout->addWidget(minusButton, 4, 4);
    mainLayout->addWidget(plusButton, 5, 4);

    mainLayout->addWidget(squareRootButton, 2, 5);
    mainLayout->addWidget(powerButton, 3,5);
    mainLayout->addWidget(reciprocalButton, 4, 5);
    mainLayout->addWidget(equalButton, 5, 5);


    setLayout(mainLayout);

    setWindowTitle("QCalc");


}

Button *Calculator::createButton(const QString &text, const char *member)
{
    Button *button = new Button(text);
    connect(button, SIGNAL(clicked()), this, member);
    return button;
}


void Calculator::digitClicked()
{
    Button *clickedButton = qobject_cast<Button *>(sender());
    int digitValue = clickedButton->text().toInt();
    if (display->text() == "0" && digitValue == 0.0)
    {
        return;
    }

    if(waitingForOperand)
    {
        display->clear();
        waitingForOperand = false;
    }

    display->setText(display->text() + QString::number(digitValue));
}

void Calculator::addToMemory()
{
    equalClicked();
    sumInMemory += display->text().toDouble();
}

void Calculator::setMemory()
{
    equalClicked();
    sumInMemory = display->text().toDouble();
}

void Calculator::clearMemory()
{
    sumInMemory = 0.0;
}

void Calculator::readMemory()
{
    display->setText(QString::number(sumInMemory));
    waitingForOperand = true;
}

void Calculator::clear()
{
    if(waitingForOperand)
    {
        return;
    }
    display->setText("0");
    waitingForOperand = true;
}

void Calculator::clearAll()
{
    display->setText("0");
    factorSoFar = 0.0;
    sumSoFar = 0.0;
    pendingAdditiveOperator.clear();
    pendingMultiplicativeOperator.clear();
    waitingForOperand = true;
}

void Calculator::unaryOperatorClicked()
{
    Button *clickedButton = qobject_cast<Button *>(sender());
    QString clickedOperator = clickedButton->text();

    double operand = display->text().toDouble();
    double result = 0.0;

    if(clickedOperator == "sqrt")
    {
        if(operand < 0.0)
        {
            abortOperation();
            return;
        }
        result = sqrt(operand);
    }
    else if(clickedOperator == "x\262")
    {
        result = pow(operand, 2);
    }
    else if(clickedOperator == "1/x")
    {
        if(operand <= 0.0)
        {
            abortOperation();
            return;
        }
        result = 1.0 / operand;
    }
    display->setText(QString::number(result));
    waitingForOperand = true;
}

void Calculator::additiveOperatorClicked()
{
    Button *clickedButton = qobject_cast<Button *>(sender());
    QString clickedOperator = clickedButton->text();

    double operand = display->text().toDouble();
    if(!pendingMultiplicativeOperator.isEmpty())
    {
        if(!calculate(operand, pendingMultiplicativeOperator))
        {
            abortOperation();
            return;
        }
        display->setText(QString::number(factorSoFar));
        operand = factorSoFar;
        factorSoFar = 0.0;

        pendingMultiplicativeOperator.clear();
    }

    if(!pendingAdditiveOperator.isEmpty())
    {
        if(!calculate(operand, pendingAdditiveOperator))
        {
            abortOperation();
            return;
        }
        display->setText(QString::number(sumSoFar));
    }
    else
    {
        sumSoFar = operand;
    }

    pendingAdditiveOperator = clickedOperator;
    waitingForOperand = true;
}

void Calculator::multiplicativeOperatorClicked()
{
    Button *clickedButton = qobject_cast<Button *>(sender());
    QString clickedOperator = clickedButton->text();

    double operand = display->text().toDouble();

    if(!pendingMultiplicativeOperator.isEmpty())
    {
        if(!calculate(operand, pendingMultiplicativeOperator))
        {
            abortOperation();
            return;
        }
        display->setText(QString::number(factorSoFar));
    }
    else
    {
        factorSoFar = operand;
    }

    pendingMultiplicativeOperator = clickedOperator;
    waitingForOperand = true;
}

void Calculator::equalClicked()
{
    double operand = display->text().toDouble();

    if(!pendingMultiplicativeOperator.isEmpty())
    {
        if(!calculate(operand, pendingMultiplicativeOperator))
        {
            abortOperation();
            return;
        }
        operand = factorSoFar;
        factorSoFar = 0.0;
        pendingMultiplicativeOperator.clear();
    }

    if(!pendingAdditiveOperator.isEmpty())
    {
        if(!calculate(operand, pendingAdditiveOperator))
        {
            abortOperation();
            return;
        }
        pendingAdditiveOperator.clear();
    }
    else
    {
        sumSoFar = operand;
    }


    display->setText(QString::number(sumSoFar));
    sumSoFar = 0.0;
    waitingForOperand = true;
}

void Calculator::pointClicked()
{
    if(waitingForOperand)
    {
        display->setText("0");
    }
    if(!display->text().contains("."))
    {
        display->setText(display->text() + ".0");
    }
    waitingForOperand = false;
}

void Calculator::changeSignClicked()
{
    QString text = display->text();
    double value = text.toDouble();
    if(value > 0.0)
    {
        text.prepend("-");
    }
    else
    {
        text.remove(0,1);
    }
    display->setText(text);
}
