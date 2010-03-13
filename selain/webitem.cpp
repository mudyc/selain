
#include "webitem.h"

#include <QGraphicsWebView>
#include <QWebFrame>
#include <QWebPage>
#include <QDebug>
#include <QTimer>
#include <QVector2D>
#include <QStyleOptionGraphicsItem>
#include <QDataStream>

#include <cstdlib>

Loader::Loader(): QThread()
{
}

void Loader::run()
{
    QProcess proc;
    this->proc = &proc;
    proc.setReadChannel(QProcess::StandardOutput);
    connect(&proc, SIGNAL(readyReadStandardOutput()), this, SLOT(read()));
    connect(&proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
    connect(&proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));

    QStringList args;
    args << _url.toString();
    proc.start("../../pixmapload/debug/pixmaploader", args);
    exec();
}

void Loader::read()
{
    while (proc->bytesAvailable() > 0) {
        QString str(proc->readLine(10000));
        str.chop(2);

        if (str.startsWith("progress: ")) {
            emit progress(str.split(" ").at(1).toInt());
        } else if (str.startsWith("size: ")) {
            QStringList size = str.split(" ");
            emit sizeInfo(size.at(1).toInt(), size.at(2).toInt());
        } else if (str.startsWith("data: ")) {
            QByteArray arr(str.split(" ").at(1).toAscii());
            arr = QByteArray::fromBase64(arr);
            QDataStream data(&arr, QIODevice::ReadWrite);
            QPixmap pix;
            data >> pix;
            emit pixmap(pix);
        }
    }
}

void Loader::finished(int ret,QProcess::ExitStatus stat)
{
    //qDebug() << "DONE " <<  ret << " "<< stat;
}

void Loader::error(QProcess::ProcessError err)
{
    //qDebug() << "error" << err;
}

void Loader::setUrl(QUrl url_)
{
    this->_url = url_;
}

Buoy::Buoy(): QGraphicsPixmapItem(0) {}
Buoy::~Buoy() {}
void Buoy::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "release";
    emit web->buoyClicked();
}
void Buoy::mouseMoveEvent ( QGraphicsSceneMouseEvent * ev )
{
    qDebug() << "move";
}
void Buoy::mousePressEvent ( QGraphicsSceneMouseEvent * ev )
{
    qDebug() << "pressed..";
}


WebItem::WebItem(): QGraphicsWebView()
{
    main = true;
    _isLoaded = false;
    zoom = -1;
    line()->setPen(QPen(QColor(255,255,255,80)));
    QColor colors[] = {
        QColor(255,168,219),
        QColor(255,79,175),
        QColor(215,165,247),
        QColor(171,102,238),
        QColor(255,156,62),
        QColor(255,109,0),
        QColor(126,246,147),
        QColor(11,233,25),
    };
    loadingEllipse()->setBrush(QBrush(colors[rand()%8]));
    loadingEllipse()->setSpanAngle(250);

    //connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loaded(bool)));
    //connect(this, SIGNAL(loadProgress(int)), this, SLOT(loadProgressed(int)));
    //setFlag(QGraphicsItem::ItemClipsToShape);

    connect(&loader, SIGNAL(progress(int)), this, SLOT(loadProgressed(int)), Qt::QueuedConnection);
    connect(&loader, SIGNAL(pixmap(QPixmap)), this, SLOT(pixmapReady(QPixmap)), Qt::QueuedConnection);
    connect(&loader, SIGNAL(sizeInfo(int,int)), this, SLOT(setSize(int,int)), Qt::QueuedConnection);

    progres.start(1000/10);
    connect(&progres, SIGNAL(timeout()), this, SLOT(progressAnim()));
    connect(&kineticTimer, SIGNAL(timeout()), this, SLOT(kineticScrolling()));

    pixmapItem()->web = this;
}

WebItem::~WebItem() {}

void WebItem::loadProgressed(int val)
{
    loadingEllipse()->setSpanAngle(57*val);
}

void WebItem::progressAnim()
{
    QGraphicsEllipseItem *e = loadingEllipse();
    QPen pen = e->pen();
    if (pen.style() == Qt::SolidLine )
        pen.setStyle(Qt::DashLine);
    pen.setDashOffset(pen.dashOffset() + 1);
    pen.setWidth(2);
    e->setPen(pen);
}

QPainterPath WebItem::shape() const
{
     QPainterPath path;
     path.addEllipse(boundingRect());
     return path;
}

void WebItem::stop()
{
    loader.quit();
    progres.stop();
    kineticTimer.stop();
    QGraphicsWebView::stop();
}

void WebItem::setSize(int w, int h)
{
    this->_size = QSizeF(w,h);
}

void WebItem::pixmapReady(QPixmap p)
{
    qDebug() << "pixmap ready";
    this->pixmap = p;
    _pixmapItem.setPixmap(p);

    progres.stop();
    loadingEllipse()->setRect(-10,-10,1,1);

    _isLoaded = true;

    emit pixmapReady();
}

