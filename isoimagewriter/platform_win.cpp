/*
 * Copyright 2016 ROSA
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

////////////////////////////////////////////////////////////////////////////////
// This file contains Windows implementation of platform-dependent functions

#include "common.h"
#include "usbdevice.h"

#include <QApplication>
#include <QMessageBox>


// Several WinAPI COM specific macros for keeping the code clean

// Runs the COM request specified, checks for return value and throws an exception
// with descriptive error message if it's not OK
#define CHECK_OK(code, msg)                             \
    {                                                   \
        HRESULT res = code;                             \
        if (res != S_OK)                                \
        {                                               \
            throw formatErrorMessageFromCode(msg, res); \
        }                                               \
    }

// Releases the COM object and nullifies the pointer
#define SAFE_RELEASE(obj)   \
    {                       \
        if (obj != NULL)    \
        {                   \
            obj->Release(); \
            obj = NULL;     \
        }                   \
    }

// Allocated a BSTR string using the specified text, checks for successful memory allocation
// and throws an exception with descriptive error message if unsuccessful
#define ALLOC_BSTR(name, str)                                                 \
    {                                                                         \
        name = SysAllocString(str);                                           \
        if (name == NULL)                                                     \
        {                                                                     \
            throw i18n("Memory allocation for %1 failed.", #name); \
        }                                                                     \
    }

// Releases the BSTR string and nullifies the pointer
#define FREE_BSTR(str)      \
    {                       \
        SysFreeString(str); \
        str = NULL;         \
    }


bool platformEnumFlashDevices(AddFlashDeviceCallbackProc callback, void* cbParam)
{
    // Using WMI for enumerating the USB devices

    // Namespace of the WMI classes
    BSTR strNamespace       = NULL;
    // "WQL" - the query language we're gonna use (the only possible, actually)
    BSTR strQL              = NULL;
    // Query string for requesting physical devices
    BSTR strQueryDisks      = NULL;
    // Query string for requesting partitions for each of the physical devices
    BSTR strQueryPartitions = NULL;
    // Query string for requesting logical disks for each of the partitions
    BSTR strQueryLetters    = NULL;

    // Various COM objects for executing the queries, enumerating lists and retrieving properties
    IWbemLocator*         pIWbemLocator         = NULL;
    IWbemServices*        pWbemServices         = NULL;
    IEnumWbemClassObject* pEnumDisksObject      = NULL;
    IEnumWbemClassObject* pEnumPartitionsObject = NULL;
    IEnumWbemClassObject* pEnumLettersObject    = NULL;
    IWbemClassObject*     pDiskObject           = NULL;
    IWbemClassObject*     pPartitionObject      = NULL;
    IWbemClassObject*     pLetterObject         = NULL;

    // Temporary object for attaching data to the combobox entries
    UsbDevice* deviceData = NULL;

    try
    {
        // Start with allocating the fixed strings
        ALLOC_BSTR(strNamespace, L"root\\cimv2");
        ALLOC_BSTR(strQL, L"WQL");
        ALLOC_BSTR(strQueryDisks, L"SELECT * FROM Win32_DiskDrive WHERE InterfaceType = \"USB\"");

        // Create the IWbemLocator and execute the first query (list of physical disks attached via USB)
        CHECK_OK(CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown, reinterpret_cast<void**>(&pIWbemLocator)), i18n("CoCreateInstance(WbemAdministrativeLocator) failed."));
        CHECK_OK(pIWbemLocator->ConnectServer(strNamespace, NULL, NULL, NULL, 0, NULL, NULL, &pWbemServices), i18n("ConnectServer failed."));
        CHECK_OK(pWbemServices->ExecQuery(strQL, strQueryDisks, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumDisksObject), i18n("Failed to query USB flash devices."));

        // Enumerate the received list of devices
        for (;;)
        {
            // Get the next available device or exit the loop
            ULONG uReturned;
            pEnumDisksObject->Next(WBEM_INFINITE, 1, &pDiskObject, &uReturned);
            if (uReturned == 0)
                break;

            VARIANT val;

            // Fetch the required properties and store them in the UsbDevice object
            UsbDevice* deviceData = new UsbDevice;

            // User-friendly name of the device
            if (pDiskObject->Get(L"Model", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
            {
                if (val.vt == VT_BSTR)
                {
                    deviceData->m_VisibleName = QString::fromWCharArray(val.bstrVal);
                }
                VariantClear(&val);
            }

            // System name of the device
            if (pDiskObject->Get(L"DeviceID", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
            {
                if (val.vt == VT_BSTR)
                {
                    deviceData->m_PhysicalDevice = QString::fromWCharArray(val.bstrVal);
                }
                VariantClear(&val);
            }

            // Size of the devifce
            if (pDiskObject->Get(L"Size", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
            {
                if (val.vt == VT_BSTR)
                {
                    deviceData->m_Size = QString::fromWCharArray(val.bstrVal).toULongLong();
                }
                VariantClear(&val);
            }

            // Sector size of the devifce
            if (pDiskObject->Get(L"BytesPerSector", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
            {
                if (val.vt == VT_I4)
                {
                    deviceData->m_SectorSize = val.intVal;
                }
                VariantClear(&val);
            }

            // The device object is no longer needed, release it
            SAFE_RELEASE(pDiskObject);

            // Construct the request for listing the partitions on the current disk
            QString qstrQueryPartitions = "ASSOCIATORS OF {Win32_DiskDrive.DeviceID='" + deviceData->m_PhysicalDevice + "'} WHERE AssocClass = Win32_DiskDriveToDiskPartition";
            ALLOC_BSTR(strQueryPartitions, reinterpret_cast<const wchar_t*>(qstrQueryPartitions.utf16()));

            // Execute the query
            CHECK_OK(pWbemServices->ExecQuery(strQL, strQueryPartitions, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumPartitionsObject), i18n("Failed to query list of partitions."));

            // Enumerate the received list of partitions
            for (;;)
            {
                // Get the next available partition or exit the loop
                pEnumPartitionsObject->Next(WBEM_INFINITE, 1, &pPartitionObject, &uReturned);
                if (uReturned == 0)
                    break;

                // Fetch the DeviceID property and store it for using in the next request
                QString qstrQueryLetters = "";
                if (pPartitionObject->Get(L"DeviceID", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
                {
                    if (val.vt == VT_BSTR)
                    {
                        qstrQueryLetters = QString::fromWCharArray(val.bstrVal);
                    }
                    VariantClear(&val);
                }

                // The partition object is no longer needed, release it
                SAFE_RELEASE(pPartitionObject);

                // If DeviceID was fetched proceed to the logical disks
                if (qstrQueryLetters != "")
                {
                    // Construct the request for listing the logical disks related to the current partition
                    qstrQueryLetters = "ASSOCIATORS OF {Win32_DiskPartition.DeviceID='" + qstrQueryLetters + "'} WHERE AssocClass = Win32_LogicalDiskToPartition";
                    ALLOC_BSTR(strQueryLetters, reinterpret_cast<const wchar_t*>(qstrQueryLetters.utf16()));

                    // Execute the query
                    CHECK_OK(pWbemServices->ExecQuery(strQL, strQueryLetters, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumLettersObject), i18n("Failed to query list of logical disks."));

                    // Enumerate the received list of logical disks
                    for (;;)
                    {
                        // Get the next available logical disk or exit the loop
                        pEnumLettersObject->Next(WBEM_INFINITE, 1, &pLetterObject, &uReturned);
                        if (uReturned == 0)
                            break;

                        // Fetch the disk letter and add it to the list of volumes in the UsbDevice object
                        if (pLetterObject->Get(L"Caption", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
                        {
                            if (val.vt == VT_BSTR)
                            {
                                deviceData->m_Volumes << QString::fromWCharArray(val.bstrVal);
                            }
                            VariantClear(&val);
                        }

                        // The logical disk object is no longer needed, release it
                        SAFE_RELEASE(pLetterObject);
                    }

                    // Release the logical disks enumerator object and the corresponding query string
                    SAFE_RELEASE(pEnumLettersObject);
                    FREE_BSTR(strQueryLetters);
                }
            }

            // Release the partitions enumerator object and the corresponding query string
            SAFE_RELEASE(pEnumPartitionsObject);
            FREE_BSTR(strQueryPartitions);

            // The device information is now complete, append the entry
            callback(cbParam, deviceData);
            // The object is now under the GUI control, nullify the pointer
            deviceData = NULL;
        }
    }
    catch (QString errMessage)
    {
        // Something bad happened
        QMessageBox::critical(
            QApplication::activeWindow(),
            ApplicationTitle,
            errMessage
        );
    }

    // The cleanup stage
    if (deviceData != NULL)
        delete deviceData;

    SAFE_RELEASE(pLetterObject);
    SAFE_RELEASE(pPartitionObject);
    SAFE_RELEASE(pDiskObject);
    SAFE_RELEASE(pEnumDisksObject);
    SAFE_RELEASE(pEnumPartitionsObject);
    SAFE_RELEASE(pEnumLettersObject);
    SAFE_RELEASE(pWbemServices);
    SAFE_RELEASE(pIWbemLocator);

    FREE_BSTR(strNamespace);
    FREE_BSTR(strQL);
    FREE_BSTR(strQueryDisks);
    FREE_BSTR(strQueryPartitions);
    FREE_BSTR(strQueryLetters);
    return true;
}

bool ensureElevated()
{
    // In Windows the manifest already ensures elevated privileges
    return true;
}
