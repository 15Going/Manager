#include "widget.h"
#include "ui_widget.h"
#include "pieview.h"

#include <QSqlQueryModel>
#include <QSplitter>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QInputDialog>
#include <QPalette>
#include <QTableView>
#include <QDateTime>
#include <QFile>
#include <QStandardItemModel>
#include <QRandomGenerator>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    setFixedSize(750,500);

    QSqlQueryModel *typeModel = new QSqlQueryModel(this);
    typeModel->setQuery("select name from type");
    ui->sellTypeComboBox->setModel(typeModel);

    ui->sellTypeComboBox_2->setModel(typeModel);

    QSplitter *splitter = new QSplitter(ui->managePage);
    splitter->resize(700,360);
    splitter->move(0,50);
    splitter->addWidget(ui->toolBox);
    splitter->addWidget(ui->dailyList);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,1);

    on_sellCancelBtn_clicked();

    ui->incTipLabel_type->setStyleSheet("color:red;");
    ui->incTipLabel_type->setText(tr("*不能为空"));
    ui->incTipLabel_brand->setStyleSheet("color:red;");
    ui->incTipLabel_type->setText(tr("*不能为空"));
    ui->incTipLabel_price->setStyleSheet("color:red;");
    ui->incTipLabel_type->setText(tr("*不能为空"));

    ui->stackedWidget->setCurrentIndex(0);
    showDailyList();

    ui->typeComboBox->setModel(typeModel);
    createChartModelView();
}

Widget::~Widget()
{
    delete ui;
}

QString Widget::getDateTime(DateTimeType type)
{
    QDateTime datetime = QDateTime::currentDateTime();
    QString date = datetime.toString("yyyy-MM-dd");
    QString time = datetime.toString("hh:mm");
    QString dateAndTime = datetime.toString("yyyy-MM-dd dddd hh:mm");//dddd表示星期
    if(type == Date)
        return date;
    else if(type == Time)
        return time;
    else
        return dateAndTime;
}


void Widget::on_sellTypeComboBox_currentTextChanged(const QString &arg1)
{
    if(arg1 == "请选择类型"){
        on_sellCancelBtn_clicked();
    }else{
        ui->sellBrandComboBox->setEnabled(true);
        QSqlQueryModel *model = new QSqlQueryModel(this);
        model->setQuery(QString("select name from brand where type = '%1'")
                        .arg(arg1));
        ui->sellBrandComboBox->setModel(model);
        ui->sellCancelBtn->setEnabled(true);
    }
}


void Widget::on_sellBrandComboBox_currentTextChanged(const QString &arg1)
{
    ui->sellNumSpinBox->setValue(0);
    ui->sellNumSpinBox->setEnabled(false);
    ui->sellSumLineEdit->clear();
    ui->sellSumLineEdit->setEnabled(false);
    ui->sellOkBtn->setEnabled(false);

    QSqlQuery query;
    query.exec(QString("select price from brand where name = '%1' and type = '%2'")
               .arg(arg1).arg(ui->sellTypeComboBox->currentText()));
    query.next();
    ui->sellPriceLineEdit->setEnabled(true);
    ui->sellPriceLineEdit->setReadOnly(true);
    ui->sellPriceLineEdit->setText(query.value(0).toString());

    query.exec(QString("select last from brand where name = '%1' and type = '%2'")
               .arg(arg1).arg(ui->sellTypeComboBox->currentText()));
    query.next();
    int num = query.value(0).toInt();
    if(num == 0)
        QMessageBox::information(this,tr("提示"),tr("该商品已经售完！"),QMessageBox::Ok);
    else{
        ui->sellNumSpinBox->setEnabled(true);
        ui->sellNumSpinBox->setMaximum(num);
        ui->sellLastNumLabel->setText(tr("剩余数量：%1").arg(num));
        ui->sellLastNumLabel->setVisible(true);
    }
}


