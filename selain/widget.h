#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QWidget>

#include <QGraphicsView>
#include <QGraphicsWebView>
#include <QMap>
#include <QHash>
#include <QDebug>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>

#include "webitem.h"


 inline uint qHash(const QRect &k) {
     return qHash(k.x() ^ k.y()) ^ qHash(k.width() ^ k.height());
 }

 inline uint qHash(const QUrl &k) {
     return qHash(k.toString());
 }

class Widget : public QGraphicsView
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void loadFinished(bool ok);


    virtual void resizeEvent(QResizeEvent *event);

    void createPixmapBuoy();
    void updatePixmapBuoyCoordinates();

private slots:
    void newMain();
    inline void changed() { qDebug() << "chagned"; }
    inline void changedGeometry(QRect r) { qDebug() << "geomet" << r; }
    void createBuoysAfterNewMain();

private:
    WebItem *addBuoy(QWebElement elem);


    QHash<QUrl, WebItem*> url2webs;
    QGraphicsView *view;
    WebItem *web;
    QGraphicsItem *circle;
    QGraphicsPixmapItem *bgImg;

    QHash<QRect, WebItem*> buoys;
};

#endif // WIDGET_H