void WebItem::loaded(bool ok)
{
    qDebug() << Q_FUNC_INFO;

    if (!main)
        daa();
    else
        progres.stop();
    //if ()
    /*
    progres.stop();
    if (childItems().count() > 0) {
        QGraphicsRectItem *rect = static_cast<QGraphicsRectItem*>(childItems().at(0));
        QPen pen = rect->pen();
        pen.setStyle(Qt::SolidLine);;
        rect->setPen(pen);
    }

    QTimer::singleShot(500, this, SLOT(sizeChanged()));
    //connect(page()->mainFrame(), SIGNAL(contentsSizeChanged(QSize)),
    //        this, SLOT(sizeChanged(QSize)));
    qDebug() << url() << "loaded";
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (!main)
        setZoomFactor(0.2);
        */
}

void WebItem::daa()
{
    QSize size = page()->mainFrame()->contentsSize();
    size = QSize(800,600);

    view.setScene(&scene);
    view.scene()->addItem(this);
    setPos(-400,-300);
    view.setMinimumSize(size);
    view.setMaximumSize(size);
    view.update();
    QTimer::singleShot(1000, this, SLOT(daa2()));
    QTimer::singleShot(2000, this, SLOT(daa3()));
}

void WebItem::daa2()
{
    QSize size = page()->mainFrame()->contentsSize();
    //setPos(-size.width()/2,-size.height()/2);
    setPos(-400,-300);
    view.setMinimumSize(size);
    view.setMaximumSize(size);
    update();
    view.update();
}
void WebItem::daa3()
{
    QSize size = page()->mainFrame()->contentsSize();
    //pix = pix.grabWidget(web, 0,0,size.width(),size.height());
    pixmap = QPixmap::grabWidget(&view, 0,0,size.width(),size.height());
    _pixmapItem.setPixmap(pixmap.scaled(70,100));

    progres.stop();
    //loadingEllipse()->scene()->removeItem(loadingEllipse());
    loadingEllipse()->setRect(-10,-10,1,1);

    _isLoaded = true;

    emit pixmapReady();

    //view.scene()->removeItem(this);
}


void WebItem::sizeChanged() {
    sizeChanged(page()->mainFrame()->contentsSize());
}

void WebItem::sizeChanged(QSize s)
{
    //qDebug() << Q_FUNC_INFO << ", " << s;
    if (s == QSize(0,0))
        return;

    page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    if (main) {
        if (page()->viewportSize() != s)
            page()->setViewportSize(s);
    } else { // buoy
        qreal w = 80./s.width()*zoomFactor();
        qreal h = 100./s.height()*zoomFactor();
        // qDebug() << s << w << h << zoomFactor();

        qreal z = w<h?w:h;
        setZoomFactor(z);
        if (this->zoom == -1 || this->zoom > z) {
            this->zoom = z;
        }
        if (page()->viewportSize() != QSize(80,100))
            page()->setViewportSize(QSize(80,100));
    }
}


void WebItem::mouseMoveEvent ( QGraphicsSceneMouseEvent * ev )
{
    qDebug() << Q_FUNC_INFO;
    if (!main)
        return;
    struct Event e = {ev->screenPos(), QTime::currentTime()};
    lastEvents.append(e);

    //QGraphicsWebView::mouseMoveEvent(ev);
    qreal dx = lastPos.x() - ev->scenePos().x();
    qreal dy = lastPos.y() - ev->scenePos().y();
    setPos(x() - dx, y() - dy);
    lastPos = ev->scenePos();
}
void WebItem::mousePressEvent ( QGraphicsSceneMouseEvent * ev )
{
    qDebug() << Q_FUNC_INFO;
    lastEvents.clear();
    kineticTimer.stop();

    //QGraphicsWebView::mousePressEvent(ev);
    pressPos = ev->scenePos();
    lastPos = pressPos;
}
void WebItem::mouseReleaseEvent ( QGraphicsSceneMouseEvent * ev )
{
    qDebug() << Q_FUNC_INFO;
    if (!main)
        emit buoyClicked();
    else if (lastEvents.count() > 2){
        //QGraphicsWebView::mouseReleaseEvent(ev);

        // let's find out the kinetics for last (five) events..
        while (lastEvents.count() > 5)
            lastEvents.removeFirst();

        // direction
        kineticDirection = lastEvents.first().xy - lastEvents.last().xy;
        QVector2D s(kineticDirection);
        // v = s/t
        kineticSpeed = s.length()/lastEvents.first().time.msecsTo(lastEvents.last().time);

        qDebug() << kineticDirection << kineticSpeed;
        kineticTimer.start(1000/30);
    }
}

void WebItem::kineticScrolling()
{
    // kinetic speed is points in milliseconds
    QVector2D dxy(-kineticDirection);
    dxy.normalize();
    dxy *= kineticSpeed;
    // this method is called 30 times per second = 33ms periods.
    dxy *= kineticTimer.interval();
    dxy += QVector2D(pos());
    setPos(dxy.x(), dxy.y());
    kineticSpeed *= 0.93; // slow it down
    if (kineticSpeed < 0.001)
        kineticTimer.stop();
}
