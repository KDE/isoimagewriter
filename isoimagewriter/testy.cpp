/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  KDE neon <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "testy.h"
#include "isoimagewriter_debug.h"
#include "usbdevice.h"

#include <KAuth>
#include <QApplication>
#include <QMessageBox>
#include <QTimer>

int main(int argc, char *argv[])
{
    Testy testy(argc, argv);
    return 0;
}

Testy::Testy(int argc, char *argv[]) {
    QApplication app(argc, argv);
    m_widget = new QWidget();
    m_layout = new QHBoxLayout(m_widget);
    m_widget->setLayout(m_layout);
    m_button = new QPushButton("hello", m_widget);
    m_layout->addWidget(m_button);
    m_deviceList = new QComboBox;
    m_layout->addWidget(m_deviceList);
    connect(m_button, SIGNAL(clicked()), this, SLOT(run()));
    //QTimer::singleShot(0, this, SLOT(runAsync()));
    QTimer::singleShot(0, this, SLOT(runWriteImage()));
    m_widget->show();
    app.exec();    
}

void Testy::run() {
    qCDebug(ISOIMAGEWRITER_LOG) << "run";
    KAuth::Action action(QLatin1String("org.kde.imagewriter.writeimage"));
    action.setHelperId("org.kde.imagewriter");
    //KAuth::Action action(QLatin1String("org.kde.kcontrol.kcmplymouth.install"));
    //action.setHelperId("org.kde.kcontrol.kcmplymouth");
    //action.setArguments(helperargs);
    QVariantMap helperargs;
    helperargs[QStringLiteral("filename")] = "bar";
    action.setArguments(helperargs);
    
    KAuth::ExecuteJob *job = action.execute();
    bool execSuccess = job->exec();
    if (!execSuccess) {
        QMessageBox::information(m_button, "Error", QString("KAuth returned an error code: %1 string: %2").arg(job->error()).arg(job->errorString()));
    } else {
        qCDebug(ISOIMAGEWRITER_LOG) << "all good";
        QVariantMap data = job->data();
        qCDebug(ISOIMAGEWRITER_LOG) << "returned: " << data["contents"].value<QString>();
        qCDebug(ISOIMAGEWRITER_LOG) << "returned: " << data.value("contents").value<QString>();
        QString contents = job->data()["contents"].toString();
        qCDebug(ISOIMAGEWRITER_LOG) << "returned: " << contents;
    }
}

void Testy::runAsync() {
    qCDebug(ISOIMAGEWRITER_LOG) << "runAsync";
    KAuth::Action action(QLatin1String("org.kde.imagewriter.writeimage"));
    action.setHelperId("org.kde.imagewriter");
    QVariantMap helperargs;
    helperargs[QStringLiteral("filename")] = "bar";
    action.setArguments(helperargs);
    KAuth::ExecuteJob *job = action.execute();
    connect(job, SIGNAL(percent(KJob*, unsigned long)), this, SLOT(progressStep(KJob*, unsigned long)));
    connect(job, SIGNAL(newData(const QVariantMap &)), this, SLOT(progressStep(const QVariantMap &)));
    connect(job, SIGNAL(statusChanged(KAuth::Action::AuthStatus)), this, SLOT(statusChanged(KAuth::Action::AuthStatus)));
    connect(job, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)));
    job->start();
    qCDebug(ISOIMAGEWRITER_LOG) << "runAsync start()";
    qDebug() << "DAVE" << action.isValid();
}

void Testy::progressStep(KJob* job, unsigned long step) {
    qCDebug(ISOIMAGEWRITER_LOG) << "progressStep %() " << step;
    if (step == 2) {
        qDebug() << "KILL!";
        KAuth::ExecuteJob *job2 = (KAuth::ExecuteJob *)job;
        job2->kill();
    }
}


void Testy::statusChanged(KAuth::Action::AuthStatus status) {
    qCDebug(ISOIMAGEWRITER_LOG) << "status: " << status;
}

void Testy::progressStep(const QVariantMap &) {
    qCDebug(ISOIMAGEWRITER_LOG) << "progressStep() ";// << step;
}

void Testy::finished(KJob* job) {
    qCDebug(ISOIMAGEWRITER_LOG) << "finished() " << job->error();
    KAuth::ExecuteJob *job2 = (KAuth::ExecuteJob *)job;
    qCDebug(ISOIMAGEWRITER_LOG) << "finished() " << job2->data();
}

void addFlashDeviceCallback(void* cbParam, UsbDevice* device)
{
    qCDebug(ISOIMAGEWRITER_LOG) << "addFlashDeviceCallback";
    /*
    Ui::MainDialog* ui = (Ui::MainDialog*)cbParam;
    ui->deviceList->addItem(device->formatDisplayName(), QVariant::fromValue(device));
    */
}

// Reloads the list of USB flash disks
void Testy::enumFlashDevices()
{
    // Remember the currently selected device
    QString selectedDevice = "";
    /*
    int idx = m_deviceList->currentIndex();
    if (idx >= 0)
    {
        UsbDevice* dev = m_deviceList->itemData(idx).value<UsbDevice*>();
        selectedDevice = dev->m_PhysicalDevice;
    }
    */
    // Remove the existing entries
    //cleanup();
    m_deviceList->clear();
    // Disable the combobox
    // TODO: Disable the whole dialog
    m_deviceList->setEnabled(false);

    platformEnumFlashDevices(addFlashDeviceCallback, this);

    // Restore the previously selected device (if present)
    if (selectedDevice != "")
        for (int i = 0; i < m_deviceList->count(); ++i)
        {
            UsbDevice* dev = m_deviceList->itemData(i).value<UsbDevice*>();
            if (dev->m_PhysicalDevice == selectedDevice)
            {
                m_deviceList->setCurrentIndex(i);
                break;
            }
        }
    // Reenable the combobox
    m_deviceList->setEnabled(true);
    /*
    // Update the Write button enabled/disabled state
    m_writeButton->setEnabled((m_deviceList->count() > 0) && (m_ImageFile != ""));
    // Update the Clear button enabled/disabled state
    m_clearButton->setEnabled(m_deviceList->count() > 0);
    */
}

void Testy::runWriteImage() {
    qCDebug(ISOIMAGEWRITER_LOG) << "runWriteImage";
    KAuth::Action action(QLatin1String("org.kde.imagewriter.writefile"));
    action.setHelperId("org.kde.imageimage");
    QVariantMap helperargs;
    helperargs[QStringLiteral("zeroing")] = "false";
    helperargs[QStringLiteral("filename")] = "/home/jr/src/iso/neon-useredition-20170323-1018-amd64.iso";
    helperargs[QStringLiteral("usbdevice")] = "false";
    
    action.setArguments(helperargs);
    KAuth::ExecuteJob *job = action.execute();
    connect(job, SIGNAL(percent(KJob*, unsigned long)), this, SLOT(progressStep(KJob*, unsigned long)));
    connect(job, SIGNAL(newData(const QVariantMap &)), this, SLOT(progressStep(const QVariantMap &)));
    connect(job, SIGNAL(statusChanged(KAuth::Action::AuthStatus)), this, SLOT(statusChanged(KAuth::Action::AuthStatus)));
    connect(job, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)));
    job->start();
    qCDebug(ISOIMAGEWRITER_LOG) << "runWriteImage start()";
    qDebug() << "action.isValid()? " << action.isValid();
}
