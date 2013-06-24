////////////////////////////////////////////////////////////////////////////////
// This file contains Mac implementation of platform-dependent functions

#include "common.h"
#include "usbdevice.h"

#include <QApplication>

#include <Cocoa/Cocoa.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/IOBSD.h>
#include <Authorization.h>
#include <ServiceManagement.h>

bool readBooleanRegKey(io_service_t device, CFStringRef key)
{
    CFTypeRef value = IORegistryEntrySearchCFProperty(
        device,
        kIOServicePlane,
        key,
        kCFAllocatorDefault,
        kIORegistryIterateRecursively
    );
    bool res = false;
    if (value != nil)
    {
        if (CFGetTypeID(value) == CFBooleanGetTypeID())
            res = (CFBooleanGetValue((CFBooleanRef)value) ? true : false);
        CFRelease(value);
    }
    return res;
}

unsigned long long readIntegerRegKey(io_service_t device, CFStringRef key)
{
    CFTypeRef value = IORegistryEntrySearchCFProperty(
        device,
        kIOServicePlane,
        key,
        kCFAllocatorDefault,
        kIORegistryIterateRecursively
    );
    unsigned long long res = 0;
    if (value != nil)
    {
        CFNumberGetValue((CFNumberRef)value, kCFNumberLongLongType, &res);
        CFRelease(value);
    }
    return res;
}


CFStringRef readStringRegKey(io_service_t device, CFStringRef key)
{
    CFTypeRef value = IORegistryEntrySearchCFProperty(
        device,
        kIOServicePlane,
        key,
        kCFAllocatorDefault,
        kIORegistryIterateRecursively
    );
    CFStringRef res = nil;
    if (value != nil)
    {
        if (CFGetTypeID(value) == CFStringGetTypeID())
            res = (CFStringRef)value;
        else
            CFRelease(value);
    }
    return res;
}


bool platformEnumFlashDevices(AddFlashDeviceCallbackProc callback, void* cbParam)
{
    CFMutableDictionaryRef matchingDict;
    io_iterator_t iter;
    kern_return_t kr;
    io_service_t device;

    // Set up a matching dictionary for the class
    matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    if (matchingDict == NULL)
    {
        return false;
    }

    // Obtain iterator
    kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iter);
    if (kr != KERN_SUCCESS)
    {
        return false;
    }

    CFStringEncoding encodingMethod = CFStringGetSystemEncoding();

    // Enumerate the devices
    while ((device = IOIteratorNext(iter)))
    {
        // Skip all non-removable devices
        if (!readBooleanRegKey(device, CFSTR(kIOMediaRemovableKey)))
        {
            IOObjectRelease(device);
            continue;
        }

        // Skip devices without BSD names (that is, not real disks)
        CFStringRef tempStr = readStringRegKey(device, CFSTR(kIOBSDNameKey));
        if (tempStr == nil)
        {
            IOObjectRelease(device);
            continue;
        }

        // Fetch the required properties and store them in the UsbDevice object
        UsbDevice* deviceData = new UsbDevice;

        // Physical device name (use it also as the volumes list - the real list may be too long)
        // Using "rdiskN" instead of BSD name "diskN" to work around an OS X bug when writing
        // to "diskN" is extremely slow
        deviceData->m_PhysicalDevice = "/dev/r";
        deviceData->m_PhysicalDevice += CFStringGetCStringPtr(tempStr, encodingMethod);
        CFRelease(tempStr);
        deviceData->m_Volumes << deviceData->m_PhysicalDevice;

        // User-friendly device name: vendor+product
        tempStr = readStringRegKey(device, CFSTR(kUSBVendorString));
        if (tempStr != nil)
        {
            deviceData->m_VisibleName = CFStringGetCStringPtr(tempStr, encodingMethod);
            deviceData->m_VisibleName = deviceData->m_VisibleName.trimmed();
            CFRelease(tempStr);
        }
        tempStr = readStringRegKey(device, CFSTR(kUSBProductString));
        if (tempStr != nil)
        {
            deviceData->m_VisibleName += " ";
            deviceData->m_VisibleName += CFStringGetCStringPtr(tempStr, encodingMethod);
            deviceData->m_VisibleName = deviceData->m_VisibleName.trimmed();
            CFRelease(tempStr);
        }

        // Size of the flash disk
        deviceData->m_Size = readIntegerRegKey(device, CFSTR(kIOMediaSizeKey));
        deviceData->m_SectorSize = readIntegerRegKey(device, CFSTR(kIOMediaPreferredBlockSizeKey));

        // The device information is now complete, append the entry
        callback(cbParam, deviceData);

        // Free the resources
        IOObjectRelease(device);
    }

    IOObjectRelease(iter);
    return true;
}

bool ensureElevated()
{
    uid_t uid = getuid();
    uid_t euid = geteuid();
    if ((uid == 0) || (euid == 0))
        return true;

    AuthorizationItem authItem = { kSMRightModifySystemDaemons, 0, NULL, 0 };
    AuthorizationRights authRights = { 1, &authItem };
    AuthorizationFlags flags = kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed | kAuthorizationFlagPreAuthorize | kAuthorizationFlagExtendRights;

    AuthorizationRef authRef = NULL;

    if (AuthorizationCreate(&authRights, kAuthorizationEmptyEnvironment, flags, &authRef) != errAuthorizationSuccess)
        return false;

    QByteArray appPath = QApplication::arguments()[0].toUtf8();
    QByteArray arg = ("--lang=" + getLocale()).toUtf8();
    char* const args[] = { arg.data(), NULL };
    if (AuthorizationExecuteWithPrivileges(authRef, appPath.constData(), kAuthorizationFlagDefaults, args, NULL) != errAuthorizationSuccess)
        return false;

    exit(0);
}
