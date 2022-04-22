#include "ble.h"

BLE::BLE(QObject *parent) : QObject(parent)
{
    init();
}

BLE::~BLE()
{
    delete m_LowController;
    delete m_DiscoveryAgent;
    servicesList.clear();
    characterList.clear();
    bleDevicesList.clear();
}

void BLE::init()
{
    m_DiscoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    m_DiscoveryAgent->setLowEnergyDiscoveryTimeout(5000);
    connect(m_DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,[=](const QBluetoothDeviceInfo &info){
        if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration){
            qDebug()<<info.name()<<info.address().toString();
            bleDevicesList.append(info);
        }
    });
    connect(m_DiscoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),[=](QBluetoothDeviceDiscoveryAgent::Error error){
        if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError){
            qDebug("The Bluetooth is powered off.");
        }
        else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError) {
            qDebug("Writing or reading from the device resulted in an error.");
        }
    });
    connect(m_DiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, [=]{
        if (bleDevicesList.isEmpty())
            qDebug("No Low Energy devices found...");
        else
            qDebug("Scan finished!");
        emit signal_scanFinished();
        isScanning = false;
    });
}

//开始扫描设备
void BLE::startScanDevices()
{
    //清空设备列表
    bleDevicesList.clear();
    //开始扫描
    m_DiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    if(m_DiscoveryAgent->isActive()){
        qDebug("Scanning.\n");
        isScanning = true;
    }
}

//连接设备
void BLE::connectDevice(QString address)
{
    for(auto dev : bleDevicesList){
        if(dev.address().toString() == address){
            nowDevice = dev;
        }
    }
    if(!nowDevice.isValid()){
        qDebug("Not a valid device");
        return;
    }

    servicesList.clear();
    characterList.clear();

    if (m_LowController && previousAddress != nowDevice.address().toString()) {
        m_LowController->disconnectFromDevice();
        delete m_LowController;
        m_LowController = nullptr;
    }

    if (!m_LowController) {
        m_LowController = QLowEnergyController::createCentral(nowDevice);
        connect(m_LowController, &QLowEnergyController::connected,[=](){
            qDebug("Device connect success.");
            m_LowController->discoverServices();
        });
        connect(m_LowController, QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error),[=](QLowEnergyController::Error){
            qDebug()<<"Error : "+m_LowController->errorString();
        });
        connect(m_LowController, &QLowEnergyController::disconnected,[=]{
            qDebug("Device disconnected.");
        });
        connect(m_LowController, &QLowEnergyController::serviceDiscovered,[=]{
            //
        });
        connect(m_LowController, &QLowEnergyController::discoveryFinished,[=]{
            qDebug("Services scan finished.");

            servicesUUIDList = m_LowController->services();
            for(auto s : servicesUUIDList){
                qDebug()<<s.toString();
            }
            emit signal_findservicesFinished();
        });
    }

    m_LowController->connectToDevice();
    previousAddress = nowDevice.address().toString();
}

void BLE::connectService(QString uuid)
{
    QLowEnergyService *service = nullptr;

    for(int i=0;i<servicesUUIDList.length();i++)
    {
        if(servicesUUIDList.at(i).toString() == uuid){
            service = m_LowController->createServiceObject(servicesUUIDList.at(i));
        }
    }

    if (!service)
        return;
    characterList.clear();


    if (service->state() == QLowEnergyService::DiscoveryRequired) {
        connect(service, &QLowEnergyService::stateChanged,this, &BLE::serviceDetailsDiscovered);
        service->discoverDetails();
        return;
    }
}

void BLE::serviceDetailsDiscovered(QLowEnergyService::ServiceState newState)
{
    qDebug()<<"State : "<<newState;

    if (newState != QLowEnergyService::ServiceDiscovered) {
        if (newState != QLowEnergyService::DiscoveringServices) {

        }
        return;
    }
    auto service = qobject_cast<QLowEnergyService *>(sender());
    if (!service){
        return;
    }

    //BLE设备特征值改变
    connect(service, &QLowEnergyService::characteristicChanged,[=](const QLowEnergyCharacteristic &c, const QByteArray &value){
        QString Charuuid = c.uuid().toString();
        QString Value = value.toHex();
        qDebug()<<"BLE设备-"+Charuuid+"特性值发生变化："+ Value;
    });
    //当特征读取请求成功返回其值时，发出此信号。
    connect(service,&QLowEnergyService::characteristicRead,[=](const QLowEnergyCharacteristic &c, const QByteArray &value){
        QString Charname = c.uuid().toString();
        QString Value = value.toHex();
        qDebug()<<"BLE设备-"+Charname+"特性值读取到的值："+ Value;
    });
    //当特性值成功更改为newValue时，会发出此信号。
    connect(service,&QLowEnergyService::characteristicWritten,[=](const QLowEnergyCharacteristic &c, const QByteArray &value){
        QString Charname = c.uuid().toString();
        QString Value = value.toHex();
        qDebug()<<"BLE设备-"+Charname+"特性值成功写入值："+ Value;
    });
    //描述符修改
    connect(service,&QLowEnergyService::descriptorWritten,[=](const QLowEnergyDescriptor &descriptor, const QByteArray &newValue){
        QString Charname = QString("%1").arg(descriptor.name());
        qDebug()<<"BLE设备-"+Charname+"描述符成功写入值："+ QString(newValue);
    });
    //各种错误信息
    connect(service, static_cast<void(QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
            [=](QLowEnergyService::ServiceError newErrorr)
    {
        if(QLowEnergyService::NoError == newErrorr){
            qDebug()<<"没有发生错误。\n";
        }
        if(QLowEnergyService::OperationError==newErrorr){
            qDebug()<<"错误: 当服务没有准备好时尝试进行操作!\n";
        }
        if(QLowEnergyService::CharacteristicReadError==newErrorr){
            qDebug()<<"尝试读取特征值失败!\n";
        }
        if(QLowEnergyService::CharacteristicWriteError==newErrorr){
            qDebug()<<"尝试为特性写入新值失败!\n";
        }
        if(QLowEnergyService::DescriptorReadError==newErrorr){
            qDebug()<<"尝试读取描述符值失败!\n";
        }
        if(QLowEnergyService::DescriptorWriteError==newErrorr){
            qDebug()<<"尝试向描述符写入新值失败!\n";
        }
        if(QLowEnergyService::UnknownError==newErrorr){
            qDebug()<<"与服务交互时发生未知错误!\n";
        }
    });

    characterList = service->characteristics();
    emit signal_findcharsFinished();

    for(auto s : characterList){
        qDebug()<<s.uuid();
    }

    for (const QLowEnergyCharacteristic &ch : characterList) {
        if(ch.isValid()){
            if(ch.properties() & QLowEnergyCharacteristic::Notify){
                QLowEnergyDescriptor notice = ch.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if(notice.isValid()){
                    service->writeDescriptor(notice,QByteArray::fromHex("0100"));
                }
            }

            if(ch.properties() & QLowEnergyCharacteristic::WriteNoResponse ||  ch.properties() & QLowEnergyCharacteristic::Write){
                QByteArray arr;
                arr.resize(20);
                for(int i=0;i<20;i++){
                    arr[i] = static_cast<char>(i);
                }
                service->writeCharacteristic(ch,arr);
            }
            if(ch.properties() & QLowEnergyCharacteristic::Read){
                service->readCharacteristic(ch);
            }
        }
    }
}
