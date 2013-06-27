////////////////////////////////////////////////////////////////////////////////
// Mac OS X implementation of UsbDeviceMonitor

#include "usbdevicemonitor.h"
#include "usbdevicemonitor_mac_p.h"

#include <Cocoa/Cocoa.h>
#include <IOKit/IOBSD.h>


// Private class implementation

UsbDeviceMonitorPrivate::UsbDeviceMonitorPrivate()
{
}

UsbDeviceMonitorPrivate::~UsbDeviceMonitorPrivate()
{
}


// Supplemental callback functions

void UsbDeviceAddedCallback(void *refCon, io_iterator_t iterator)
{
    while (IOIteratorNext(iterator)) {}; // Run out the iterator or notifications won't start
    UsbDeviceMonitor* monitor = (UsbDeviceMonitor*)refCon;
    emit monitor->deviceChanged();
}

void UsbDeviceRemovedCallback(void *refCon, io_iterator_t iterator)
{
    while (IOIteratorNext(iterator)) {}; // Run out the iterator or notifications won't start
    UsbDeviceMonitor* monitor = (UsbDeviceMonitor*)refCon;
    emit monitor->deviceChanged();
}


// Main class implementation

UsbDeviceMonitor::UsbDeviceMonitor(QObject *parent) :
    QObject(parent),
    d_ptr(new UsbDeviceMonitorPrivate())
{
}

UsbDeviceMonitor::~UsbDeviceMonitor()
{
    cleanup();
    delete d_ptr;
}

// Closes handles and frees resources
void UsbDeviceMonitor::cleanup()
{
}

// Implements QAbstractNativeEventFilter interface for processing WM_DEVICECHANGE messages (Windows)
bool UsbDeviceMonitor::nativeEventFilter(const QByteArray& eventType, void* message, long* result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
    return false;
}

bool UsbDeviceMonitor::startMonitoring()
{
    IONotificationPortRef notificationPort = IONotificationPortCreate(kIOMasterPortDefault);
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                   IONotificationPortGetRunLoopSource(notificationPort),
                   kCFRunLoopDefaultMode);

    // If we monitor USB devices, notification comes too early, and the new disk device is not present yet.
    // So, instead we monitor for the new disks.
    CFMutableDictionaryRef matchingDict = IOServiceMatching("IOMediaBSDClient");
    CFRetain(matchingDict); // Need to use it twice and IOServiceAddMatchingNotification() consumes a reference

    io_iterator_t portIterator = 0;
    // Register for notifications when a serial port is added to the system
    kern_return_t result = IOServiceAddMatchingNotification(notificationPort,
                                                            kIOPublishNotification,
                                                            matchingDict,
                                                            UsbDeviceAddedCallback,
                                                            this,
                                                            &portIterator);
    while (IOIteratorNext(portIterator)) {}; // Run out the iterator or notifications won't start (you can also use it to iterate the available devices).

    // Also register for removal notifications
    IONotificationPortRef terminationNotificationPort = IONotificationPortCreate(kIOMasterPortDefault);
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                       IONotificationPortGetRunLoopSource(terminationNotificationPort),
                       kCFRunLoopDefaultMode);
    result = IOServiceAddMatchingNotification(terminationNotificationPort,
                                              kIOTerminatedNotification,
                                              matchingDict,
                                              UsbDeviceRemovedCallback,
                                              this,         // refCon/contextInfo
                                              &portIterator);

    while (IOIteratorNext(portIterator)) {}; // Run out the iterator or notifications won't start (you can also use it to iterate the available devices).

    return true;
}
