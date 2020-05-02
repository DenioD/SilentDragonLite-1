#ifndef CHATMODEL_H
#define CHATMODEL_H
#include <QString>
#include <QStandardItemModel>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <map>
#include <vector>
#include <QListView>
#include "precompiled.h"
#include "mainwindow.h"
#include "controller.h"
#include "settings.h"
#include "camount.h"

class ListViewDelegate : public QAbstractItemDelegate
{
    int d_radius;
    int d_toppadding;
    int d_bottompadding;
    int d_leftpadding;
    int d_rightpadding;
    int d_verticalmargin;
    int d_horizontalmargin;
    int d_pointerwidth;
    int d_pointerheight;
    float d_widthfraction;
    public:
        inline ListViewDelegate(QObject *parent = nullptr);

    protected:
        inline void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
        inline QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;
};

inline ListViewDelegate::ListViewDelegate(QObject *parent): QAbstractItemDelegate(parent), d_radius(5), d_toppadding(5), d_bottompadding(3), d_leftpadding(5), d_rightpadding(5), d_verticalmargin(15), d_horizontalmargin(10), d_pointerwidth(10), d_pointerheight(17), d_widthfraction(.7)
{

}

inline void ListViewDelegate::paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    QTextDocument bodydoc;
    QTextOption textOption(bodydoc.defaultTextOption());
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    bodydoc.setDefaultTextOption(textOption);
    bodydoc.setDefaultFont(QFont("Roboto", 12));
    QString bodytext(index.data(Qt::DisplayRole).toString());
    bodydoc.setHtml(bodytext);
    qreal contentswidth = option.rect.width() * d_widthfraction - d_horizontalmargin - d_pointerwidth - d_leftpadding - d_rightpadding;
    bodydoc.setTextWidth(contentswidth);
    qreal bodyheight = bodydoc.size().height();
    bool outgoing = index.data(Qt::UserRole + 1).toString() == "Outgoing";
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // uncomment to see the area provided to paint this item
    
    //painter->drawRect(option.rect);

    painter->translate(option.rect.left() + d_horizontalmargin, option.rect.top() + ((index.row() == 0) ? d_verticalmargin : 0));

    // background color for chat bubble
    QColor bgcolor("fbffff");
    if (outgoing)
        bgcolor = "#fffcfb";

    // create chat bubble
    QPainterPath pointie;

    // left bottom
    pointie.moveTo(0, bodyheight + d_toppadding + d_bottompadding);

    // right bottom
    pointie.lineTo(0 + contentswidth + d_pointerwidth + d_leftpadding + d_rightpadding - d_radius,
                 bodyheight + d_toppadding + d_bottompadding);
    pointie.arcTo(0 + contentswidth + d_pointerwidth + d_leftpadding + d_rightpadding - 2 * d_radius,
                bodyheight + d_toppadding + d_bottompadding - 2 * d_radius,
                2 * d_radius, 2 * d_radius, 270, 90);

    // right top
    pointie.lineTo(0 + contentswidth + d_pointerwidth + d_leftpadding + d_rightpadding, 0 + d_radius);
    pointie.arcTo(0 + contentswidth + d_pointerwidth + d_leftpadding + d_rightpadding - 2 * d_radius, 0,
                2 * d_radius, 2 * d_radius, 0, 90);

    // left top
    pointie.lineTo(0 + d_pointerwidth + d_radius, 0);
    pointie.arcTo(0 + d_pointerwidth, 0, 2 * d_radius, 2 * d_radius, 90, 90);

    // left bottom almost (here is the pointie)
    pointie.lineTo(0 + d_pointerwidth, bodyheight + d_toppadding + d_bottompadding - d_pointerheight);
    pointie.closeSubpath();

    // rotate bubble for outgoing messages
    if (outgoing)
    {
        painter->translate(option.rect.width() - pointie.boundingRect().width() - d_horizontalmargin - d_pointerwidth, 0);
        painter->translate(pointie.boundingRect().center());
        painter->rotate(180);
        painter->translate(-pointie.boundingRect().center());
    }

    // now paint it!
    painter->setPen(QPen(bgcolor));
    painter->drawPath(pointie);
    painter->fillPath(pointie, QBrush(bgcolor));

    // rotate back or painter is going to paint the text rotated...
    if (outgoing)
    {
        painter->translate(pointie.boundingRect().center());
        painter->rotate(-180);
        painter->translate(-pointie.boundingRect().center());
    }

    // set text color used to draw message body
    QAbstractTextDocumentLayout::PaintContext ctx;
    if (outgoing)
        ctx.palette.setColor(QPalette::Text, QColor("black"));
    else
        ctx.palette.setColor(QPalette::Text, QColor("white"));

    // draw body text
    painter->translate((outgoing ? 0 : d_pointerwidth) + d_leftpadding, 0);
    bodydoc.documentLayout()->draw(painter, ctx);

    painter->restore();
}

