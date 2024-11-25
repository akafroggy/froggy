#ifndef TCPFILETRANSFER_H
#define TCPFILETRANSFER_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QTabWidget>
#include <QMessageBox>
#include <QDataStream>
#include <QFileDialog>

class TcpFileTransfer : public QDialog
{
    Q_OBJECT

public:
    explicit TcpFileTransfer(QWidget *parent = nullptr);
    ~TcpFileTransfer();

private slots:
    // Client Slots
    void openFile();
    void startClient();
    void startFileTransfer();
    void updateClientProgress(qint64 numBytes);

    // Server Slots
    void startServer();
    void acceptConnection();
    void updateServerProgress();
    void displayServerError(QAbstractSocket::SocketError socketError);

private:
    // UI Components
    QTabWidget *tabWidget;

    // Client Widgets
    QTextEdit *clientIpInput;
    QTextEdit *clientPortInput;
    QPushButton *openFileButton;
    QPushButton *startClientButton;
    QProgressBar *clientProgressBar;
    QLabel *clientStatusLabel;

    // Server Widgets
    QTextEdit *serverPortInput;
    QPushButton *startServerButton;
    QProgressBar *serverProgressBar;
    QLabel *serverStatusLabel;

    // Networking
    QTcpSocket tcpClient;
    QTcpServer tcpServer;
    QTcpSocket *tcpServerConnection;

    // File Handling
    QFile *localFile;
    QString fileName;
    qint64 totalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 loadSize;
    QByteArray outBlock;
    qint64 byteReceived;
    qint64 fileNameSize;
    QByteArray inBlock;
};

#endif // TCPFILETRANSFER_H
