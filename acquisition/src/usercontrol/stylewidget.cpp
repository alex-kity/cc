﻿#include "stylewidget.h"
#include "qfontdatabase.h"
#include "qpainter.h"
#include "qdatetime.h"
#include "qdebug.h"

StyleWidget::StyleWidget(QWidget *parent) : QWidget(parent)
{
    bgColor = QColor("#26282C");
    signColor = QColor("#CD2929");
    signSize = 30;
    checked = false;
    hovered = false;

    //引入图形字体
    int fontId = QFontDatabase::addApplicationFont(":/font/fontawesome-webfont.ttf");
    QStringList fontName = QFontDatabase::applicationFontFamilies(fontId);

    if (fontName.count() > 0) {
        iconFont = QFont(fontName.at(0));
    } else {
        qDebug() << "load fontawesome-webfont.ttf error";
    }

    hover = false;
}

void StyleWidget::enterEvent(QEvent *)
{
    hover = true;
    update();
}

void StyleWidget::leaveEvent(QEvent *)
{
    hover = false;
    update();
}

void StyleWidget::mousePressEvent(QMouseEvent *)
{
    setChecked(!checked);
}

void StyleWidget::paintEvent(QPaintEvent *)
{
    //绘制准备工作,启用反锯齿
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    //绘制背景
    drawBg(&painter);
    //绘制选中标记符号
    drawSign(&painter);
}

void StyleWidget::drawBg(QPainter *painter)
{
    painter->save();

    //如果悬停则绘制外边框和内边框
    if (hover || hovered) {
        painter->setBrush(Qt::NoBrush);

        QPen pen = painter->pen();
        pen.setWidth(2);

        //绘制外边框
        pen.setColor(bgColor);
        painter->setPen(pen);
        painter->drawRect(5, 5, width() - 10, height() - 10);

        //绘制里边框
        pen.setColor("#FFFFFF");
        painter->setPen(pen);
        painter->drawRect(7, 7, width() - 14, height() - 14);
    }

    //绘制里边背景
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(bgColor));
    painter->drawRect(8, 8, width() - 16, height() - 16);

    painter->restore();
}

void StyleWidget::drawSign(QPainter *painter)
{
    painter->save();

    //如果当前选中则绘制角标
    if (checked) {
        //计算角标区域
        QRect rec(width() - signSize, height() - signSize, signSize, signSize);

        //绘制带边框背景
        QPen pen = painter->pen();
        pen.setWidth(3);
        pen.setColor("#FFFFFF");

        painter->setPen(pen);
        painter->setBrush(QColor(signColor));
        painter->drawEllipse(rec.x() + 3, rec.y() + 3, signSize - 6, signSize - 6);

        //绘制文字
        iconFont.setPointSize(12);
        painter->setFont(iconFont);
        painter->drawText(rec, Qt::AlignCenter, QChar(0xf00c));
    }

    painter->restore();
}

QColor StyleWidget::getBgColor() const
{
    return this->bgColor;
}

QColor StyleWidget::getSignColor() const
{
    return this->signColor;
}

int StyleWidget::getSignSize() const
{
    return this->signSize;
}

bool StyleWidget::getChecked() const
{
    return this->checked;
}

bool StyleWidget::getHovered() const
{
    return this->hovered;
}

QSize StyleWidget::sizeHint() const
{
    return QSize(120, 120);
}

QSize StyleWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

void StyleWidget::setBgColor(const QColor &bgColor)
{
    if (this->bgColor != bgColor) {
        this->bgColor = bgColor;
        update();
    }
}

void StyleWidget::setSignColor(const QColor &signColor)
{
    if (this->signColor != signColor) {
        this->signColor = signColor;
        update();
    }
}

void StyleWidget::setSignSize(int signSize)
{
    if (this->signSize != signSize) {
        this->signSize = signSize;
        update();
    }
}

void StyleWidget::setChecked(bool checked)
{
    if (this->checked != checked) {
        this->checked = checked;
        update();

        if (checked) {
            emit selected(bgColor);
        }
    }
}

void StyleWidget::setHovered(bool hovered)
{
    if (this->hovered != hovered) {
        this->hovered = hovered;
        update();
    }
}
