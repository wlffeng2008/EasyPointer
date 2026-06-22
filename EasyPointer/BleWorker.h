#ifndef BLEWORKER_H
#define BLEWORKER_H
#include <QObject>

#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>


class BleWorker: public QObject
{
    Q_OBJECT
public:
    explicit BleWorker(QObject *parent = nullptr);
    void scanBleDevices(const QString &strName);
    void connectBle(const QString&strMac);

    QLowEnergyController *m_control = nullptr;

signals:
    void onBleDevice(const QString &strMac, const QString &strName);
    void onBleDataIn(const QByteArray&data,int nType);

private:
    bool m_bExit=false;

    void connectService(const QBluetoothUuid&Uuid);
    QLowEnergyService *m_service0 = nullptr;
    QLowEnergyCharacteristic m_write0;
    QLowEnergyCharacteristic m_read0;

    QLowEnergyService *m_service1 = nullptr;
    QLowEnergyCharacteristic m_write1;
    QLowEnergyCharacteristic m_read1;


    QLowEnergyService *m_service2 = nullptr;
    QLowEnergyCharacteristic m_write2;
    QLowEnergyCharacteristic m_read2;

    QLowEnergyCharacteristic m_character;
    QLowEnergyDescriptor m_descriptor;
};

#endif // BLEWORKER_H
