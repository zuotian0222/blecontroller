#ifndef BLE_H
#define BLE_H

#include <QObject>
#include <QVariant>
#include <QList>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QBluetoothServiceInfo>

class BLE : public QObject
{
    Q_OBJECT
public:
    explicit BLE(QObject *parent = nullptr);
    ~BLE();

    void init();

    //开始扫描设备
    void startScanDevices();
    //连接设备
    void connectDevice(QString address);
    //连接服务
    void connectService(QString uuid);

    bool getScanning(){
        return isScanning;
    }

    QList<QBluetoothDeviceInfo> getDevices(){
        return bleDevicesList;
    }

    QList<QBluetoothUuid> getServicesUUID(){
        return servicesUUIDList;
    }

    QList<QLowEnergyCharacteristic> getChars(){
        return characterList;
    }

private slots:
    // QLowEnergyService related
    void serviceDetailsDiscovered(QLowEnergyService::ServiceState newState);

signals:
    void signal_scanFinished();

    void signal_findservicesFinished();

    void signal_findcharsFinished();

private:
    QBluetoothDeviceDiscoveryAgent * m_DiscoveryAgent;  //设备发现对象

    bool isScanning = false;

    QLowEnergyController *          m_LowController = nullptr;    //中心控制器

    QList<QBluetoothDeviceInfo >    bleDevicesList;

    QBluetoothDeviceInfo            nowDevice;

    QString                         previousAddress;        //之前的设备

    QList<QLowEnergyService>        servicesList;

    QList<QLowEnergyCharacteristic> characterList;

    QList<QBluetoothUuid>           servicesUUIDList;

};

#endif // BLE_H
