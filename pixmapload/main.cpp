#include <QtWebKit>
#include <QDebug>
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include <iostream>

#include "obj.h"

Obj::Obj(QString url): QObject()
{
    std::cout << "url: " << url.toStdString() << "\n";
    fflush(stdout);

    web = new QWebView();
    connect(web, SIGNAL(loadProgress(int)), this, SLOT(writeProgress(int)));
    connect(web, SIGNAL(loadFinished(bool)), this, SLOT(prepareToGrabPage()));
    web->load(QUrl(url));
}

Obj::~Obj() {}

void Obj::writeProgress(int v)
{
    std::cout << "progress: " << v << "\n";
    fflush(stdout);
}
void Obj::prepareToGrabPage()
{
    QTimer::singleShot(500, this, SLOT(grabPage()));
}

void Obj::grabPage()
{
    QSize size = web->page()->mainFrame()->contentsSize();
    QPixmap pix = QPixmap::grabWidget(web, 0,0,size.width(),size.height());
    //show(pix);
    pix = pix.scaled(70, 100);
    //show(pix);

    std::cout << "size: " << size.width() << " " << size.height() << "\n";
    fflush(stdout);

    QByteArray arr;
    QDataStream stream(&arr, QIODevice::ReadWrite);
    stream << pix;
    std::cout << "data: " << QString(arr.toBase64()).toStdString() << "\n";
    fflush(stdout);

    QTimer::singleShot(4000, QApplication::instance(), SLOT(quit()));
}

void Obj::show(QPixmap p)
{
    QGraphicsView *w = new QGraphicsView(new QGraphicsScene());
    QGraphicsPixmapItem *img = new QGraphicsPixmapItem();
    img->setPixmap(p);
    w->scene()->addItem(img);
    img->setPos(0,0);
    w->show();
    w->setMinimumSize(p.size());
    w->setMaximumSize(p.size()*2);
    w->setWindowTitle(web->url().toString());
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Obj *obj = NULL;
    if (argc > 1) {
        std::cout << "start.." << argv[1] << "\n";
        fflush(stdout);
        obj = new Obj(QString(argv[1]));
    }
    return app.exec();
}
