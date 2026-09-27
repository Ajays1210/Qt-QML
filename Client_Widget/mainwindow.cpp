#include "mainwindow.h"

#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QT TCP/UDP Client");
    resize(650, 450);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    auto *title = new QLabel("<h2>Data List</h2>");
    mainLayout->addWidget(title);

    m_list = new QListWidget;
    mainLayout->addWidget(m_list);

    auto *buttonLayout = new QHBoxLayout;

    m_addButton = new QPushButton("Add");
    m_updateButton = new QPushButton("Update");
    m_deleteButton = new QPushButton("Delete");

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_updateButton);
    buttonLayout->addWidget(m_deleteButton);

    mainLayout->addLayout(buttonLayout);

    m_statusLabel = new QLabel("Starting network...");
    mainLayout->addWidget(m_statusLabel);

    connect(m_addButton, &QPushButton::clicked,
            this, &MainWindow::onAddClicked);

    connect(m_updateButton, &QPushButton::clicked,
            this, &MainWindow::onUpdateClicked);

    connect(m_deleteButton, &QPushButton::clicked,
            this, &MainWindow::onDeleteClicked);

    m_network = new Network;
    m_network->moveToThread(&m_networkThread);

    connect(&m_networkThread, &QThread::started,
            m_network, &Network::start);

    connect(&m_networkThread, &QThread::finished,
            m_network, &QObject::deleteLater);

    connect(m_network, &Network::addResponse,
            this, &MainWindow::onAddResponse);

    connect(m_network, &Network::updateResponse,
            this, &MainWindow::onUpdateResponse);

    connect(m_network, &Network::deleteResponse,
            this, &MainWindow::onDeleteResponse);

    connect(m_network, &Network::statusMessage,
            this, &MainWindow::onStatusMessage);

    m_networkThread.start();
}

MainWindow::~MainWindow()
{
    if (m_networkThread.isRunning()) {
        QMetaObject::invokeMethod(m_network, "stop", Qt::BlockingQueuedConnection);
        m_networkThread.quit();
        m_networkThread.wait();
    }
}

void MainWindow::onAddClicked()
{
    openForm(false);
}

void MainWindow::onUpdateClicked()
{
    int row = selectedRow();

    if (row < 0) {
        QMessageBox::warning(this, "Update", "Select an item first.");
        return;
    }

    openForm(true);
}

void MainWindow::onDeleteClicked()
{
    int row = selectedRow();

    if (row < 0) {
        QMessageBox::warning(this, "Delete", "Select an item first.");
        return;
    }

    const Record record = m_records[row];

    const auto result = QMessageBox::question(
        this,
        "Delete",
        QString("Delete Unique ID %1?").arg(record.uniqueId));

    if (result != QMessageBox::Yes)
        return;

    m_statusLabel->setText("Sending delete request...");

    QMetaObject::invokeMethod(
        m_network,
        "sendDelete",
        Qt::QueuedConnection,
        Q_ARG(unsigned int, record.uniqueId));
}

