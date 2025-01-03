#include "httpmgr.h"

HttpMgr::HttpMgr(){
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

HttpMgr::~HttpMgr(){

}

//发http post req
void HttpMgr::postHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod){
    QByteArray data = QJsonDocument(json).toJson();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));

    auto self = shared_from_this();

    QNetworkReply *reply = _networkMgr.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod](){
        //连接失败
        if(reply->error() != QNetworkReply::NoError){
            qDebug() << "HttpMgr reply" << reply->errorString();
            emit self->sig_http_finish(req_id, mod, "", StatusCodes::GateFailed); // -> &RegisterDialog::slot_reg_mod_finish
            reply->deleteLater();
            return;
        }

        QString res = reply->readAll();
        emit self->sig_http_finish(req_id, mod, res, StatusCodes::Success);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(ReqId req_id, Modules mod, QString res, StatusCodes statusCode){
    switch (mod){
    case Modules::MOD_REGISTER:
        qDebug() << "HttpMgr::sig_reg_mod_finish statusCode =" << statusCode;
        emit sig_reg_mod_finish(req_id, res, statusCode);
        break;
    case Modules::MOD_RESET:
        //TODO Modules::MOD_RESET, is ok
        qDebug() << "HttpMgr::sig_reset_mod_finish statusCode =" << statusCode;
        emit sig_reset_mod_finish(req_id, res, statusCode);
        break;
    case Modules::MOD_LOGIN:
        qDebug() << "HttpMgr::slot_http_finish statusCode =" << statusCode;
        emit sig_login_mod_finish(req_id, res, statusCode);
        break;
    }
}
