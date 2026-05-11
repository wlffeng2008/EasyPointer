#ifndef XFDATADECODER_H
#define XFDATADECODER_H

#include <QJsonObject>
#include <QObject>
#include <QVector>


class XFText{
public:
    int sn;
    int bg;
    int ed;
    QString text;
    QString pgs;
    QByteArray rg;
    bool deleted;
    bool ls;
    //JsonObject vad;
};

class XFCw {
public:
    int sc;
    QString w;
    void Decoder(QJsonObject cwObj);
};

class XFWs{
public:
    int bg;
    //int ed;
    QList<XFCw> cw;
    void Decoder(QJsonObject wsObj);
};

class XFResult{
public:
    int sn;
    bool ls;
    int bg;
    int ed;
    QString pgs;
    QByteArray rg;
    QList<XFWs>  ws;
    void Decoder(QJsonObject resultObj);
    XFText getText();
};

class XFData {
public:
    int status;
    XFResult result;

    void Decoder(QJsonObject dataObj);
};

class XFWSRespData
{
public:
    QString sid;
    QString message;
    int code;
    XFData data;

    void Decoder(QString strMsg);
};

class XFDataDecoder
{
public:
    XFDataDecoder();
    void Decode(XFText text);
    QString GetString();
    int GetDeleteLength();
    void Diacard();
    int getTextcount();
private:
    //QArrayData<XFText> texts;
    //QList<XFText> texts;
    QVector<XFText> texts;
    int defc = 10;
};

#endif // XFDATADECODER_H
