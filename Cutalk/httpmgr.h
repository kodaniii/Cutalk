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
    void postHttpReq(QUrl, QJsonObject, HttpReqId, Modules);

private:
    //call Base Class(Singleton<HttpMgr>) constructor
    friend class Singleton<HttpMgr>;
    HttpMgr();
    QNetworkAccessManager _networkMgr;

private slots:
    void slot_http_finish(HttpReqId, Modules, QString, StatusCodes);

signals:
    void sig_http_finish(HttpReqId, Modules, QString, StatusCodes);

    void sig_reg_mod_finish(HttpReqId, QString, StatusCodes);

    void sig_reset_mod_finish(HttpReqId, QString, StatusCodes);
};
#endif // HTTPMGR_H
