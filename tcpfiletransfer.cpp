#include "tcpfiletransfer.h"

TcpFileTransfer::TcpFileTransfer(QWidget *parent)
    : QDialog(parent),
    localFile(nullptr),
    totalBytes(0),
    bytesWritten(0),
    bytesToWrite(0),
    loadSize(4096),
    byteReceived(0),
    fileNameSize(0)
{
    // 初始化界面元件
    tabWidget = new QTabWidget(this);

    // 客戶端選項卡
    QWidget *clientTab = new QWidget;
    clientIpInput = new QTextEdit;
    clientIpInput->setFixedHeight(30);
    clientIpInput->setPlaceholderText("輸入 IP 地址");
    clientPortInput = new QTextEdit;
    clientPortInput->setFixedHeight(30);
    clientPortInput->setPlaceholderText("輸入埠號");
    openFileButton = new QPushButton("選擇檔案");
    startClientButton = new QPushButton("啟動客戶端");
    clientProgressBar = new QProgressBar;
    clientStatusLabel = new QLabel("客戶端已準備就緒");

    QVBoxLayout *clientLayout = new QVBoxLayout;
    QHBoxLayout *clientInputLayout = new QHBoxLayout;
    clientInputLayout->addWidget(clientIpInput);
    clientInputLayout->addWidget(clientPortInput);
    clientLayout->addLayout(clientInputLayout);
    clientLayout->addWidget(openFileButton);
    clientLayout->addWidget(startClientButton);
    clientLayout->addWidget(clientProgressBar);
    clientLayout->addWidget(clientStatusLabel);
    clientTab->setLayout(clientLayout);

    // 伺服器選項卡
    QWidget *serverTab = new QWidget;
    serverPortInput = new QTextEdit;
    serverPortInput->setFixedHeight(30);
    serverPortInput->setPlaceholderText("輸入埠號");
    startServerButton = new QPushButton("啟動伺服器");
    serverProgressBar = new QProgressBar;
    serverStatusLabel = new QLabel("伺服器已準備就緒");

    QVBoxLayout *serverLayout = new QVBoxLayout;
    serverLayout->addWidget(new QLabel("伺服器埠號："));
    serverLayout->addWidget(serverPortInput);
    serverLayout->addWidget(startServerButton);
    serverLayout->addWidget(serverProgressBar);
    serverLayout->addWidget(serverStatusLabel);
    serverTab->setLayout(serverLayout);

    tabWidget->addTab(clientTab, "客戶端");
    tabWidget->addTab(serverTab, "伺服器");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    setWindowTitle("TCP 檔案傳輸");

    // 連接訊號與槽
    connect(openFileButton, &QPushButton::clicked, this, &TcpFileTransfer::openFile);
    connect(startClientButton, &QPushButton::clicked, this, &TcpFileTransfer::startClient);
    connect(&tcpClient, &QTcpSocket::connected, this, &TcpFileTransfer::startFileTransfer);
    connect(&tcpClient, &QTcpSocket::bytesWritten, this, &TcpFileTransfer::updateClientProgress);
    connect(startServerButton, &QPushButton::clicked, this, &TcpFileTransfer::startServer);
    connect(&tcpServer, &QTcpServer::newConnection, this, &TcpFileTransfer::acceptConnection);
    connect(&tcpServer, &QTcpServer::acceptError, this, &TcpFileTransfer::displayServerError);
}

TcpFileTransfer::~TcpFileTransfer()
{
    if (localFile)
        delete localFile;
}

// 客戶端功能
void TcpFileTransfer::openFile()
{
    fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        startClientButton->setEnabled(true);
}

void TcpFileTransfer::startClient()
{
    bool portOk;
    int port = clientPortInput->toPlainText().toInt(&portOk);
    if (!portOk) {
        QMessageBox::warning(this, "錯誤", "無效的埠號");
        return;
    }

    tcpClient.connectToHost(clientIpInput->toPlainText(), port);
    clientStatusLabel->setText("正在連接...");
}

void TcpFileTransfer::startFileTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly)) {
        QMessageBox::warning(this, "錯誤", "無法打開檔案");
        return;
    }
    totalBytes = localFile->size();
    QDataStream out(&outBlock, QIODevice::WriteOnly);
    out << qint64(0) << qint64(0) << fileName;
    totalBytes += outBlock.size();
    out.device()->seek(0);
    out << totalBytes << qint64(outBlock.size() - sizeof(qint64) * 2);
    bytesToWrite = totalBytes - tcpClient.write(outBlock);
    clientStatusLabel->setText("已連接");
}

void TcpFileTransfer::updateClientProgress(qint64 numBytes)
{
    bytesWritten += numBytes;
    if (bytesToWrite > 0) {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= tcpClient.write(outBlock);
    } else {
        localFile->close();
    }
    clientProgressBar->setMaximum(totalBytes);
    clientProgressBar->setValue(bytesWritten);
    clientStatusLabel->setText(QString("已傳送 %1 byte").arg(bytesWritten));
}

// 伺服器功能
void TcpFileTransfer::startServer()
{
    bool portOk;
    int port = serverPortInput->toPlainText().toInt(&portOk);
    if (!portOk) {
        QMessageBox::warning(this, "錯誤", "無效的埠號");
        return;
    }

    if (!tcpServer.listen(QHostAddress::Any, port)) {
        QMessageBox::critical(this, "錯誤", "無法啟動伺服器");
        return;
    }
    serverStatusLabel->setText("正在監聽...");
}

void TcpFileTransfer::acceptConnection()
{
    tcpServerConnection = tcpServer.nextPendingConnection();
    connect(tcpServerConnection, &QTcpSocket::readyRead, this, &TcpFileTransfer::updateServerProgress);
    serverStatusLabel->setText("已接受連接");
}

void TcpFileTransfer::updateServerProgress()
{
    // 處理數據接收（類似客戶端）
}

void TcpFileTransfer::displayServerError(QAbstractSocket::SocketError socketError)
{
    QMessageBox::critical(this, "錯誤", tcpServer.errorString());
}