void Widget::on_sellNumSpinBox_valueChanged(int arg1)
{
    if(arg1 == 0){
        ui->sellSumLineEdit->clear();
        ui->sellSumLineEdit->setEnabled(false);
        ui->sellOkBtn->setEnabled(false);
    }else{
        ui->sellSumLineEdit->setEnabled(true);
        ui->sellSumLineEdit->setReadOnly(true);
        qreal sum = arg1 * ui->sellPriceLineEdit->text().toInt();
        ui->sellSumLineEdit->setText(QString::number(sum));
        ui->sellOkBtn->setEnabled(true);
    }
}


void Widget::on_sellCancelBtn_clicked()
{
    ui->sellTypeComboBox->setCurrentIndex(0);
    ui->sellBrandComboBox->clear();
    ui->sellBrandComboBox->setEnabled(false);
    ui->sellPriceLineEdit->clear();
    ui->sellPriceLineEdit->setEnabled(false);
    ui->sellNumSpinBox->setValue(0);
    ui->sellNumSpinBox->setEnabled(false);
    ui->sellSumLineEdit->clear();
    ui->sellSumLineEdit->setEnabled(false);
    ui->sellOkBtn->setEnabled(false);
    ui->sellCancelBtn->setEnabled(false);
    ui->sellLastNumLabel->setVisible(false);
}


void Widget::on_sellOkBtn_clicked()
{
    QString type = ui->sellTypeComboBox->currentText();
    QString name = ui->sellBrandComboBox->currentText();
    int value = ui->sellNumSpinBox->value();
    int last = ui->sellNumSpinBox->maximum() - value;

    QSqlQuery query;
    query.exec(QString("select sell from brand where name = '%1' and type = '%2'")
               .arg(name).arg(type));
    query.next();
    int sell = query.value(0).toInt() + value;

    //事务操作
    QSqlDatabase::database().transaction();
    bool rtn = query.exec(
                QString("update brand set sell = %1,last = %2 where name = '%3'"
                        " and type = '%4'").arg(sell).arg(last).arg(name).arg(type));
    if(rtn){
        QSqlDatabase::database().commit();
        QMessageBox::information(this,tr("提示"),tr("购买成功！"),QMessageBox::Ok);
        writeXml();
        showDailyList();
        on_sellCancelBtn_clicked();
    }else{
        QSqlDatabase::database().rollback();
    }
}


void Widget::on_sellTypeComboBox_2_currentTextChanged(const QString &arg1)
{
    if(arg1 == "请选择类型"){
        on_sellCancelBtn_2_clicked();
    }else{
        ui->sellBrandComboBox_2->setEnabled(true);
        QSqlQueryModel *model = new QSqlQueryModel(this);
        model->setQuery(QString("select name from brand where type = '%1'")
                   .arg(arg1));
        ui->sellBrandComboBox_2->setModel(model);
        ui->sellCancelBtn_2->setEnabled(true);
    }
}


void Widget::on_sellBrandComboBox_2_currentTextChanged(const QString &arg1)
{
    QString type = ui->sellTypeComboBox_2->currentText();
    QSqlQuery query;
    query.exec(QString("select price from brand where name = '%1' and type = '%2'")
               .arg(arg1).arg(type));
    query.next();
    qreal price = query.value(0).toInt();
    ui->sellPriceLineEdit_2->setText(QString::number(price));
    ui->sellPriceLineEdit_2->setEnabled(true);
    ui->sellPriceLineEdit_2->setReadOnly(true);
    ui->sellNumSpinBox_2->setEnabled(true);
}


void Widget::on_sellNumSpinBox_2_valueChanged(int arg1)
{
    ui->sellOkBtn_2->setEnabled(true);
    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery("SELECT id, name FROM type");
    model->setHeaderData(0, Qt::Horizontal, tr("id"));
    model->setHeaderData(1, Qt::Horizontal, tr("name"));
    QTableView *view = new QTableView;
    view->setModel(model);
    view->show();
    int row = model->rowCount();
    qDebug()<<row;
}


