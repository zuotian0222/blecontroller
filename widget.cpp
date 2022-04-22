#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_ble = new BLE(this);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_scan_clicked()
{
    connect(m_ble,&BLE::signal_scanFinished,[=]{
        auto list = m_ble->getDevices();
        if(list.isEmpty())
            return ;
        ui->listWidget_dev->clear();
        for(auto s : list){
            ui->listWidget_dev->addItem(s.address().toString());
        }
    });

    if(!m_ble->getScanning())
        m_ble->startScanDevices();
}


void Widget::on_pushButton_connect_clicked()
{
    connect(m_ble,&BLE::signal_findservicesFinished,[=]{
        auto list = m_ble->getServicesUUID();
        if(list.isEmpty())
            return ;
        ui->listWidget_services->clear();
        for(auto s : list){
            ui->listWidget_services->addItem(s.toString());
        }
    });

    m_ble->connectDevice(ui->listWidget_dev->currentItem()->text());
}

void Widget::on_pushButton_service_clicked()
{
    connect(m_ble,&BLE::signal_findcharsFinished,[=]{
        auto list = m_ble->getChars();
        if(list.isEmpty())
            return ;
        ui->listWidget_character->clear();
        for(auto s : list){
            ui->listWidget_character->addItem(s.uuid().toString());
        }

    });

    m_ble->connectService(ui->listWidget_services->currentItem()->text());
}
