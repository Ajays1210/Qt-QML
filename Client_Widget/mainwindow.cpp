#include "mainwindow.h"

#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <cstring>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QT TCP/UDP Client");
    resize(650, 450);

    // ---- Build the screen: title, list, three buttons, status line ----
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    auto *title = new QLabel("<h2>Data List</h2>");
    mainLayout->addWidget(title);

    m_list = new QListWidget;
    mainLayout->addWidget(m_list);

    auto *buttonLayout = new QHBoxLayout;

    auto *addButton = new QPushButton("Add");
    auto *updateButton = new QPushButton("Update");
    auto *deleteButton = new QPushButton("Delete");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(deleteButton);

    mainLayout->addLayout(buttonLayout);

    m_statusLabel = new QLabel("Starting network...");
    mainLayout->addWidget(m_statusLabel);

    // ---- Button clicks ----
    // Add and Update just open the form (false = Add, true = Update).
    connect(addButton, &QPushButton::clicked,
            this, [this]() { openForm(false); });

    connect(updateButton, &QPushButton::clicked,
            this, [this]() { openForm(true); });

    connect(deleteButton, &QPushButton::clicked,
            this, &MainWindow::onDeleteClicked);

    // ---- Network thread ----
    // The Network object is created WITHOUT a parent (an object with a parent
    // cannot be moved to another thread), then moved to its own thread.
    m_network = new Network;
    m_network->moveToThread(&m_networkThread);

    // When the thread starts, Network::start() runs inside it
    // (creates the sockets there).
    connect(&m_networkThread, &QThread::started,
            m_network, &Network::start);

    // Delete the Network object safely when its thread finishes.
    connect(&m_networkThread, &QThread::finished,
            m_network, &QObject::deleteLater);

    // Network (network thread) -> MainWindow (main thread).
    // The threads differ, so Qt automatically queues these signals.
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

// Runs when the window is closed. Stops the network thread cleanly.
MainWindow::~MainWindow()
{
    m_networkThread.quit();   // end the thread's event loop
    m_networkThread.wait();   // block until the thread has fully finished
    // When the thread finishes, the 'finished' signal (connected in the
    // constructor) deletes the Network object, and its sockets close with it.
}

// Delete button: ask for confirmation, then send the delete command.
// The row is NOT removed here. It is removed in onDeleteResponse, only if
// the server answers with Ack = success.
void MainWindow::onDeleteClicked()
{
    int row = selectedRow();

    if (row < 0) {
        QMessageBox::warning(this, "Delete", "Select an item first.");
        return;
    }

    // Copy the ID now: while the question box is open, a response could
    // arrive and rebuild the list.
    const unsigned int uniqueId = m_records[row].uniqueId;

    const auto result = QMessageBox::question(
        this,
        "Delete",
        QString("Delete Unique ID %1?").arg(uniqueId));

    if (result != QMessageBox::Yes)
        return;

    m_statusLabel->setText("Sending delete request...");

    // Run sendDelete() in the NETWORK thread (m_network lives there).
    // Queued = this call returns at once and the GUI never waits for the network.
    QMetaObject::invokeMethod(
        m_network,
        [this, uniqueId]() { m_network->sendDelete(uniqueId); },
        Qt::QueuedConnection);
}

// Shows the Add/Update form. If the user presses Apply, the command is sent to
// the server. The list is NOT changed here, only when the response arrives.
void MainWindow::openForm(bool updateMode)
{
    // Update needs a selected row to edit.
    const int row = selectedRow();

    if (updateMode && row < 0) {
        QMessageBox::warning(this, "Update", "Select an item first.");
        return;
    }

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
    commentEdit->setMaxLength(49);   // struct field is char[50], 1 byte kept for '\0'

    // Update mode: fill the fields from the selected row, and lock the ID.
    if (updateMode) {
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

    // exec() shows the form and waits here until it is closed.
    if (dialog.exec() != QDialog::Accepted)
        return;

    // Convert the text to numbers. The 'ok' flags say if the conversion worked.
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

    m_statusLabel->setText(updateMode ? "Sending update request..." : "Sending add request...");

    // Run sendAdd()/sendUpdate() in the NETWORK thread (queued, GUI never waits).
    if (updateMode) {
        QMetaObject::invokeMethod(
            m_network,
            [this, uniqueId, lat, longi, comment]() {
                m_network->sendUpdate(uniqueId, lat, longi, comment);
            },
            Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(
            m_network,
            [this, uniqueId, lat, longi, comment]() {
                m_network->sendAdd(uniqueId, lat, longi, comment);
            },
            Qt::QueuedConnection);
    }
}

// Row number of the selected list item, or -1 if nothing is selected.
int MainWindow::selectedRow() const
{
    if (!m_list->currentItem())
        return -1;

    return m_list->row(m_list->currentItem());
}

// Clears the list widget and fills it again from m_records.
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

// Used by both onAddResponse and onUpdateResponse (the structs are identical).
// If the ID is already in the list, that row is replaced. Otherwise a new row
// is added.
void MainWindow::storeRecord(const Protocol::AddDataResp &response)
{
    Record record;
    record.uniqueId = response.UniqueID;
    record.lat = response.lat;
    record.longi = response.longi;

    // Read the text up to the first '\0', but never past the 50 bytes.
    record.comment = QString::fromUtf8(
        response.comment,
        static_cast<int>(strnlen(response.comment, sizeof(response.comment))));

    bool found = false;
    for (Record &existing : m_records) {
        if (existing.uniqueId == record.uniqueId) {
            existing = record;
            found = true;
            break;
        }
    }

    if (!found)
        m_records.append(record);

    refreshList();
}

// UDP response to an Add. Only if Ack is success is the item put in the list.
void MainWindow::onAddResponse(Protocol::AddDataResp response)
{
    if (response.Ack != Protocol::ACK_SUCCESS) {
        m_statusLabel->setText(QString("ADD failed for ID %1").arg(response.UniqueID));
        QMessageBox::warning(this, "Add failed", QString("Server rejected ID %1.").arg(response.UniqueID));
        return;
    }

    storeRecord(response);

    m_statusLabel->setText(QString("ADD successful. ID %1").arg(response.UniqueID));
}

// UDP response to an Update. On success the existing row is replaced.
void MainWindow::onUpdateResponse(Protocol::UpdateDataResp response)
{
    if (response.Ack != Protocol::ACK_SUCCESS) {
        m_statusLabel->setText(QString("UPDATE failed for ID %1").arg(response.UniqueID));
        QMessageBox::warning(this, "Update failed", QString("Server could not find ID %1.").arg(response.UniqueID));
        return;
    }

    storeRecord(response);

    m_statusLabel->setText(QString("UPDATE successful. ID %1").arg(response.UniqueID));
}

// UDP response to a Delete. On success the row is removed.
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

// Text from Network (connected / failed / bound ...), shown in the status line.
void MainWindow::onStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
}