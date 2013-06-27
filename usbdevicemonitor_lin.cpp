////////////////////////////////////////////////////////////////////////////////
// Linux implementation of UsbDeviceMonitor

#include "usbdevicemonitor.h"
#include "usbdevicemonitor_lin_p.h"


// Private class implementation

UsbDeviceMonitorPrivate::UsbDeviceMonitorPrivate(QObject *parent) :
    QObject(parent)
{
}

UsbDeviceMonitorPrivate::~UsbDeviceMonitorPrivate()
{
}

// Processes udev socket notification
void UsbDeviceMonitorPrivate::processUdevNotification(int socket)
{
    Q_UNUSED(socket);

    // Read the device information
    // We don't really need it, but we have to empty the queue
    struct udev_device* dev = udev_monitor_receive_device(m_udevMonitor);
    if (dev)
    {
        udev_device_unref(dev);
        emit q_ptr->deviceChanged();
    }
}


// Main class implementation

UsbDeviceMonitor::UsbDeviceMonitor(QObject *parent) :
    QObject(parent),
    d_ptr(new UsbDeviceMonitorPrivate())
{
    d_ptr->q_ptr = this;
    d_ptr->m_udev = NULL;
    d_ptr->m_udevMonitor = NULL;
    d_ptr->m_udevNotifier = NULL;
}

UsbDeviceMonitor::~UsbDeviceMonitor()
{
    cleanup();
    delete d_ptr;
}

// Closes handles and frees resources
void UsbDeviceMonitor::cleanup()
{
    if (d_ptr->m_udevMonitor != NULL)
    {
        udev_monitor_unref(d_ptr->m_udevMonitor);
        d_ptr->m_udevMonitor = NULL;
    }
    if (d_ptr->m_udev != NULL)
    {
        udev_unref(d_ptr->m_udev);
        d_ptr->m_udev = NULL;
    }
    if (d_ptr->m_udevNotifier)
    {
        d_ptr->m_udevNotifier->setEnabled(false);
        delete d_ptr->m_udevNotifier;
        d_ptr->m_udevNotifier = NULL;
    }
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
    // In Linux we use udev monitor
    try
    {
        d_ptr->m_udev = udev_new();
        if (d_ptr->m_udev == NULL)
            throw 1;

        // Initialize monitoring
        d_ptr->m_udevMonitor = udev_monitor_new_from_netlink(d_ptr->m_udev, "udev");
        if (d_ptr->m_udevMonitor == NULL)
            throw 1;
        // Set filter to get notified only about block devices of type "disk"
        if (udev_monitor_filter_add_match_subsystem_devtype(d_ptr->m_udevMonitor, "block", "disk") < 0)
            throw 1;
        // Start monitoring
        if (udev_monitor_enable_receiving(d_ptr->m_udevMonitor) < 0)
            throw 1;
        // Get the socket file descriptor for QSocketNotifier
        int fd = udev_monitor_get_fd(d_ptr->m_udevMonitor);

        // Initialize QSocketNotifier for watching the socket
        d_ptr->m_udevNotifier = new QSocketNotifier(fd, QSocketNotifier::Read);
        connect(d_ptr->m_udevNotifier, &QSocketNotifier::activated, d_ptr, &UsbDeviceMonitorPrivate::processUdevNotification);
        d_ptr->m_udevNotifier->setEnabled(true);
    }
    catch (...)
    {
        // Something went wrong, destroy everything and do without monitoring udev
        cleanup();
        return false;
    }

    return true;
}
