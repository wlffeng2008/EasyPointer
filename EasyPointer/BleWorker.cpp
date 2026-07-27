#include "BleWorker.h"
#include <QtGlobal>

bool isBluetoothAvailable()
{
    QBluetoothLocalDevice localDevice;
    QBluetoothAddress address = localDevice.address();

    // 如果地址为空，则蓝牙不可用
    if (address.isNull()) {
        qDebug() << "蓝牙适配器未找到";
        return false;
    }

    // 检查蓝牙是否已开启
    if (localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        qDebug() << "蓝牙已关闭";
        // localDevice.powerOn();
    }

    return true;
}


BleWorker::BleWorker(QObject *parent):QObject(parent)
{

}

void BleWorker::scanBleDevices(const QString &strDevName)
{
    bool bFindBle = isBluetoothAvailable();
    if(bFindBle)
    {
        QBluetoothLocalDevice*localDevice = new QBluetoothLocalDevice(this) ;
        QBluetoothDeviceDiscoveryAgent *discoveryAgent = nullptr;
        if(discoveryAgent)
            delete discoveryAgent;
        discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this,[=](const QBluetoothDeviceInfo&device){
                    //qDebug()<< device.deviceUuid() << device.name() << device.coreConfigurations();
                    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration && device.isValid() )
                    {
                        if(localDevice->pairingStatus(device.address()) == QBluetoothLocalDevice::Unpaired)
                            return ;
                    }
                    QString strName = device.name();
                    if(strName.contains(strDevName) || strDevName.isEmpty())
                    {
                        QString strMac = device.address().toString();
                        emit onBleDevice(strMac,strName);

                        QString itemText = strMac + " -- " +strName;
                        qDebug().noquote() << itemText  << device.deviceUuid();

                        if(m_control)
                        {
                            m_control->discoverServices();
                            m_control->disconnectFromDevice();
                            delete m_control;
                        }

                        m_control = QLowEnergyController::createCentral(device,this); // 创建蓝牙控制器对象
                        if (m_control)
                        {
                            qDebug() << "创建蓝牙控制器成功！" ;

                            connect(m_control, &QLowEnergyController::connected, this, [=]{
                                qDebug() << "蓝牙控制器连接成功，开始搜索蓝牙服务！";
                                m_control->discoverServices();
                            });

                            connect(m_control, &QLowEnergyController::disconnected, this, [=](){
                                qDebug() << "蓝牙设备已断开！";
                            });

                            connect(m_control, &QLowEnergyController::serviceDiscovered, this, [=](const QBluetoothUuid &newService){
                                qDebug() <<"发现一个蓝牙服务-uuid: "<< newService << Qt::hex << newService.data1 ;
                                connectService(newService);
                            });

                            //connect(m_control, SIGNAL(discoveryFinished()), this, SLOT(onServerDiscoveryFinished()));

                            connect(m_control, &QLowEnergyController::errorOccurred, this, [=](QLowEnergyController::Error error){  qDebug() << error; });

                            //connectBle("");
                            m_control->connectToDevice();
                        }
                        else
                        {
                            qDebug() << "创建蓝牙控制器失败！" ;
                        }
                    }
                });

        connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
                this,[=](QBluetoothDeviceDiscoveryAgent::Error error){
                    qDebug()<< "onDeviceDiscoverError: " << error;
                });

        discoveryAgent->stop() ;
        discoveryAgent->setLowEnergyDiscoveryTimeout(25000);
        discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    }

}

