/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef USBDEVICE_H
#define USBDEVICE_H

#include <QObject>
#include <QStringList>
#include <KLocalizedString>
#include <KFormat>

class UsbDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString visibleName READ visibleName WRITE setVisibleName NOTIFY visibleNameChanged)
    Q_PROPERTY(QStringList volumes READ volumes WRITE setVolumes NOTIFY volumesChanged)
    Q_PROPERTY(quint64 size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QString physicalDevice READ physicalDevice WRITE setPhysicalDevice NOTIFY physicalDeviceChanged)
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameChanged)

public:
    explicit UsbDevice(QObject *parent = nullptr) : QObject(parent), m_Size(0) {}

    QString visibleName() const { return m_VisibleName; }
    void setVisibleName(const QString &visibleName) {
        if (m_VisibleName == visibleName) return;
        m_VisibleName = visibleName;
        emit visibleNameChanged();
        emit displayNameChanged();
    }

    QStringList volumes() const { return m_Volumes; }
    void setVolumes(const QStringList &volumes) {
        if (m_Volumes == volumes) return;
        m_Volumes = volumes;
        emit volumesChanged();
        emit displayNameChanged();
    }

    quint64 size() const { return m_Size; }
    void setSize(quint64 size) {
        if (m_Size == size) return;
        m_Size = size;
        emit sizeChanged();
        emit displayNameChanged();
    }

    QString physicalDevice() const { return m_PhysicalDevice; }
    void setPhysicalDevice(const QString &physicalDevice) {
        if (m_PhysicalDevice == physicalDevice) return;
        m_PhysicalDevice = physicalDevice;
        emit physicalDeviceChanged();
    }

    QString displayName() const {
        return ((m_Volumes.isEmpty()) ? i18n("<unmounted>")
                : m_Volumes.join(", ")) + " - " + m_VisibleName + " (" + KFormat().formatByteSize(m_Size) + QLatin1Char(')');
    }

    QString     m_VisibleName = i18n("Unknown Device");
    QStringList m_Volumes;
    quint64     m_Size = 0;
    QString     m_PhysicalDevice;
    quint32     m_SectorSize;

signals:
    void visibleNameChanged();
    void volumesChanged();
    void sizeChanged();
    void physicalDeviceChanged();
    void displayNameChanged();

};

#endif // USBDEVICE_H