void Widget::on_sellCancelBtn_2_clicked()
{
    ui->sellTypeComboBox_2->setCurrentIndex(0);
    ui->sellBrandComboBox_2->clear();
    ui->sellBrandComboBox_2->setEnabled(false);
    ui->sellPriceLineEdit_2->clear();
    ui->sellPriceLineEdit_2->setEnabled(false);
    ui->sellNumSpinBox_2->setValue(0);
    ui->sellNumSpinBox_2->setEnabled(false);
    ui->sellOkBtn_2->setEnabled(false);
    ui->sellCancelBtn_2->setEnabled(false);
}


void Widget::on_sellOkBtn_2_clicked()
{
    QString name = ui->sellBrandComboBox_2->currentText();
    QString type = ui->sellTypeComboBox_2->currentText();
    QSqlQuery query;
    query.exec(QString("select last from brand where name = '%1' and type = '%2'")
               .arg(name).arg(type));
    query.next();
    int oldLast = query.value(0).toInt();
    int last = ui->sellNumSpinBox_2->value() + oldLast;

    query.exec(QString("select sum from brand where name = '%1' and type = '%2'")
               .arg(name).arg(type));
    query.next();
    int oldSum = query.value(0).toInt();
    int sum = ui->sellNumSpinBox_2->value() + oldSum;

    QSqlDatabase::database().transaction();
    bool rbt = query.exec(
                QString("update brand set sum = '%1',last = '%2' where name = '%3' and type = '%4'")
                .arg(sum).arg(last).arg(name).arg(type));
    if(rbt == true){
        QSqlDatabase::database().commit();
        QMessageBox::information(this,tr("提示"),tr("商品入库成功!"),QMessageBox::Ok);
        on_sellCancelBtn_2_clicked();
    }else
        QSqlDatabase::database().rollback();

//    query.exec(QString("select sum from brand where name = '%1' and type = '%2'")
//               .arg(name).arg(type));
//    query.next();
//    int n = query.value(0).toInt();
//    qDebug()<<n;
}



void Widget::on_sellOkBtn_new_clicked()
{
    QString type = ui->sellTypeLineEdit_new->text();
    if(type.isEmpty()){
        QMessageBox::warning(this,tr("警告"),tr("类型不能为空！"),QMessageBox::Ok);
        return;
    }
    QString name = ui->sellbrandLineEdit_new->text();
    if(name.isEmpty()){
        QMessageBox::warning(this,tr("警告"),tr("品牌不能为空！"),QMessageBox::Ok);
        return;
    }
    int price = ui->sellPriceLineEdit_new->text().toInt();
    if(price == 0){
        QMessageBox::warning(this,tr("警告"),tr("单价不能为空！"),QMessageBox::Ok);
        return;
    }
    int sum = ui->sellNumSpinBox_new->value();

    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(QString("select id from type"));
    int typeRow = model->rowCount();
    model->setQuery(QString("select id from brand"));
    int brandRow = model->rowCount();

    //事务操作
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.exec(QString("select name from type where name = '%1'")
                         .arg(type));
    if(!query.next()){
        bool ok1 = query.exec(QString("insert into type values('0%1','%2')")
                              .arg(typeRow + 1).arg(type));
        if(ok1)
            QSqlDatabase::database().commit();
    }
    //事务操作
    QSqlDatabase::database().transaction();
    bool ok2;
    ok2 = query.exec(QString("insert into brand values('0%1','%2','%3',%4,%5,0,%6)")
                     .arg(brandRow+1).arg(name).arg(type).arg(price).arg(sum).arg(sum));
    if(ok2){
        QSqlDatabase::database().commit();
        QMessageBox::information(this,tr("提示"),tr("添加成功"),QMessageBox::Ok);

        on_sellCancelBtn_new_clicked();
    }else{
        QSqlDatabase::database().rollback();
    }
}


void Widget::on_sellTypeLineEdit_new_textChanged(const QString &arg1)
{
    if(!arg1.isEmpty())
        ui->incTipLabel_type->hide();
}


void Widget::on_sellbrandLineEdit_new_textChanged(const QString &arg1)
{
    if(!arg1.isEmpty())
        ui->incTipLabel_brand->hide();
}