void BleWorker::connectService(const QBluetoothUuid&Uuid)
{
    if(Uuid.data1 == 0x6e400001)
    {
        if(m_service0) delete m_service0 ;
        m_service0 = m_control->createServiceObject(Uuid,this); // 创建服务
        if (!m_service0)
        {
            qDebug() << "创建蓝牙服务失败！" ;
            return ;
        }

        qDebug() << "创建蓝牙服务成功0 " ;

        // 监听服务状态变化
        connect(m_service0,&QLowEnergyService::stateChanged ,this,[=](QLowEnergyService::ServiceState){

            QList<QLowEnergyCharacteristic> charList = m_service0->characteristics();
            for(const QLowEnergyCharacteristic &bleChar: std::as_const(charList))
            {
                int nCharVal = bleChar.uuid().toUInt16();
                const QLowEnergyCharacteristic::PropertyTypes props = bleChar.properties();
                if(!bleChar.isValid())
                    continue ;

                qDebug().noquote() << "Characteristic: "<< bleChar.uuid() << bleChar.name() << QString::number(nCharVal,16).toUpper() << props;

                if(props&QLowEnergyCharacteristic::Notify )
                {
                    QLowEnergyDescriptor descriptor = bleChar.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                    //qDebug().noquote()<<"descriptor isValid:" << descriptor.isValid() ;
                    m_service0->writeDescriptor(descriptor,QByteArray::fromHex("0100"));
                }

                if(props&QLowEnergyCharacteristic::WriteNoResponse || props&QLowEnergyCharacteristic::Write)
                {
                    m_write0 = bleChar;
                }
            }

        });

        // 监听服务的characteristic变化，有数据传来
        connect(m_service0,&QLowEnergyService::characteristicChanged, this,[=](const QLowEnergyCharacteristic &info, const QByteArray &value){
            qDebug() << value.toHex(' ').toUpper();
        });

        connect(m_service0, &QLowEnergyService::descriptorWritten, this, [=](QLowEnergyDescriptor,const QByteArray&value){
            qDebug().noquote() <<"描述符写入成功!"  << value.toHex();  });

        connect(m_service0, &QLowEnergyService::errorOccurred, this, [=](QLowEnergyService::ServiceError error){
            qDebug().noquote() << error;  });

        connect(m_service0, &QLowEnergyService::characteristicRead,this, [=](const QLowEnergyCharacteristic &info, const QByteArray &value){
            qDebug().noquote() << "Read1 <=:" << value.toHex(' ').toUpper() << value.data();  });

        m_service0->discoverDetails();
    }

    if(Uuid.data1 == 0x6e400004)
    {
        if(m_service1) delete m_service1;
        m_service1 = m_control->createServiceObject(Uuid,this); // 创建服务
        if (!m_service1)
        {
            qDebug() << "创建蓝牙服务1失败！" ;
            return ;
        }

        qDebug() << "创建蓝牙服务成功1 " ;

        // 监听服务状态变化
        connect(m_service1,&QLowEnergyService::stateChanged ,this,[=](QLowEnergyService::ServiceState){

            QList<QLowEnergyCharacteristic> charList = m_service1->characteristics();
            for(const QLowEnergyCharacteristic &bleChar: std::as_const(charList))
            {
                int nCharVal = bleChar.uuid().toUInt16();
                const QLowEnergyCharacteristic::PropertyTypes props = bleChar.properties();
                if(!bleChar.isValid())
                    continue ;

                qDebug().noquote() << "Characteristic: "<< bleChar.uuid() << bleChar.name() << QString::number(nCharVal,16).toUpper() << props;

                if(props&QLowEnergyCharacteristic::Notify )
                {
                    QLowEnergyDescriptor descriptor = bleChar.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                    //qDebug().noquote()<<"descriptor isValid:" << descriptor.isValid() ;
                    //m_service1->writeDescriptor(descriptor,QByteArray::fromHex("0100"));
                }

                if(props&QLowEnergyCharacteristic::WriteNoResponse || props&QLowEnergyCharacteristic::Write)
                {
                    m_write1 = bleChar;
                }
            }

        });

        // 监听服务的characteristic变化，有数据传来
        connect(m_service1,&QLowEnergyService::characteristicChanged, this,[=](const QLowEnergyCharacteristic &info, const QByteArray &value){
            qDebug() << value.toHex(' ').toUpper();
        });

        connect(m_service1, &QLowEnergyService::descriptorWritten,this, [=](QLowEnergyDescriptor,const QByteArray&value){
            qDebug().noquote() <<"描述符写入成功!"   << value.toHex();  });

        connect(m_service1, &QLowEnergyService::errorOccurred, this, [=](QLowEnergyService::ServiceError error){
            qDebug().noquote() << error;  });

        connect(m_service1, &QLowEnergyService::characteristicRead,this, [=](const QLowEnergyCharacteristic &info, const QByteArray &value){
            qDebug().noquote() << "Read3 <=:" << value.toHex(' ').toUpper() << value.data();  });

        m_service1->discoverDetails();
    }


    if(Uuid.data1 == 0x6e40000a)
    {
        if(m_service2) delete m_service2;
        m_service2 = m_control->createServiceObject(Uuid,this); // 创建服务
        if (!m_service2)
        {
            qDebug() << "创建蓝牙服务2失败！" ;
            return ;
        }

        qDebug() << "创建蓝牙服务成功2 " ;

        // 监听服务状态变化
        connect(m_service2,&QLowEnergyService::stateChanged ,this,[=](QLowEnergyService::ServiceState){
            QList<QLowEnergyCharacteristic> charList = m_service1->characteristics();
            for(const QLowEnergyCharacteristic &bleChar: std::as_const(charList))
            {
                int nCharVal = bleChar.uuid().toUInt16();
                const QLowEnergyCharacteristic::PropertyTypes props = bleChar.properties();
                if(!bleChar.isValid())
                    continue ;

                qDebug().noquote() << "Characteristic: "<< bleChar.uuid() << bleChar.name() << QString::number(nCharVal,16).toUpper() << props;

                if(props&QLowEnergyCharacteristic::Notify )
                {
                    QLowEnergyDescriptor descriptor = bleChar.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                    //qDebug().noquote()<<"descriptor isValid:" << descriptor.isValid() ;
                    //m_service2->writeDescriptor(descriptor,QByteArray::fromHex("0100"));
                }

                if(props&QLowEnergyCharacteristic::WriteNoResponse || props&QLowEnergyCharacteristic::Write)
                {
                    m_write2 = bleChar;
                }
            }
        });

        // 监听服务的characteristic变化，有数据传来
        connect(m_service2,&QLowEnergyService::characteristicChanged, this, [=](const QLowEnergyCharacteristic &info, const QByteArray &value){
            qDebug() << value.toHex(' ').toUpper();
        });

        connect(m_service2, &QLowEnergyService::descriptorWritten,[=](QLowEnergyDescriptor,const QByteArray&value){
            qDebug().noquote() <<"描述符写入成功!"   << value.toHex();  });

        connect(m_service2, &QLowEnergyService::errorOccurred, this, [=](QLowEnergyService::ServiceError error){
            qDebug().noquote() << error;  });

        connect(m_service2, &QLowEnergyService::characteristicRead,this, [=](const QLowEnergyCharacteristic &info, const QByteArray &value){
            qDebug().noquote() << "Read2 <=:" << value.toHex(' ').toUpper() << value.data();  });

        m_service2->discoverDetails();
    }
}


