#ifndef HTTPMGR_H
#define HTTPMGR_H

#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>

//CRTP
class HttpMgr: public QObject, public Singleton<HttpMgr>, public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT

public:
    ~HttpMgr();

private:
    //call Base Class(Singleton<HttpMgr>) constructor
    friend class Singleton<HttpMgr>;
    HttpMgr();
    QNetworkAccessManager _networkMgr;
    void postHttpReq(QUrl, QJsonObject, HttpReqId, Modules);

private slots:
    void slot_http_finish(HttpReqId, Modules, QString, StatusCode);

signals:
    void sig_http_finish(HttpReqId, Modules, QString, StatusCode);
};

#endif // HTTPMGR_H
