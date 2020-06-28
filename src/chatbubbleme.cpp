#include "chatbubbleme.h"
#include "ui_chatbubbleme.h"

ChatBubbleMe::ChatBubbleMe(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatBubbleMe)
{
    ui->setupUi(this);
}

ChatBubbleMe::~ChatBubbleMe()
{
    delete ui;
}
