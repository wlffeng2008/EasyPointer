#include "xfdatadecoder.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>

void XFCw::Decoder(QJsonObject cwObj)
{
    sc = cwObj.value(QStringLiteral("sc")).toInt();
    w = cwObj.value(QStringLiteral("w")).toString();

}

void XFWs::Decoder(QJsonObject wsObj)
{
    bg = wsObj.value(QStringLiteral("bg")).toInt();
    //ed = wsObj.value(QStringLiteral("ed")).toInt();
    QJsonArray jsonWSArray = wsObj.value(QStringLiteral("cw")).toArray();
    for (const QJsonValue &value : jsonWSArray)
    {
        XFCw cw;
        cw.Decoder(value.toObject());
        this->cw.append(cw);
    }
}

void XFResult::Decoder(QJsonObject resultObj)
{
    sn = resultObj.value(QStringLiteral("sn")).toInt();
    bg = resultObj.value(QStringLiteral("bg")).toInt();
    ed = resultObj.value(QStringLiteral("ed")).toInt();
    pgs = resultObj.value(QStringLiteral("pgs")).toString();
    ls = resultObj.value(QStringLiteral("ls")).toBool();
    QJsonArray jsonArray = resultObj.value(QStringLiteral("rg")).toArray();
    for (const QJsonValue &value : jsonArray)
    {
        rg.append(value.toInt());
    }
    QJsonArray jsonWSArray = resultObj.value(QStringLiteral("ws")).toArray();
    for (const QJsonValue &value : jsonWSArray)
    {
        XFWs ws;
        ws.Decoder(value.toObject());
        this->ws.append(ws);
    }
}

XFText XFResult::getText()
{
    XFText text;
    text.deleted = false; // default is true?
    text.sn = sn;
    text.bg = bg;
    text.ed = ed;
    text.pgs = pgs;
    text.ls = ls;
    text.rg = rg;
    QString str = "";
    for (const auto &value : ws)
    {
        str += value.cw[0].w;
    }
    text.text = str;
    return text;
}

void XFData::Decoder(QJsonObject dataObj)
{
    status = dataObj["status"].toInt();
    QJsonObject resultObj = dataObj["result"].toObject();
    result.Decoder(resultObj);
}

void XFWSRespData::Decoder(QString strMsg)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strMsg.toUtf8());
    if (jsonDocument.isNull()) {
        qDebug() << "Invalid json string" << strMsg;
        return;
    }
    if (jsonDocument.isObject())
    {
        QJsonObject respObject = jsonDocument.object();
        sid = respObject.value(QStringLiteral("sid")).toString();
        message = respObject.value(QStringLiteral("message")).toString();
        code = respObject.value(QStringLiteral("code")).toInt();
        QJsonObject dataObj = respObject["data"].toObject();
        data.Decoder(dataObj);
    }

    return ;
}


XFDataDecoder::XFDataDecoder()
{

}

void XFDataDecoder::Decode(XFText text)
{
    //qDebug()<<"msg count" << texts.count() << text.deleted;
    if(text.pgs == "rpl")
    {
        for(int i=text.rg[0]-1; i<=text.rg[1]-1; i++)
        {
            texts[i].deleted = true;
        }
    }
    texts.append(text);

}

QString XFDataDecoder::GetString()
{
    QString str = "";
//    for (const XFText &value : texts)
//    {
//        //qDebug()<<"getstring"<<value.deleted<<value.sn<<value.text;
//        if(value.pgs == "apd")
//        {
//            str = "";
//        }
//        if(!value.deleted)
//        {
//            str += value.text;
//        }
//    }
    int listlen = texts.count();
    str = texts[listlen-1].text;
    //qDebug()<<"decode getstring" << str <<"msg count" << texts.count();
    return str;
}
int XFDataDecoder::GetDeleteLength()
{
//    int len = 0;;
//    for (const XFText &value : texts)
//    {
//        if(value.deleted)
//        {
//            len += value.text.count();
//        }
//    }
//    qDebug()<<"decode getdel len" << len;
//    return len;
    int listlen = texts.count();
    int len = 0;
    if(listlen >= 2)
    {
        len = texts[listlen-2].text.count();
        if(texts[listlen-1].pgs == "apd")
        {
            len = 0;
        }
    }
    return len;
}

int XFDataDecoder::getTextcount()
{
    return texts.count();
}

void XFDataDecoder::Diacard()
{
    qDebug()<<"Discard msg..................";
    texts.clear();
}