/*

实现通讯，主要依靠三个服务完成。
服务1：
发送实时包数据，接收手机端发送的指令。
服务2：
接收体检包数据
服务3：
若体检包数据有丢包，则通过此数据找回丢失的数据包
1.5蓝牙串口通讯服务：

服务UUID如下所示
服务1：(用于蓝牙与app通讯)
功能：实时包的发送、读取手机端发送的指令
服务uuid：
            uuid0：{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E}
特性uuid：
从设备端读：uuid1：{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E}
写入设备端：uuid2：{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E}

服务2：(用于上传云端)
功能：接收体检包数据
服务uuid：
            uuid3：{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x04, 0x00, 0x40, 0x6E}
特性uuid：
从设备端读：uuid4：{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x06, 0x00, 0x40, 0x6E}
写入设备端：uuid5：{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x05, 0x00, 0x40, 0x6E}

服务3：(用于上传云端)
功能：若体检包数据有丢包，则通过此数据找回丢失的数据包
服务uuid：
            uuid6：{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x0a, 0x00, 0x40, 0x6E}
特性uuid：
从设备端读：uuid7：{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x0c, 0x00, 0x40, 0x6E}
写入设备端：uuid8：{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x0b, 0x00, 0x40, 0x6E}


一、	采集控制
开启采集  向uuid2写入0x01
停止采集  向uuid2写入0x02

static quint8 uuid0[]={0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E};
*/


QUuid raw16BytesToQUuid(const uint8_t* raw)
{
    // 前4字节小端转uint32
    uint32_t l = (raw[3] << 24) | (raw[2] << 16) | (raw[1] << 8) | raw[0];
    // 第4、5字节小端ushort
    uint16_t w1 = (raw[5] << 8) | raw[4];
    // 第6、7字节小端ushort
    uint16_t w2 = (raw[7] << 8) | raw[6];

    return QUuid(
        l, w1, w2,
        raw[8], raw[9], raw[10], raw[11],
        raw[12], raw[13], raw[14], raw[15]
        );
}


void BleWorker::connectBle(const QString&strMac)
{
    QBluetoothUuid uuid0("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
    QBluetoothUuid uuid1("6e400003-b5a3-f393-e0a9-e50e24dcca9e");
    QBluetoothUuid uuid2("6e400002-b5a3-f393-e0a9-e50e24dcca9e");

    m_service0 = m_control->createServiceObject(uuid0,this); // 创建服务
    if (!m_service0)
    {
        qDebug() << "创建蓝牙服务失败！" ;
        return ;
    }

    qDebug() << "创建蓝牙服务成功！ " ;
}

void BleWorker::ReadRealData(bool read)
{
    quint8 data  = (read ? 0x01 : 0x02) ;
    m_service0->writeCharacteristic(m_write0,QByteArray(1,data));
    qDebug() << "ReadRealData" << data ;
}

void BleWorker::ReadHWInfo()
{
    m_service0->writeCharacteristic(m_write0,QByteArray(0x03,1));
    qDebug() << "ReadHWInfo" ;
}