void MainWindow::openForm(bool updateMode)
{
    QDialog dialog(this);
    dialog.setWindowTitle(updateMode ? "Update Data" : "Add Data");
    dialog.resize(420, 260);

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout;

    auto *uniqueIdEdit = new QLineEdit;
    auto *latEdit = new QLineEdit;
    auto *longEdit = new QLineEdit;
    auto *commentEdit = new QLineEdit;

    uniqueIdEdit->setPlaceholderText("Example: 1001");
    latEdit->setPlaceholderText("Example: 12.9716");
    longEdit->setPlaceholderText("Example: 77.5946");
    commentEdit->setMaxLength(49);

    int row = selectedRow();

    if (updateMode && row >= 0) {
        const Record &record = m_records[row];

        uniqueIdEdit->setText(QString::number(record.uniqueId));
        latEdit->setText(QString::number(record.lat, 'f', 6));
        longEdit->setText(QString::number(record.longi, 'f', 6));
        commentEdit->setText(record.comment);

        uniqueIdEdit->setReadOnly(true);
    }

    form->addRow("Unique ID:", uniqueIdEdit);
    form->addRow("Latitude:", latEdit);
    form->addRow("Longitude:", longEdit);
    form->addRow("Comment:", commentEdit);

    layout->addLayout(form);

    auto *buttonLayout = new QHBoxLayout;
    auto *applyButton = new QPushButton("Apply");
    auto *cancelButton = new QPushButton("Cancel");

    buttonLayout->addStretch();
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);

    layout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(applyButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() != QDialog::Accepted)
        return;

    bool idOk = false;
    bool latOk = false;
    bool longOk = false;

    unsigned int uniqueId = uniqueIdEdit->text().toUInt(&idOk);
    float lat = latEdit->text().toFloat(&latOk);
    float longi = longEdit->text().toFloat(&longOk);
    QString comment = commentEdit->text();

    if (!idOk || !latOk || !longOk || comment.isEmpty()) {
        QMessageBox::warning(
            this,
            "Invalid Data",
            "Enter a valid Unique ID, latitude, longitude and comment.");
        return;
    }

    if (!updateMode) {
        for (const Record &record : m_records) {
            if (record.uniqueId == uniqueId) {
                QMessageBox::warning(
                    this, "Duplicate ID",
                    "That Unique ID is already present in the client list.");
                return;
            }
        }
    }

    m_statusLabel->setText(updateMode ? "Sending update request..." : "Sending add request...");

    if (updateMode) {
        QMetaObject::invokeMethod(
            m_network,
            "sendUpdate",
            Qt::QueuedConnection,
            Q_ARG(unsigned int, uniqueId),
            Q_ARG(float, lat),
            Q_ARG(float, longi),
            Q_ARG(QString, comment));
    } else {
        QMetaObject::invokeMethod(
            m_network,
            "sendAdd",
            Qt::QueuedConnection,
            Q_ARG(unsigned int, uniqueId),
            Q_ARG(float, lat),
            Q_ARG(float, longi),
            Q_ARG(QString, comment));
    }
}

int MainWindow::selectedRow() const
{
    if (!m_list->currentItem())
        return -1;

    return m_list->row(m_list->currentItem());
}

void MainWindow::refreshList()
{
    m_list->clear();

    for (const Record &record : m_records) {
        m_list->addItem(
            QString("ID: %1    Lat: %2    Long: %3    Comment: %4")
                .arg(record.uniqueId)
                .arg(record.lat, 0, 'f', 6)
                .arg(record.longi, 0, 'f', 6)
                .arg(record.comment));
    }
}

void MainWindow::clearForm()
{
}

void MainWindow::onAddResponse(Protocol::AddDataResp response)
{
    if (response.Ack != Protocol::ACK_SUCCESS) {
        m_statusLabel->setText(QString("ADD failed for ID %1").arg(response.UniqueID));
        QMessageBox::warning(this, "Add failed", QString("Server rejected ID %1.").arg(response.UniqueID));
        return;
    }

    Record record;
    record.uniqueId = response.UniqueID;
    record.lat = response.lat;
    record.longi = response.longi;
    record.comment = QString::fromUtf8(response.comment);

    m_records.append(record);
    refreshList();

    m_statusLabel->setText(QString("ADD successful. ID %1").arg(response.UniqueID));
}

void MainWindow::onUpdateResponse(Protocol::UpdateDataResp response)
{
    if (response.Ack != Protocol::ACK_SUCCESS) {
        m_statusLabel->setText(QString("UPDATE failed for ID %1").arg(response.UniqueID));
        QMessageBox::warning(this, "Update failed", QString("Server could not find ID %1.").arg(response.UniqueID));
        return;
    }

    for (Record &record : m_records) {
        if (record.uniqueId == response.UniqueID) {
            record.lat = response.lat;
            record.longi = response.longi;
            record.comment = QString::fromUtf8(response.comment);
            break;
        }
    }

    refreshList();

    m_statusLabel->setText(QString("UPDATE successful. ID %1").arg(response.UniqueID));
}

void MainWindow::onDeleteResponse(Protocol::DeleteDataResp response)
{
    if (response.Ack != Protocol::ACK_SUCCESS) {
        m_statusLabel->setText(QString("DELETE failed for ID %1").arg(response.UniqueID));
        QMessageBox::warning(this, "Delete failed", QString("Server could not delete ID %1.").arg(response.UniqueID));
        return;
    }

    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].uniqueId == response.UniqueID) {
            m_records.removeAt(i);
            break;
        }
    }

    refreshList();

    m_statusLabel->setText(QString("DELETE successful. ID %1").arg(response.UniqueID));
}

void MainWindow::onStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
}