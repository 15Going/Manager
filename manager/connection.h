#ifndef CONNECTION_H
#define CONNECTION_H

#include <QtSql>
#include <QDebug>
#include <QDomDocument>

static bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    db.setHostName("yafeilinux");
    db.setDatabaseName("data.db");
    db.setUserName("yafei");
    db.setPassword("123456");
    if(!db.open())
       return false;

    QSqlQuery query;
    query.exec("create table type(id varchar primary key,name varchar)");
    query.exec(QString("insert into type values('0','请选择类型')"));
    query.exec(QString("insert into type values('01','电视')"));
    query.exec(QString("insert into type values('02','空调')"));

    query.exec("create table brand(id varchar primary key,name varchar,"
               "type varchar,price int,sum int,sell int,last int)");
    query.exec(QString("insert into brand values('01','海信','电视',2444,50,10,40)"));
    query.exec(QString("insert into brand values('02','海尔','空调',3588,25,10,15)"));
    query.exec(QString("insert into brand values('03','创维','电视',9000,90,50,40)"));
    query.exec(QString("insert into brand values('04','海尔','电视',3764,65,35,30)"));
    query.exec(QString("insert into brand values('05','王牌','电视',4900,80,30,50)"));
    query.exec(QString("insert into brand values('06','格力','空调',1470,70,25,50)"));

    query.exec("create table password(pwd varchar)");
    query.exec("insert into password values('123456')");

    return true;
}

static bool createXml()
{
    QFile file("data.xml");
    if(file.exists())
        return true;
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    QDomDocument doc;
    QDomProcessingInstruction instruction;
    instruction = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);
    QDomElement root = doc.createElement(QString("日销售清单"));
    doc.appendChild(root);
    QTextStream out(&file);
    doc.save(out,4);
    file.close();
    return true;
}

#endif // CONNECTION_H