void Widget::on_sellPriceLineEdit_new_textChanged(const QString &arg1)
{
    if(!arg1.isEmpty())
        ui->incTipLabel_price->hide();
}


void Widget::on_sellCancelBtn_new_clicked()
{
    ui->sellTypeLineEdit_new->clear();
    ui->sellbrandLineEdit_new->clear();
    ui->sellNumSpinBox_new->clear();
    ui->sellPriceLineEdit_new->clear();
}

bool Widget::docRead()
{
    QFile file("data.xml");
    if(!file.open(QIODevice::ReadOnly))
        return false;
    if(!doc.setContent(&file)){
        file.close();
        return false;
    }
    file.close();
    return true;

}

bool Widget::docWrite()
{
    QFile file("data.xml");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    QTextStream out(&file);
    doc.save(out,4);//保存到文件
    file.close();
    return true;
}

void Widget::writeXml()
{
    if(docRead()){
        QString currentDate = getDateTime(Date);
        QDomElement root = doc.documentElement();
        if(!root.hasChildNodes()){
            QDomElement date = doc.createElement(QString("日期"));
            QDomAttr curDate = doc.createAttribute("date");
            curDate.setValue(currentDate);
            date.setAttributeNode(curDate);
            root.appendChild(date);
            createNodes(date);
        }else{
            QDomElement date = root.lastChild().toElement();
            if(date.attribute("date") == currentDate)
                createNodes(date);
            else{
                QDomElement date = doc.createElement(QString("日期"));
                QDomAttr curDate = doc.createAttribute("date");
                curDate.setValue(currentDate);
                date.setAttributeNode(curDate);
                root.appendChild(date);
                createNodes(date);
            }
        }
        docWrite();
    }
}

void Widget::createNodes(QDomElement &date)
{
    QDomElement time = doc.createElement(QString("时间"));
    QDomAttr curTime = doc.createAttribute("time");
    curTime.setValue(getDateTime(Time));

    time.setAttributeNode(curTime);
    date.appendChild(time);

    QDomElement type = doc.createElement(QString("类型"));
    QDomElement brand = doc.createElement(QString("品牌"));
    QDomElement price = doc.createElement(QString("单价"));
    QDomElement num = doc.createElement(QString("数量"));
    QDomElement sum = doc.createElement(QString("金额"));
    QDomText text;
    text = doc.createTextNode(QString("%1")
                              .arg(ui->sellTypeComboBox->currentText()));
    type.appendChild(text);
    text = doc.createTextNode(QString("%1")
                              .arg(ui->sellBrandComboBox->currentText()));
    brand.appendChild(text);
    text = doc.createTextNode(QString("%1")
                              .arg(ui->sellPriceLineEdit->text()));
    price.appendChild(text);
    text = doc.createTextNode(QString("%1")
                              .arg(ui->sellNumSpinBox->value()));
    num.appendChild(text);
    text = doc.createTextNode(QString("%1")
                              .arg(ui->sellSumLineEdit->text()));
    sum.appendChild(text);

    time.appendChild(type);
    time.appendChild(brand);
    time.appendChild(price);
    time.appendChild(num);
    time.appendChild(sum);
}

