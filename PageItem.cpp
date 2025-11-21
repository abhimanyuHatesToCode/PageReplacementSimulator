#include "PageItem.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QPropertyAnimation>

PageItem::PageItem(int pageNumber, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), m_page(pageNumber)
{
    setRect(0, 0, 60, 36);
    setBrush(QColor(100, 180, 255, 230));
    setPen(QPen(Qt::black, 1));

    m_text = new QGraphicsTextItem(QString::number(pageNumber), this);
    QFont f;
    f.setPointSize(12);
    f.setBold(true);
    m_text->setFont(f);
    m_text->setDefaultTextColor(Qt::white);

    qreal tw = m_text->boundingRect().width();
    qreal th = m_text->boundingRect().height();
    m_text->setPos((rect().width()-tw)/2, (rect().height()-th)/2 - 2);

    m_effect = new QGraphicsColorizeEffect(this);
    m_effect->setStrength(0.0);
    setGraphicsEffect(m_effect);
}

void PageItem::setPageNumber(int page) {
    m_page = page;
    m_text->setPlainText(QString::number(page));
    qreal tw = m_text->boundingRect().width();
    qreal th = m_text->boundingRect().height();
    m_text->setPos((rect().width()-tw)/2, (rect().height()-th)/2 - 2);
}

void PageItem::flashColor(const QColor &c) {
    if (!m_effect) return;
    m_effect->setColor(c);

    QPropertyAnimation *anim = new QPropertyAnimation(m_effect, "strength");
    anim->setDuration(450);
    anim->setStartValue(0.0);
    anim->setKeyValueAt(0.5, 0.85);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}
