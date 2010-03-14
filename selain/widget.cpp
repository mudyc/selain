#include "widget.h"

#include <QGraphicsView>
#include <QGraphicsWebView>
#include <QWebView>
#include <QWebFrame>
#include <QWebPage>
#include <QWebElementCollection>
#include <QWebElement>
#include <QDebug>
#include <QRect>
#include <QPen>
#include <QFileInfo>
#include <QGraphicsEffect>
#include <QColor>
#include <QVector2D>

#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QTimer>

#include <math.h>

Widget::Widget(QWidget *parent)
    : QGraphicsView(parent)
{

    circle = NULL;
    view = this;
    view->setScene(new QGraphicsScene());


    web = new WebItem();
    connect(web, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
    connect(web->page(), SIGNAL(contentsChanged()), this, SLOT(changed()));
    connect(web->page(), SIGNAL(geometryChangeRequested(QRect)), this, SLOT(changedGeometry(QRect)));

    QFileInfo f("../en.htm");
    qDebug() << f.absoluteFilePath() << f.exists();
    //web->load(QUrl(f.absoluteFilePath()));
    //web->load(QUrl("http://ixonos.com"));
    web->load(QUrl("http://google.com"));
    web->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    web->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    web->setPos(100,100);
    //web->setMaximumSize(300,300);
    //web->show();

    view->scene()->addItem(web);
    //web->setPos(20,20);
    view->setMinimumSize(800,600);
    bgImg = new QGraphicsPixmapItem(QPixmap("../sky.jpg"));
    //bgImg = new QGraphicsPixmapItem(QPixmap("../water.jpg"));
    bgImg->setPos(0,0);
    bgImg->setZValue(-1000);
    scene()->addItem(bgImg);
    //setFrameRect(QRect(0,0,800,600));
    view->show();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(web, SIGNAL(xChanged()), this, SLOT(updatePixmapBuoyCoordinates()));
    connect(web, SIGNAL(yChanged()), this, SLOT(updatePixmapBuoyCoordinates()));

    //web->loader.setUrl(QUrl("http://google.com"));
    //web->loader.moveToThread(&web->loader);
    //web->loader.proc.moveToThread(&web->loader);
    //web->loader.start();
    //QTimer::singleShot(2000, &web->loader, SLOT(start()));
    /*
    QTimer t;
    t.setInterval(1000);
    connect(&t, SIGNAL(timeout()), &(web->loader), SLOT(dbg()), Qt::QueuedConnection);
    t.start();
    */
}

Widget::~Widget()
{

}

void Widget::resizeEvent(QResizeEvent *event)
{
    if (circle)
        scene()->removeItem(circle);
    circle = scene()->addEllipse(0,0,geometry().width(),geometry().height());
    circle->setZValue(-100);
}


void Widget::loadFinished(bool ok)
{
    QSize r = web->page()->mainFrame()->contentsSize();
    web->setGeometry(QRectF(web->x(),web->y(),r.width(), r.height()));

    QGraphicsItem *rect = scene()->addRect(0, 0, r.width(), r.height(), QPen(), QBrush(QColor(255,255,255)));
    rect->setParentItem(web);
    rect->setFlag(QGraphicsItem::ItemStacksBehindParent);

    foreach(QWebElement elem, web->page()->currentFrame()->findAllElements("a")){
        WebItem *buoy = addBuoy(elem);
        buoys.insert(elem.geometry(), buoy);
    }

    updatePixmapBuoyCoordinates();
}

void Widget::createPixmapBuoy()
{
    qDebug() << Q_FUNC_INFO;
    WebItem *buoy = static_cast<WebItem *>(sender());

    scene()->removeItem(buoy->loadingEllipse());
    scene()->addItem(buoy->pixmapItem());

    QGraphicsRectItem *rect = buoy->bgRect();
    rect->setBrush(QBrush(QColor(255,255,255)));
    rect->setParentItem(buoy->pixmapItem());
    rect->setRect(-1,-1,71,101);
    rect->setFlag(QGraphicsItem::ItemStacksBehindParent);
    scene()->addItem(rect);

    //QSize s = buoy->page()->mainFrame()->contentsSize();
    //buoy->setGeometry(QRectF(2000,2000, s.width(), s.height()));
    //buoy->setGeometry(QRectF(0,0,100,100));
    //buoy->setZValue(100);
    //scene()->addItem(buoy);
    //rect->setParentItem(buoy);
    //buoy->update();

    updatePixmapBuoyCoordinates();
}


WebItem *Widget::addBuoy(QWebElement elem)
{
    QPen pen;
    pen.setStyle(Qt::DashDotLine);
    pen.setWidth(3);
    pen.setBrush(Qt::green);

    QString url = elem.attribute("href");
    qDebug() << "href: " <<url;
    if (!(url.startsWith("http://") || url.startsWith("https://")))
        url = web->url().toString(QUrl::RemovePath|QUrl::RemoveQuery) + url;
    QUrl URL(url);

    qDebug() << "href2: "<< url;

    WebItem *buoy = url2webs.value(URL, NULL);
    qDebug() << "add buoy? " << buoy;
    if (buoy == NULL || !buoy->isLoaded()) {
        if (!buoy)
            buoy = new WebItem();

        buoy->loader.setUrl(URL);
        buoy->loader.moveToThread(&buoy->loader);
        buoy->loader.start();

        scene()->addItem(buoy->loadingEllipse());
        buoy->loadingEllipse()->setSpanAngle(250);
        buoy->progres.start(1000/10);
    } else
        emit buoy->pixmapReady();

    qDebug() << "add buoy? " << buoy;
    buoy->setBuoy(true);
    connect(buoy, SIGNAL(buoyClicked()), this, SLOT(newMain()));
    connect(buoy, SIGNAL(pixmapReady()), this, SLOT(createPixmapBuoy()));
    //buoy->load(QUrl(url));
    //  buoy->load(this->web->url());

    url2webs.insert(URL, buoy);

    scene()->addItem(buoy->line());
    // too slow!
    //buoy->setGraphicsEffect(new QGraphicsDropShadowEffect());

    return buoy;
}

qreal pow2(qreal a) { return a*a; }

void Widget::updatePixmapBuoyCoordinates()
{
    //qDebug() << Q_FUNC_INFO;

    // sort buoys by the range from centre
    QPointF center2 = QPointF(width(), height())*0.5;
    QVector2D center = QVector2D(web->mapFromScene(center2));

    foreach (QRect r, buoys.keys()) {
        QVector2D anchor = QVector2D(r.center());
        qreal d = (center - anchor).length();

        WebItem *buoy = buoys.value(r);

        if (buoy == NULL) continue;

        if (buoy->isLoaded())
            buoy->pixmapItem()->setZValue(10.0f+(1.0f/d));
        else
            buoy->loadingEllipse()->setZValue(10.0f+(1.0f/d));
        buoy->line()->setZValue(10.0f+(1.0001f/d));

        // check if anchor is inside the circle
        QPointF anchorP = web->mapToScene(r.center());
        QPointF anchorP2 = anchorP - center2;
        if (pow2(anchorP2.x()/center2.x()) + pow2(anchorP2.y()/center2.y()) <= 1) {

            QGraphicsRectItem *rect = buoy->bgRect();
            rect->setBrush(QBrush(QColor(0,255,255)));

            // project to buoy circle
            qreal r = center2.x(),
                  s = center2.y();
            //line.setLine(center2.x(), center2.y(), anchorP.x(), anchorP.y());
            QPointF p = center2 - anchorP;
            qreal a = -p.y();
            qreal b = p.x();
            qreal c = a/anchorP.x() + b/anchorP.y();

            qreal q = pow2(a)*pow2(r) + pow2(b)*pow2(s);
            qreal det = sqrt(-pow2(c) + q);
            if (!isnan(det) && det != 0) {
                /*
                qreal x0 = r*(a*c*r + b*s*det)/q;
                qreal y0 = s*(b*c*s - a*r*det)/q;
                */
                qreal x1 = r*(a*c*r - b*s*det)/q;
                qreal y1 = s*(b*c*s + a*r*det)/q;

                QPointF xy(x1,y1);
                xy += center2;
                QPen pen(QColor(143,120,120, 120));
                pen.setWidth(3);
                buoy->line()->setPen(pen);
                buoy->line()->setLine(xy.x()+5,xy.y()+5,anchorP.x(),anchorP.y());
                if (buoy->isLoaded()) {
                    xy -= QPointF(buoy->pixmapItem()->pixmap().width(), buoy->pixmapItem()->pixmap().height())*0.5;
                    buoy->pixmapItem()->setPos(xy);
                } else
                    buoy->loadingEllipse()->setRect(xy.x()-35, xy.y()-35, 70,70);

            } else qDebug() << "asdf";
        } else {
            //qDebug() << "out" << anchorP << center2;
            //buoy->setPos(web->mapToScene(r.center()) - QPointF(buoy->geometry().width(),buoy->geometry().height())*0.5);
            //QGraphicsRectItem *r = (QGraphicsRectItem *)buoy->childItems().at(0);
            //r->setBrush(QBrush(QColor(255,255,255)));
            buoy->loadingEllipse()->setRect(-100,-100, 1,1);
            buoy->pixmapItem()->setPos(-100,-100);
            buoy->line()->setLine(-1,-1,-1,-1);
        }
    }
}

void Widget::newMain()
{
    qDebug() << Q_FUNC_INFO;

    foreach(WebItem *i, buoys.values()){
        i->stop();
        i->setBuoy(true);
    }
    foreach (QGraphicsItem *i, scene()->items()) {
        scene()->removeItem(i);
    }
    scene()->addItem(bgImg);
    scene()->addItem(circle);


    WebItem *main = static_cast<WebItem *>(sender());
    QPointF orig = main->pixmapItem()->pos();
    QSizeF fullSize = main->page()->mainFrame()->contentsSize();

    // create new main
    web = main;//new WebItem(); //main
    if (main->loader.url() != web->url()) {
        fullSize = web->fullSize();
        web->load(main->loader.url());
    }
    web->setBuoy(false);
    web->setZValue(0);
    web->setZoomFactor(0.15);
    //web->setMinimumSize(-1,-1);
    //QSize r = web->page()->mainFrame()->contentsSize();
    //web->setGeometry(QRectF(web->x(), web->y(), r.width(), r.height()));
    scene()->addItem(web);
    //scene()->addItem(web->pixmapItem());
    //web->pixmapItem()->setParentItem(web);


    // create animation
    int duration = 5*1000; //ms
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    QPropertyAnimation *zoom = new QPropertyAnimation(web, "zoomFactor", group);
    zoom->setEndValue(1.0);
    zoom->setDuration(duration);
    QSize r = web->page()->mainFrame()->contentsSize()/web->zoomFactor();

    web->setGeometry(QRectF(orig.x(), orig.y(), 70, 100));
    QPropertyAnimation *x = new QPropertyAnimation(web, "geometry", group);
    x->setEndValue(QRect(100,100, fullSize.width(), fullSize.height()));
    x->setDuration(duration);
    QPropertyAnimation *z = new QPropertyAnimation(web, "z", group);
    z->setEndValue(0);
    z->setDuration(duration);

    /*
    QPropertyAnimation *rotate = new QPropertyAnimation(web, "rotation", group);
    rotate->setDuration(duration);
    rotate->setEndValue(360);
    */
/*
    QSequentialAnimationGroup *rotate = new QSequentialAnimationGroup(group);
    qreal durations[]={ 1./6., 1./3., 1./3., 1./6.};
    int rots[] = { -20,20,-20,0 };
    for (int i=0; i<sizeof(durations)/sizeof(durations[0]); i++) {
        QPropertyAnimation *r1 = new QPropertyAnimation(web, "rotation", group);
        if (i==0 || i== 3)
            r1->setEasingCurve(QEasingCurve::InCirc);
        else
            r1->setEasingCurve(QEasingCurve::InOutExpo);
        r1->setDuration(duration*durations[i]);
        r1->setEndValue(rots[i]);
        rotate->addAnimation(r1);
    }
*/
    group->start();

    //web->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    //web->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);



    QTimer::singleShot(duration*1.2, this, SLOT(createBuoysAfterNewMain()));
}

void Widget::createBuoysAfterNewMain()
{
    //web->view.scene()->removeItem(web);

    // create new buoys..
    foreach(QWebElement elem, web->page()->currentFrame()->findAllElements("a")){
        WebItem *buoy = addBuoy(elem);
        buoys.insert(elem.geometry(), buoy);
    }

    updatePixmapBuoyCoordinates();

    connect(web, SIGNAL(xChanged()), this, SLOT(updatePixmapBuoyCoordinates()));
    connect(web, SIGNAL(yChanged()), this, SLOT(updatePixmapBuoyCoordinates()));
}
