#ifndef USBDEVICEMONITOR_H
#define USBDEVICEMONITOR_H

////////////////////////////////////////////////////////////////////////////////
// Class implementing monitoring for inserting/removing USB devices


#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QSocketNotifier>

#include "common.h"

class UsbDeviceMonitor : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit UsbDeviceMonitor(QObject *parent = 0);
    ~UsbDeviceMonitor();
    
    // Implements QAbstractNativeEventFilter interface for processing WM_DEVICECHANGE messages (Windows)
    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result);

protected:
#if defined(Q_OS_LINUX)
    // udev library context
    struct udev* m_udev;
    // udev device monitor handle
    struct udev_monitor* m_udevMonitor;
    // Watcher for udev monitor socket
    QSocketNotifier* m_udevNotifier;
#endif
    // Closes handles and frees resources
    void cleanup();

signals:
    // Emitted when device change notification arrives
    void deviceChanged();

public slots:
#if defined(Q_OS_LINUX)
    // Processes udev socket notification
    void processUdevNotification(int socket);
#endif
    // Initializes monitoring for USB devices
    bool startMonitoring();
};

#endif // USBDEVICEMONITOR_H
