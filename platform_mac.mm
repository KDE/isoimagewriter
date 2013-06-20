////////////////////////////////////////////////////////////////////////////////
// This file contains Mac implementation of platform-dependent functions

#include <Cocoa/Cocoa.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/IOBSD.h>
#include <Authorization.h>
#include <ServiceManagement.h>

#include "common.h"

bool restartElevated(const char* path)
{
    AuthorizationItem authItem = { kSMRightModifySystemDaemons, 0, NULL, 0 };
    AuthorizationRights authRights = { 1, &authItem };
    AuthorizationFlags flags = kAuthorizationFlagDefaults | kAuthorizationFlagInteractionAllowed | kAuthorizationFlagPreAuthorize | kAuthorizationFlagExtendRights;

    AuthorizationRef authRef = NULL;

    OSStatus status = AuthorizationCreate(&authRights, kAuthorizationEmptyEnvironment, flags, &authRef);

    status = AuthorizationExecuteWithPrivileges(authRef, path, kAuthorizationFlagDefaults, NULL, NULL);

    return true;
}

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
        CFStringRef bsdName = readStringRegKey(device, CFSTR(kIOBSDNameKey));
        if (bsdName == nil)
        {
            IOObjectRelease(device);
            continue;
        }
        char bsdNameStr[128] = "/dev/";
        strcat(bsdNameStr, CFStringGetCStringPtr(bsdName, encodingMethod));

        // Get the rest of parameters and add the device into the list
        CFStringRef vendor = readStringRegKey(device, CFSTR(kUSBVendorString));
        const char* vendorStr = "";
        if (vendor != nil)
            vendorStr = CFStringGetCStringPtr(vendor, encodingMethod);
        CFStringRef product = readStringRegKey(device, CFSTR(kUSBProductString));
        const char* productStr = "";
        if (product != nil)
            productStr = CFStringGetCStringPtr(product, encodingMethod);
        unsigned long long size = readIntegerRegKey(device, CFSTR(kIOMediaSizeKey));
        unsigned long long sectorSize = readIntegerRegKey(device, CFSTR(kIOMediaPreferredBlockSizeKey));
        const char* volumes[1] = { bsdNameStr };
        callback(
            cbParam,
            vendorStr,
            productStr,
            bsdNameStr,
            volumes,
            1,
            size,
            sectorSize
        );

        // Free the resources
        CFRelease(bsdName);
        if (vendor != nil)
            CFRelease(vendor);
        if (product != nil)
            CFRelease(product);
        IOObjectRelease(device);
    }

    IOObjectRelease(iter);
    return true;
}
