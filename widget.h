#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QTreeWidget>
#include "bluetooth/ble.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_scan_clicked();

    void on_pushButton_connect_clicked();

    void on_pushButton_service_clicked();

private:
    Ui::Widget *ui;

    BLE * m_ble;
};
#endif // WIDGET_H
