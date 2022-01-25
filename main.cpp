#include "mainwindow.h"
#include <canframeid.h>
#include <QApplication>
#include <QString>
#include <QCanBus>
#include <QCanBusDevice>
#include <QtDebug>

QCanBusDevice *device;
QObject *object;

QCanBusDevice::Filter setCanFilter(const unsigned short&id){
    QCanBusDevice::Filter filter;

    filter.frameId = id;
    filter.frameIdMask = 0x7FFu;
    filter.format = QCanBusDevice::Filter::MatchBaseFormat;
    filter.type = QCanBusFrame::DataFrame;

    return filter;
}

int decodeFrame(const QCanBusFrame &frame){
    int value;
    const QByteArray payload = frame.payload();

    if(frame.isValid()){
        switch(frame.frameId()){
            case VECHILE_SPEED:
                unsigned short b3, b4;
                b3 = payload[3];
                b4 = payload[4];
                qDebug() << b3 << b4;
                value = b3 + b4;
                break;
            default:
                break;
        }
        return  value;
    }
    return 1;
}

//dont think we need this
void checkFrames(){
    while(device->framesAvailable() > 0){
        QCanBusFrame frame = device->readFrame();

        if(frame.isValid()){
            switch(frame.frameId()){
                case VECHILE_SPEED:
                    object->setProperty("speed", decodeFrame(frame));
                    break;
                default:
                    break;
            }
        }
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    if(QCanBus::instance()->plugins().contains(QStringLiteral("socketcan"))){

        //create CAN bus device and connect to can0 with SocketCAN
        device = QCanBus::instance()->createDevice("socketcan", "vcan0");
        device->connectDevice();

        //filtr CAN Bus Device
        QList<QCanBusDevice::Filter> filterList;
        filterList.append(setCanFilter(VECHILE_SPEED));

        bool fdkey = true;
        device->setConfigurationParameter(QCanBusDevice::RawFilterKey, QVariant::fromValue(filterList));
        device->setConfigurationParameter(QCanBusDevice::CanFdKey, fdkey);

        //dont think we need
        //connect framesrecieved signal to slot function for reading frames
        QObject::connect(device, &QCanBusDevice::framesReceived, checkFrames);
    }
    return a.exec();
}
