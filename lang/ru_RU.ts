<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>ImageWriter</name>
    <message>
        <location filename="../imagewriter.cpp" line="43"/>
        <source>Failed to allocate memory for buffer:</source>
        <translation>Не удалось выделить память для буфера:</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="56"/>
        <source>Failed to open the image file:</source>
        <translation>Не удалось открыть файл образа:</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="73"/>
        <source>Failed to open the drive</source>
        <translation>Не удалось открыть диск</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="78"/>
        <source>Failed to unmount the drive</source>
        <translation>Не удалось отмонтировать диск</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="94"/>
        <source>Failed to open the target device:</source>
        <translation>Не удалось открыть целевое устройство:</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="96"/>
        <source>Failed to lock the target device:</source>
        <translation>Не удалось заблокировать целевое устройство:</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="102"/>
        <source>Failed to get the target device info:</source>
        <translation>Не удалось получить информацию о целевом устройстве:</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="114"/>
        <source>Failed to write to the device:</source>
        <translation>Ошибка записи на целевое устройство:</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="116"/>
        <source>The last block was not fully written (%1 of %2 bytes)!
Aborting.</source>
        <translation>Последний блок был записан не полностью (%1 из %2 байтов)!
Операция прервана.</translation>
    </message>
    <message>
        <location filename="../imagewriter.cpp" line="135"/>
        <source>Failed to read the image file:</source>
        <translation>Ошибка чтения из файла образа:</translation>
    </message>
</context>
<context>
    <name>MainDialog</name>
    <message>
        <location filename="../maindialog.ui" line="105"/>
        <source>&amp;USB Device:</source>
        <translation>&amp;USB-диск:</translation>
    </message>
    <message>
        <location filename="../maindialog.ui" line="124"/>
        <source>Image:</source>
        <translation>Образ:</translation>
    </message>
    <message>
        <location filename="../maindialog.ui" line="173"/>
        <source>%v / %m MB</source>
        <translation>%v / %m МБ</translation>
    </message>
    <message>
        <location filename="../maindialog.ui" line="211"/>
        <source>&amp;Write</source>
        <translation>&amp;Записать</translation>
    </message>
    <message>
        <location filename="../maindialog.ui" line="224"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="64"/>
        <location filename="../maindialog.cpp" line="321"/>
        <location filename="../maindialog.cpp" line="375"/>
        <location filename="../maindialog.cpp" line="376"/>
        <source>MB</source>
        <translation>МБ</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="124"/>
        <location filename="../maindialog.cpp" line="135"/>
        <source>Writing is in progress, abort it?</source>
        <translation>Запись образа не завершена, прервать её?</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="144"/>
        <source>Disk Images</source>
        <translation>Образы дисков</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="144"/>
        <source>All Files</source>
        <translation>Все файлы</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="197"/>
        <source>CoCreateInstance(WbemAdministrativeLocator) failed.</source>
        <translation>Ошибка при вызове CoCreateInstance(WbemAdministrativeLocator).</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="198"/>
        <source>ConnectServer failed.</source>
        <translation>Ошибка при вызове ConnectServer.</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="199"/>
        <source>Failed to query USB flash devices.</source>
        <translation>Ошибка при запросе списка USB флэш-дисков.</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="253"/>
        <source>Failed to query list of partitions.</source>
        <translation>Ошибка при запросе списка разделов.</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="285"/>
        <source>Failed to query list of logical disks.</source>
        <translation>Ошибка при запросе списка логических дисков.</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="321"/>
        <source>&lt;unmounted&gt;</source>
        <translation>&lt;не примонтировано&gt;</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="374"/>
        <source>The image is larger than your selected device!</source>
        <translation>Файл образа больше по размеру, чем выбранный диск!</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="375"/>
        <source>Image size:</source>
        <translation>Размер образа:</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="375"/>
        <location filename="../maindialog.cpp" line="376"/>
        <source>b</source>
        <translation>б</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="376"/>
        <source>Disk size:</source>
        <translation>Размер диска:</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="384"/>
        <source>Writing an image will erase all existing data on the selected device.
Are you sure you wish to proceed?</source>
        <translation>Запись образа уничтожит все имеющиеся данные на выбранном диске.
Вы уверены, что хотите продолжить?</translation>
    </message>
    <message>
        <location filename="../maindialog.cpp" line="498"/>
        <source>The operation completed successfully.</source>
        <translation>Операция успешно завершена.</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../common.cpp" line="32"/>
        <source>Error code:</source>
        <translation>Код ошибки:</translation>
    </message>
    <message>
        <location filename="../maindialog.h" line="81"/>
        <source>Unknown Device</source>
        <translation>Неизвестное устройство</translation>
    </message>
</context>
</TS>
