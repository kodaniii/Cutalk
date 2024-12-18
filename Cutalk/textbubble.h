#ifndef TEXTBUBBLE_H
#define TEXTBUBBLE_H

#include <QTextEdit>
#include "BubbleFrame.h"
#include <QHBoxLayout>

class TextBubble : public BubbleFrame
{
    Q_OBJECT

public:
    TextBubble(ChatRole role, const QString &text, QWidget *parent = nullptr);

protected:
    //对于文本类型，设置气泡宽度（以后不再更改宽度）
    bool eventFilter(QObject *o, QEvent *e);
private:
    //调整气泡的高度以适应文本内容
    void adjustTextHeight();
    //设置文本内容
    void setPlainText(const QString &text);
    void initStyleSheet();
private:
    QTextEdit *m_pTextEdit;
};

#endif // TEXTBUBBLE_H
