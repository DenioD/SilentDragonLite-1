#ifndef CHATBUBBLEPARTNER_H
#define CHATBUBBLEPARTNER_H

#include <QWidget>

namespace Ui {
class ChatBubblePartner;
}

class ChatBubblePartner : public QWidget
{
    Q_OBJECT

public:
    explicit ChatBubblePartner(QWidget *parent = nullptr);
    ~ChatBubblePartner();

private:
    Ui::ChatBubblePartner *ui;
};

#endif // CHATBUBBLEPARTNER_H
