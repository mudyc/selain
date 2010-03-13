#ifndef WEBITEM_H
#define WEBITEM_H

#include <QGraphicsWebView>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QTimer>
#include <QTime>

#include <QThread>
#include <QPixmap>

#include <QThread>
#include <QProcess>
#include <QSharedMemory>

class Loader: public QThread
{
    Q_OBJECT
public:
    Loader();
    virtual void run();
    void setUrl(QUrl url);
    inline QUrl url() { return this->_url; }
private slots:
    void read();
    void finished(int,QProcess::ExitStatus);
    void error(QProcess::ProcessError err);
signals:
    void progress(int v);
    void sizeInfo(int w, int h);
    void pixmap(QPixmap p);
private:
    QProcess *proc;
    QUrl _url;
};


class WebItem;
class Buoy: public QGraphicsPixmapItem
{
public:
    Buoy();
    virtual ~Buoy();
    WebItem *web;
protected:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *ev);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *ev);

private:
};


class WebItem: public QGraphicsWebView
{
    Q_OBJECT
public:

    WebItem();
    virtual ~WebItem();

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *ev);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *ev);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev);
    virtual void stop();
    virtual QPainterPath shape() const;

    inline void setBuoy(bool isBuoy) { main = !isBuoy; }

    inline QGraphicsLineItem *line() { return &_line; }
    inline Buoy *pixmapItem() { return &_pixmapItem; }
    inline QGraphicsRectItem *bgRect() { return &_bgRect; }
    inline QGraphicsEllipseItem *loadingEllipse() { return &_ellipse; }


    inline bool isLoaded() { return _isLoaded; }

    inline QSizeF fullSize() { return _size; }

public slots:
    void daa();
    void daa2();
    void daa3();

signals:
    void buoyClicked();
    void pixmapReady();

private slots:
    void setSize(int w, int h);
    void pixmapReady(QPixmap p);
    void loaded(bool ok);
    void sizeChanged();
    void sizeChanged(QSize s);
    // animates the edges of loading progress
    void progressAnim();
    void loadProgressed(int val);
    void kineticScrolling();
private:
    bool main; // or buoy
    qreal zoom;
    bool _isLoaded;
    QPixmap pixmap;
    QGraphicsView view;
    QGraphicsScene scene;

// the size of full document
    QSizeF _size;

// panning
    QPointF pressPos, lastPos;

// loading of page period
    Loader loader;
    QTimer progres;
    QGraphicsEllipseItem _ellipse;

// loaded phase
    Buoy _pixmapItem;
    QGraphicsRectItem _bgRect;
    QGraphicsLineItem _line;


// Kinetics
    struct Event {
        QPoint xy;
        QTime time;
    };
    QList<struct Event> lastEvents;
    QPointF kineticDirection;
    qreal kineticSpeed;
    QTimer kineticTimer;

    friend class Buoy;
    friend class Widget;
};



#endif // WEBITEM_H