inline QSize ListViewDelegate::sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    QTextDocument bodydoc;
    QTextOption textOption(bodydoc.defaultTextOption());
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    bodydoc.setDefaultTextOption(textOption);
    bodydoc.setDefaultFont(QFont("Roboto", 12));
    QString bodytext(index.data(Qt::DisplayRole).toString());
    bodydoc.setHtml(bodytext);

    // the width of the contents are the (a fraction of the window width) minus (margins + padding + width of the bubble's tail)
    qreal contentswidth = option.rect.width() * d_widthfraction - d_horizontalmargin - d_pointerwidth - d_leftpadding - d_rightpadding;

    // set this available width on the text document
    bodydoc.setTextWidth(contentswidth);

    QSize size(bodydoc.idealWidth() + d_horizontalmargin + d_pointerwidth + d_leftpadding + d_rightpadding,
             bodydoc.size().height() + d_bottompadding + d_toppadding + d_verticalmargin + 1); // I dont remember why +1, haha, might not be necessary

    if (index.row() == 0) // have extra margin at top of first item
        size += QSize(0, d_verticalmargin);

    return size;
}





class ChatItem
{
    private:
        long _timestamp;
        QString _address;
        QString _contact;
        QString _memo; 
        QString _cid;
        QString _txid;
        bool _outgoing = false;

    public:
        ChatItem() {}

        ChatItem(long timestamp, QString address, QString contact, QString memo, QString cid, QString txid)
        {
            _timestamp = timestamp;
            _address = address;
            _contact = contact;
            _memo = memo;
            _cid = cid;
            _txid = txid;
            _outgoing = false;

        }

        ChatItem(long timestamp, QString address, QString contact, QString memo, QString cid, QString txid, bool outgoing)
        {
            _timestamp = timestamp;
            _address = address;
            _contact = contact;
            _memo = memo;
            _cid = cid;
            _txid = txid;
            _outgoing = outgoing;

        }

        long getTimestamp()
        {
            return _timestamp;
        }

        QString getAddress()
        {
            return _address;
        }

        QString getContact()
        {
            return _contact;
        }

        QString getMemo()
        {
            return _memo;
        }

        QString getCid()
        {
            return _cid;
        }

        QString getTxid()
        {
            return _txid;
        }

        bool isOutgoing()
        {
            return _outgoing;
        }

        void setTimestamp(long timestamp)
        {
            _timestamp = timestamp;
        }

        void setAddress(QString address)
        {
            _address = address;
        }

        void setContact(QString contact)
        {
            _contact = contact;
        }

        void setMemo(QString memo)
        {
            _memo = memo;
        }

        void setCid(QString cid)
        {
            _cid = cid;
        }
         void setTxid(QString txid)
        {
            _txid = txid;
        }

        void toggleOutgo()
        {
            _outgoing = true;
        }

        QString toChatLine()
        {
            QDateTime myDateTime;  
            myDateTime.setTime_t(_timestamp);
            QString line = QString("[") + myDateTime.toString("dd.MM.yyyy hh:mm:ss ") +  QString("] ");
            line += QString(_memo) + QString("\n");      
            return line;
        }

        ~ChatItem()
        {
            /*delete timestamp;
            delete address;
            delete contact;
            delete memo;
            delete outgoing;*/
        }
};

class ChatModel
{
    private:
        std::map<QString, ChatItem> chatItems; 
        QTableView* parent;
        Ui::MainWindow*             ui;
        MainWindow*                 main;
        std::map<QString, QString> cidMap;

    public:
        ChatModel() {};
        ChatModel(std::map<QString, ChatItem> chatItems);
        ChatModel(std::vector<ChatItem> chatItems);
        QString generateChatItemID(ChatItem item);
        std::map<QString, ChatItem> getItems();
        void setItems(std::map<QString, ChatItem> items);
        QString zaddr();
        void setItems(std::vector<ChatItem> items);
        void renderChatBox(Ui::MainWindow* ui, QListView &view);
        void renderChatBox(Ui::MainWindow* ui, QListView *view);
        void showMessages();
        void clear();
        void addMessage(ChatItem item);
        void addMessage(QString timestamp, ChatItem item);
        void addCid(QString tx, QString cid);
        QString getCidByTx(QString tx);
        void killCidCache();
};

#endif