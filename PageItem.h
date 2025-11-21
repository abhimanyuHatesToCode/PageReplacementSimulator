#ifndef PAGEITEM_H
#define PAGEITEM_H

#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsColorizeEffect>

class PageItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(qreal x READ x WRITE setX)
    Q_PROPERTY(qreal y READ y WRITE setY)

public:
    explicit PageItem(int pageNumber, QGraphicsItem *parent = nullptr);

    void setPageNumber(int page);
    int pageNumber() const { return m_page; }

    qreal x() const { return QGraphicsRectItem::x(); }
    qreal y() const { return QGraphicsRectItem::y(); }

public slots:
    void setX(qreal value) { QGraphicsRectItem::setX(value); }
    void setY(qreal value) { QGraphicsRectItem::setY(value); }

    // highlight (green/red)
    void flashColor(const QColor &c);

private:
    int m_page;
    QGraphicsTextItem *m_text;
    QGraphicsColorizeEffect *m_effect;
};

#endif // PAGEITEM_H
