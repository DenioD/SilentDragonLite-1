// Copyright 2019-2020 The Hush developers
// GPLv3

#include "Chat.h"
#include "../addressbook.h"
#include "../DataStore/DataStore.h"
Chat::Chat() {}

ChatMemoEdit::ChatMemoEdit(QWidget* parent) : QTextEdit(parent) {
    QObject::connect(this, &QTextEdit::textChanged, this, &ChatMemoEdit::updateDisplayChat);
}

void ChatMemoEdit::updateDisplayChat() {
    QString txt = this->toPlainText();
    if (lenDisplayLabelchat)
        lenDisplayLabelchat->setText(QString::number(txt.toUtf8().size()) + "/" + QString::number(maxlenchat));

    if (txt.toUtf8().size() <= maxlenchat) {
        // Everything is fine
        if (sendChatButton)
            sendChatButton->setEnabled(true);

        if (lenDisplayLabelchat)
            lenDisplayLabelchat->setStyleSheet("");
    }
    else {
        // Overweight
        if (sendChatButton)
            sendChatButton->setEnabled(false);

        if (lenDisplayLabelchat)
            lenDisplayLabelchat->setStyleSheet("color: red;");
    }
}

void ChatMemoEdit::setMaxLenChat(int len) {
    this->maxlenchat = len;
    updateDisplayChat();
}

void ChatMemoEdit::SetSendChatButton(QPushButton* button) {
    this->sendChatButton = button;
}

void ChatMemoEdit::setLenDisplayLabelChat(QLabel* label) {
    this->lenDisplayLabelchat = label;
}

ChatMemoEditRequest::ChatMemoEditRequest(QWidget* parent) : QTextEdit(parent) {
    QObject::connect(this, &QTextEdit::textChanged, this, &ChatMemoEditRequest::updateDisplayChatRequest);
}

void ChatMemoEditRequest::updateDisplayChatRequest() {
    QString txt = this->toPlainText();
    if (lenDisplayLabelchatRequest)
        lenDisplayLabelchatRequest->setText(QString::number(txt.toUtf8().size()) + "/" + QString::number(maxlenchatrequest));

    if (txt.toUtf8().size() <= maxlenchatrequest) {
        // Everything is fine
        if (sendRequestButton)
            sendRequestButton->setEnabled(true);

        if (lenDisplayLabelchatRequest)
            lenDisplayLabelchatRequest->setStyleSheet("");
    }
    else {
        // Overweight
        if (sendRequestButton)
            sendRequestButton->setEnabled(false);

        if (lenDisplayLabelchatRequest)
            lenDisplayLabelchatRequest->setStyleSheet("color: red;");
    }
}

void ChatMemoEditRequest::setMaxLenChatRequest(int len) {
    this->maxlenchatrequest = len;
    updateDisplayChatRequest();
}

void ChatMemoEditRequest::SetSendRequestButton(QPushButton* button) {
    this->sendRequestButton = button;
}

void ChatMemoEditRequest::setLenDisplayLabelChatRequest(QLabel* label) {
    this->lenDisplayLabelchatRequest = label;
}
void Chat::renderChatBox(Ui::MainWindow *ui, QListView *view, QLabel *label)
{
    
    QStandardItemModel *chat = new QStandardItemModel();
    DataStore::getChatDataStore()->dump(); // test to see if the chat items in datastore are correctly dumped to json
    for (auto &p : AddressBook::getInstance()->getAllAddressLabels())
    {
        for (auto &c : DataStore::getChatDataStore()->getAllMemos())
        {

            if (
                (p.getName() == ui->contactNameMemo->text().trimmed()) &&
                (p.getPartnerAddress() == c.second.getAddress()) &&
                (c.second.isOutgoing() == true))
               
            {

                QStandardItem *Items = new QStandardItem(c.second.toChatLine());

                Items->setData(OUTGOING, Qt::UserRole + 1);
                chat->appendRow(Items);
                ui->listChat->setModel(chat);
            
       
            }
            else
            {
                ui->listChat->setModel(chat);
            }

            if (
                (p.getName() == ui->contactNameMemo->text().trimmed()) &&
                (p.getMyAddress() == c.second.getAddress()) &&
                (c.second.isOutgoing() == false) &&
                (c.second.getCid() == p.getCid())
                )
            {
                QStandardItem *Items1 = new QStandardItem(c.second.toChatLine());
                Items1->setData(INCOMING, Qt::UserRole + 1);
                chat->appendRow(Items1);
                ui->listChat->setModel(chat);
                
               
            }
            else
            {

                ui->listChat->setModel(chat);
            }
        }
    } 
}  

  