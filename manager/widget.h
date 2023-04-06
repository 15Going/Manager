#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDomDocument>

class QStandardItemModel;

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    enum DateTimeType{Time,Date,DateTime};
    QString getDateTime(DateTimeType type);

private slots:
    void on_sellTypeComboBox_currentTextChanged(const QString &arg1);

    void on_sellBrandComboBox_currentTextChanged(const QString &arg1);

    void on_sellNumSpinBox_valueChanged(int arg1);

    void on_sellCancelBtn_clicked();

    void on_sellOkBtn_clicked();

    void on_sellTypeComboBox_2_currentTextChanged(const QString &arg1);

    void on_sellBrandComboBox_2_currentTextChanged(const QString &arg1);

    void on_sellNumSpinBox_2_valueChanged(int arg1);

    void on_sellCancelBtn_2_clicked();

    void on_sellOkBtn_2_clicked();


    void on_sellOkBtn_new_clicked();

    void on_sellTypeLineEdit_new_textChanged(const QString &arg1);

    void on_sellbrandLineEdit_new_textChanged(const QString &arg1);

    void on_sellPriceLineEdit_new_textChanged(const QString &arg1);

    void on_sellCancelBtn_new_clicked();

    void on_typeComboBox_currentTextChanged(const QString &arg1);

    void on_updateBtn_clicked();

    void on_manageBtn_clicked();

    void on_chartBtn_clicked();

    void on_passwordBtn_clicked();

private:
    Ui::Widget *ui;

    QDomDocument doc;

    QStandardItemModel *chartModel;

private:
    bool docRead();
    bool docWrite();
    void writeXml();
    void createNodes(QDomElement &date);
    void showDailyList();


    void createChartModelView();
    void showChart();
};
#endif // WIDGET_H
