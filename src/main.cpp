#include <QCoreApplication>
#include <QFile>
#include <QBuffer>
#include <QtZlib/zlib.h>

void zLib (QString m_fileName, QByteArray data, int mode)
{
    QByteArray processed_data;

    QBuffer checkHeader(&data);
    checkHeader.open(QIODevice::ReadOnly);
    checkHeader.seek(3);
    QByteArray dataHeader = checkHeader.read(5);
    checkHeader.close();

    static const char pes_header_data[] = {'\x57', '\x45', '\x53', '\x59', '\x53'};

    if (mode == 0) {
        if (dataHeader.toHex() != QByteArray::fromRawData(pes_header_data, sizeof(pes_header_data)).toHex()) {
            qWarning() << "ERROR: This is not a PES zlibbed file!";
            return;
        }

        unsigned char* buffer_data = NULL;
        uint32_t size = *(uint32_t*)(data.data() + 0x0c);
        uLong source_len = data.size();
        uLong buffer_len = (uLongf)size;

        if((buffer_data = (unsigned char*)malloc(sizeof(unsigned char) * buffer_len)) == NULL) {
            qWarning() << "ERROR: Could not allocate buffer!";
            return;
        }

        int uncompress_status = uncompress(reinterpret_cast<unsigned char*>(buffer_data), &buffer_len, reinterpret_cast<unsigned char*>(data.data() + 0x10), source_len  - 0x10);
        if (uncompress_status != Z_OK) {
            qWarning() << "Unzlibbing failed. Error code: " + QString::number(uncompress_status);
            return;
        }
        else {
            processed_data = QByteArray::fromRawData((char*)buffer_data, buffer_len);

            QFile output(m_fileName + ".unzlib");
            output.open(QIODevice::WriteOnly);
            output.write(processed_data);
            output.close();
        }
    }

    if (mode == 1) {
        if (dataHeader.toHex() == QByteArray::fromRawData(pes_header_data, sizeof(pes_header_data)).toHex()) {
            qWarning() << "ERROR: This file is already zlibbed.";
            return;
        }

        QByteArray buffer_data(compressBound(data.length()), '\0');
        uLong source_len = data.length();
        uLong buffer_len = compressBound(source_len);

        int compress_status = compress(reinterpret_cast<unsigned char*>(buffer_data.data()), &buffer_len, reinterpret_cast<unsigned char*>(data.data()), source_len);
        if (compress_status != Z_OK) {
            qWarning() << "Zlibbing failed. Error code: " + QString::number(compress_status);
            return;
        }

        QByteArray uncompressed_size(reinterpret_cast<const char *>(&source_len), sizeof(int));
        QByteArray compressed_size(reinterpret_cast<const char *>(&buffer_len), sizeof(int));

        processed_data = QByteArray::fromRawData(buffer_data.data(), buffer_len);
        processed_data.insert(0, uncompressed_size);
        processed_data.insert(0, nullptr);
        processed_data.insert(0, compressed_size);
        processed_data.insert(0, QByteArray::fromHex("0001015745535953"));

        QFile output(m_fileName + ".zlib");
        output.open(QIODevice::WriteOnly);
        output.write(processed_data);
        output.close();
    }

    if (mode == 0)
        qInfo()<< "SUCCES! File was sucessfully unzlibbed.";
    else
        qInfo()<< "SUCCES! File was sucessfully zlibbed.";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStringList arguments = QCoreApplication::arguments();

    if(arguments.count() < 3) {
        system("cls");
        qInfo("||||||||||||||||||||||||||||||||||||||||||");
        qInfo("|                                        |");
        qInfo("|           PES ZlibTool v1.00           |");
        qInfo("|  (c) 2021 IT World Software Services   |");
        qInfo("|                                        |");
        qInfo("||||||||||||||||||||||||||||||||||||||||||");
        qInfo("\n");
        qInfo("  Usage: zlibtool [options] <input-file>");
        qInfo("\n");
        qInfo("[Options]");
        qInfo("  -z: zlib the input file");
        qInfo("  -u: unzlib the input file");
        qInfo("\n");
        qInfo("* Informations");
        qInfo("  The result of zlibbed file will have a .zlib extension");
        qInfo("  The result of unzlibbed file will have a .unzlib extension");
        return 0;
    }

    if (arguments.at(1) == "-z" || arguments.at(1) == "-u")
    {
        if (arguments.at(1) == "-z" && arguments.at(2).contains(".", Qt::CaseInsensitive)){
            QString m_fileName = arguments.at(2);
            QFile filename(m_fileName);

            if (!filename.exists()) {
                qWarning() << "ERROR: The input filename doesn't exist!";
                return 1;
            } else {
                filename.open(QIODevice::ReadOnly);
                QByteArray data = filename.readAll();
                filename.close();

                zLib(m_fileName, data, 1);
            }
        }

        if (arguments.at(1) == "-u" && arguments.at(2).contains(".", Qt::CaseInsensitive)){
            QString m_fileName = arguments.at(2);
            QFile filename(m_fileName);

            if (!filename.exists()) {
                qWarning() << "ERROR: The input filename doesn't exist!";
                return 1;
            } else {
                filename.open(QIODevice::ReadOnly);
                QByteArray data = filename.readAll();
                filename.close();

                zLib(m_fileName, data, 0);
            }
        }

        if (!arguments.at(2).contains(".", Qt::CaseInsensitive))
            qWarning() << "Missing valid input filename.";
    } else
        qWarning() << "Missing valid arguments, try again.";

    return 0;
}

QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = zlibtool
TEMPLATE = app

VERSION = 1.0.0
QMAKE_TARGET_COMPANY = "IT World\\256 Software Solutions"
QMAKE_TARGET_PRODUCT = "ZlibTool"
QMAKE_TARGET_DESCRIPTION = "ZlibTool"
QMAKE_TARGET_COPYRIGHT = "\\251 IT World\\256 Software Solutions"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
