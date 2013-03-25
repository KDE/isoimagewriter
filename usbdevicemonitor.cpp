////////////////////////////////////////////////////////////////////////////////
// Implementation of UsbDeviceMonitor

#include <QApplication>

#include "usbdevicemonitor.h"

UsbDeviceMonitor::UsbDeviceMonitor(QObject *parent) :
    QObject(parent)
{
#if defined(Q_OS_LINUX)
    m_udev = NULL;
    m_udevMonitor = NULL;
    m_udevNotifier = NULL;
#endif
}

UsbDeviceMonitor::~UsbDeviceMonitor()
{
    cleanup();
}

// Closes handles and frees resources
void UsbDeviceMonitor::cleanup()
{
#if defined(Q_OS_LINUX)
    if (m_udevMonitor != NULL)
    {
        udev_monitor_unref(m_udevMonitor);
        m_udevMonitor = NULL;
    }
    if (m_udev != NULL)
    {
        udev_unref(m_udev);
        m_udev = NULL;
    }
    if (m_udevNotifier)
    {
        m_udevNotifier->setEnabled(false);
        delete m_udevNotifier;
        m_udevNotifier = NULL;
    }
#endif
}

// Implements QAbstractNativeEventFilter interface for processing WM_DEVICECHANGE messages (Windows)
bool UsbDeviceMonitor::nativeEventFilter(const QByteArray& eventType, void* message, long* result)
{
    Q_UNUSED(eventType);
#if defined(Q_OS_WIN32)
    MSG* msg = static_cast<MSG*>(message);
    if ((msg->message == WM_DEVICECHANGE) &&
        ((msg->wParam == DBT_DEVICEARRIVAL) || (msg->wParam == DBT_DEVICEREMOVECOMPLETE)))
    {
        // If the event was caused by adding or remiving a device, mark the WinAPI message as processed
        // and emit the notification signal
        *result = TRUE;
        emit deviceChanged();
        return true;
    }
#else
    Q_UNUSED(message);
    Q_UNUSED(result);
#endif
    return false;
}

#if defined(Q_OS_LINUX)
// Processes udev socket notification
void UsbDeviceMonitor::processUdevNotification(int socket)
{
    Q_UNUSED(socket);
    // Read the device information
    // We don't really need it, but we have to empty the queue
    struct udev_device* dev = udev_monitor_receive_device(m_udevMonitor);
    if (dev)
    {
        udev_device_unref(dev);
        emit deviceChanged();
    }
}
#endif

bool UsbDeviceMonitor::startMonitoring()
{
#if defined(Q_OS_WIN32)
    // In Windows we use QAbstractNativeEventFilter interface implementation and process native Windows messages
    qApp->installNativeEventFilter(this);
#elif defined(Q_OS_LINUX)
    // In Linux we use udev monitor
    try
    {
        m_udev = udev_new();
        if (m_udev == NULL)
            throw 1;

        // Initialize monitoring
        m_udevMonitor = udev_monitor_new_from_netlink(m_udev, "udev");
        if (m_udevMonitor == NULL)
            throw 1;
        // Set filter to get notified only about block devices of type "disk"
        if (udev_monitor_filter_add_match_subsystem_devtype(m_udevMonitor, "block", "disk") < 0)
            throw 1;
        // Start monitoring
        if (udev_monitor_enable_receiving(m_udevMonitor) < 0)
            throw 1;
        // Get the socket file descriptor for QSocketNotifier
        int fd = udev_monitor_get_fd(m_udevMonitor);

        // Initialize QSocketNotifier for watching the socket
        m_udevNotifier = new QSocketNotifier(fd, QSocketNotifier::Read);
        connect(m_udevNotifier, &QSocketNotifier::activated, this, &UsbDeviceMonitor::processUdevNotification);
        m_udevNotifier->setEnabled(true);
    }
    catch (...)
    {
        // Something went wrong, destroy everything and do without monitoring udev
        cleanup();
        return false;
    }
#endif

    return true;
}
