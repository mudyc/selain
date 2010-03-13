#ifndef OBJ_H
#define OBJ_H

#include <QObject>
#include <QtWebKit>
#include <QSharedMemory>


class Obj: public QObject {
    Q_OBJECT

public:
    Obj(QString url);
    virtual ~Obj();

private slots:
    void writeProgress(int v);
    void prepareToGrabPage();
    void grabPage();

private:
    void show(QPixmap p);
    QWebView *web;
};


#endif // OBJ_H