void Widget::showDailyList()
{
    ui->dailyList->clear();
    if(docRead()){
        QDomElement root = doc.documentElement();
        QString title = root.tagName();
        QListWidgetItem *titleItem = new QListWidgetItem;
        titleItem->setText(QString("-----%1-----").arg(title));
        titleItem->setTextAlignment(Qt::AlignCenter);
        ui->dailyList->addItem(titleItem);

        if(root.hasChildNodes()){
            QString currentDate = getDateTime(Date);
            QDomElement dateElement = root.lastChild().toElement();
            QString date = dateElement.attribute("date");
            if(date == currentDate){
                ui->dailyList->addItem("");
                ui->dailyList->addItem(QString("日期：%1").arg(date));
                ui->dailyList->addItem("");

                QDomNodeList children = dateElement.childNodes();
                for(int i = 0;i < children.count(); ++i){
                    QDomNode node = children.at(i);
                    QString time = node.toElement().attribute("time");
                    QDomNodeList list = node.childNodes();
                    QString type = list.at(0).toElement().text();
                    QString brand = list.at(1).toElement().text();
                    QString price = list.at(2).toElement().text();
                    QString num = list.at(3).toElement().text();
                    QString sum = list.at(4).toElement().text();

                    QString str = time + "出售" + brand + type + " " +
                            num + "台," + "单价：" + price +"元，共" + sum + "元";
                    QListWidgetItem *tempItem = new QListWidgetItem;
                    tempItem->setText("******************************");
                    tempItem->setTextAlignment(Qt::AlignCenter);
                    ui->dailyList->addItem(tempItem);
                    ui->dailyList->addItem(str);
                }
            }
        }
    }
}

void Widget::createChartModelView()
{
    chartModel = new QStandardItemModel(this);
    chartModel->setColumnCount(2);
    chartModel->setHeaderData(0,Qt::Horizontal,QString("品牌"));
    chartModel->setHeaderData(1,Qt::Horizontal,QString("销售数量"));

    QSplitter *splitter = new QSplitter(ui->chartPage);
    splitter->resize(700,320);
    splitter->move(0,80);
    QTableView *table = new QTableView;
    PieView *pieChart = new PieView;
    splitter->addWidget(table);
    splitter->addWidget(pieChart);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,2);

    table->setModel(chartModel);
    pieChart->setModel(chartModel);

    QItemSelectionModel *selectionModel = new QItemSelectionModel(chartModel);
    table->setSelectionModel(selectionModel);
    pieChart->setSelectionModel(selectionModel);
}

void Widget::showChart()
{
    QSqlQuery query;
    query.exec(QString("select name,sell from brand where type = '%1'")
               .arg(ui->typeComboBox->currentText()));

    chartModel->removeRows(0,chartModel->rowCount(QModelIndex()),
                           QModelIndex());
    int row = 0;
    while(query.next()){
        int r = QRandomGenerator::global()->bounded(256);
        int g = QRandomGenerator::global()->bounded(256);
        int b = QRandomGenerator::global()->bounded(256);

        chartModel->insertRows(row,1,QModelIndex());
        chartModel->setData(chartModel->index(row,0,QModelIndex()),
                            query.value(0).toString());
        chartModel->setData(chartModel->index(row,1,QModelIndex()),
                            query.value(1).toInt());
        chartModel->setData(chartModel->index(row,0,QModelIndex()),
                            QColor(r,g,b),Qt::DecorationRole);
        ++row;
    }

}


void Widget::on_typeComboBox_currentTextChanged(const QString &arg1)
{
    if(arg1 != "请选择类型")
        showChart();

}


void Widget::on_updateBtn_clicked()
{
    if(ui->typeComboBox->currentText() != "请选择类型")
        showChart();
}


void Widget::on_manageBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void Widget::on_chartBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

//修改密码
void Widget::on_passwordBtn_clicked()
{
    bool ok;
    QString password = QInputDialog::getText(this,tr("修改密码"),
                                             tr("请输入新密码："),QLineEdit::Normal
                                             ,QString(),&ok);
    if(ok){
        if(password.isEmpty()){
            QMessageBox::warning(this,tr("警告"),tr("新密码不能为空\n请重新点击修改密码"),
                                 QMessageBox::Ok);
            return;
        }
        //事务操作
        QSqlDatabase::database().transaction();
        QSqlQuery query;
        bool rbt = query.exec(QString("update password set pwd = '%1'").arg(password));
        if(rbt){
            QSqlDatabase::database().commit();
            QMessageBox::information(this,tr("提示"),tr("修改密码成功！"),QMessageBox::Ok);
        }else{
            QMessageBox::information(this,tr("提示"),tr("修改密码失败！"),QMessageBox::Ok);
            QSqlDatabase::database().rollback();
        }
    }else
        return;
}

