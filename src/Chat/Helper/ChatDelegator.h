// Copyright 2019-2020 The Hush developers
// GPLv3

#ifndef CHATDELEGATOR_H
#define CHATDELEGATOR_H

#include <QString>
#include <QStandardItemModel>
#include <QAbstractItemDelegate>
#include <QPainter>

enum RenderType
{
	OUTGOING=0,
	INCOMING=1,
	INDATE=2,
	OUTDATE=3
};

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

inline ListViewDelegate::ListViewDelegate(QObject *parent): QAbstractItemDelegate(parent), d_radius(15), d_toppadding(15), d_bottompadding(3), d_leftpadding(5), d_rightpadding(5), d_verticalmargin(5), d_horizontalmargin(10), d_pointerwidth(4), d_pointerheight(25), d_widthfraction(.6)
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
    bodydoc.setHtml(bodytext.replace("\n",  "<br>")); 
    bodydoc.setHtml(bodytext.replace(":a1",  "<img src=':/emoji/res/emoji/emoji1.png'>"));
    bodydoc.setHtml(bodytext.replace(":a2",  "<img src=':/emoji/res/emoji/money-mouth.png'>"));
    bodydoc.setHtml(bodytext.replace(":a3",  "<img src=':/emoji/res/emoji/laughing.png'>"));
    bodydoc.setHtml(bodytext.replace(":a4",  "<img src=':/emoji/res/emoji/sweet_smile.png'>"));
    bodydoc.setHtml(bodytext.replace(":a5",  "<img src=':/emoji/res/emoji/joy.png'>"));
    bodydoc.setHtml(bodytext.replace(":a6",  "<img src=':/emoji/res/emoji/innocent.png'>"));
    bodydoc.setHtml(bodytext.replace(":a7",  "<img src=':/emoji/res/emoji/partying_face.png'>"));
    bodydoc.setHtml(bodytext.replace(":a8",  "<img src=':/emoji/res/emoji/face-with-rolling-eyes.png'>"));
    bodydoc.setHtml(bodytext.replace(":a9",  "<img src=':/emoji/res/emoji/face-with-tongue.png'>"));
    bodydoc.setHtml(bodytext.replace(":b1",  "<img src=':/emoji/res/emoji/face_with_3hearts.png'>"));
    bodydoc.setHtml(bodytext.replace(":b2",  "<img src=':/emoji/res/emoji/heart_shaped_eyes.png'>"));
    bodydoc.setHtml(bodytext.replace(":b3",  "<img src=':/emoji/res/emoji/nauseated-face.png'>"));
    bodydoc.setHtml(bodytext.replace(":b4",  "<img src=':/emoji/res/emoji/pile-of-poo.png'>"));
    bodydoc.setHtml(bodytext.replace(":b5",  "<img src=':/emoji/res/emoji/serious-face-with-symbols-covering-mouth.png'>"));
    bodydoc.setHtml(bodytext.replace(":b6",  "<img src=':/emoji/res/emoji/smiling-face-with-sunglasses.png'>"));
    bodydoc.setHtml(bodytext.replace(":b7",  "<img src=':/emoji/res/emoji/stuck-out.png'>"));
    bodydoc.setHtml(bodytext.replace(":b8",  "<img src=':/emoji/res/emoji/hush-money-white.png'>"));
    bodydoc.setHtml(bodytext.replace(":b9",  "<img src=':/emoji/res/emoji/SD.png'>"));
    qreal contentswidth = option.rect.width() * d_widthfraction - d_horizontalmargin - d_pointerwidth - d_leftpadding - d_rightpadding;
    bodydoc.setTextWidth(contentswidth);
    qreal bodyheight = bodydoc.size().height();
    int outgoing = index.data(Qt::UserRole + 1).toInt();
    int outdate = index.data(Qt::UserRole + 1).toInt();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // uncomment to see the area provided to paint this item
    
    //painter->drawRect(option.rect);

    painter->translate(option.rect.left() + d_horizontalmargin, option.rect.top() + ((index.row() == 0) ? d_verticalmargin : 0));

    QColor bgcolor("#ffffff");
    switch(outgoing)
    {
        case INDATE:
            bgcolor = "transparent";
            break;

        case OUTDATE:
            bgcolor = "transparent";
            break;

        case OUTGOING:
            bgcolor = "#f8f9fa";
            break;

        default:
        case INCOMING:
            bgcolor = "#535353";
            break;
    }

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
    if ((outgoing == OUTGOING) || (outdate == OUTDATE))
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
    if ((outgoing == OUTGOING) || (outdate == OUTDATE))
    {
        painter->translate(pointie.boundingRect().center());
        painter->rotate(-180);
        painter->translate(-pointie.boundingRect().center());
    }

    // set text color used to draw message body
    QAbstractTextDocumentLayout::PaintContext ctx;
    switch(outgoing)
    {
        case INDATE:
            ctx.palette.setColor(QPalette::Text, QColor("Black"));
            break;

        case OUTDATE:
            ctx.palette.setColor(QPalette::Text, QColor("Black"));
            break;

        case OUTGOING:
            ctx.palette.setColor(QPalette::Text, QColor("Black"));
            break;

        default:
        case INCOMING:
            ctx.palette.setColor(QPalette::Text, QColor("whitesmoke"));
            break;
    }


    // draw body text
    painter->translate((outgoing == OUTGOING ? 0 : d_pointerwidth) + d_leftpadding, 0);
    painter->translate((outdate == OUTDATE ? 0 : d_pointerwidth) + d_leftpadding, 0);
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
    bodydoc.setHtml(bodytext.replace("\n",  "<br>"));
    bodydoc.setHtml(bodytext.replace(":a1",  "<img src=':/emoji/res/emoji/emoji1.png'>"));
    bodydoc.setHtml(bodytext.replace(":a2",  "<img src=':/emoji/res/emoji/money-mouth.png'>"));
    bodydoc.setHtml(bodytext.replace(":a3",  "<img src=':/emoji/res/emoji/laughing.png'>"));
    bodydoc.setHtml(bodytext.replace(":a4",  "<img src=':/emoji/res/emoji/sweet_smile.png'>"));
    bodydoc.setHtml(bodytext.replace(":a5",  "<img src=':/emoji/res/emoji/joy.png'>"));
    bodydoc.setHtml(bodytext.replace(":a6",  "<img src=':/emoji/res/emoji/innocent.png'>"));
    bodydoc.setHtml(bodytext.replace(":a7",  "<img src=':/emoji/res/emoji/partying_face.png'>"));
    bodydoc.setHtml(bodytext.replace(":a8",  "<img src=':/emoji/res/emoji/face-with-rolling-eyes.png'>"));
    bodydoc.setHtml(bodytext.replace(":a9",  "<img src=':/emoji/res/emoji/face-with-tongue.png'>"));
    bodydoc.setHtml(bodytext.replace(":b1",  "<img src=':/emoji/res/emoji/face_with_3hearts.png'>"));
    bodydoc.setHtml(bodytext.replace(":b2",  "<img src=':/emoji/res/emoji/heart_shaped_eyes.png'>"));
    bodydoc.setHtml(bodytext.replace(":b3",  "<img src=':/emoji/res/emoji/nauseated-face.png'>"));
    bodydoc.setHtml(bodytext.replace(":b4",  "<img src=':/emoji/res/emoji/pile-of-poo.png'>"));
    bodydoc.setHtml(bodytext.replace(":b5",  "<img src=':/emoji/res/emoji/serious-face-with-symbols-covering-mouth.png'>"));
    bodydoc.setHtml(bodytext.replace(":b6",  "<img src=':/emoji/res/emoji/smiling-face-with-sunglasses.png'>"));
    bodydoc.setHtml(bodytext.replace(":b7",  "<img src=':/emoji/res/emoji/stuck-out.png'>"));
    bodydoc.setHtml(bodytext.replace(":b8",  "<img src=':/emoji/res/emoji/hush-money-white.png'>"));
    bodydoc.setHtml(bodytext.replace(":b9",  "<img src=':/emoji/res/emoji/SD.png'>"));

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

#endif
