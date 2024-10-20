#include "httpmgr.h"

HttpMgr::HttpMgr(){
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

HttpMgr::~HttpMgr(){

}

void HttpMgr::postHttpReq(QUrl url, QJsonObject json, HttpReqId req_id, Modules mod){
    QByteArray data = QJsonDocument(json).toJson();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));

    auto self = shared_from_this();

    QNetworkReply *reply = _networkMgr.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod](){
        if(reply->error() != QNetworkReply::NoError){
            qDebug() << "HttpMgr reply " << reply->errorString();
            emit self->sig_http_finish(req_id, mod, "", StatusCode::ERR_NETWORK);
            reply->deleteLater();
            return;
        }

        QString res = reply->readAll();
        emit self->sig_http_finish(req_id, mod, res, StatusCode::SUCCESS);
        reply->deleteLater();
        return;
    });

}

void HttpMgr::slot_http_finish(HttpReqId req_id, Modules mod, QString res, StatusCode statusCode){

}
