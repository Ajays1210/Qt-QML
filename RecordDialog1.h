// RecordDialog.h
// A simple popup form used for both Add (starts empty) and Update
// (starts filled in with the existing values).
#pragma once

#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

#include "Protocol.h"

class RecordDialog : public QDialog {
    Q_OBJECT
public:
    // If updateMode is true, the ID box is locked (Update must target an
    // existing ID) and the other boxes start filled in with the current values.
    RecordDialog(QString title, bool updateMode, unsigned int id,
                 double lat, double lon, QString comment, QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(title);
        setModal(true);

        idBox = new QSpinBox();
        idBox->setRange(1, 2000000000);
        idBox->setValue((int)id);
        idBox->setEnabled(!updateMode);

        latBox = new QDoubleSpinBox();
        latBox->setRange(-90.0, 90.0);
        latBox->setDecimals(5);
        latBox->setValue(lat);

        lonBox = new QDoubleSpinBox();
        lonBox->setRange(-180.0, 180.0);
        lonBox->setDecimals(5);
        lonBox->setValue(lon);

        commentBox = new QLineEdit(comment);
        commentBox->setMaxLength(COMMENT_LENGTH - 1);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        buttons->button(QDialogButtonBox::Ok)->setText("Apply");
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        QFormLayout *form = new QFormLayout(this);
        form->addRow("Unique ID:", idBox);
        form->addRow("Latitude:", latBox);
        form->addRow("Longitude:", lonBox);
        form->addRow("Comment:", commentBox);
        form->addRow(buttons);
    }

    unsigned int getId()      { return (unsigned int)idBox->value(); }
    float        getLat()     { return (float)latBox->value(); }
    float        getLon()     { return (float)lonBox->value(); }
    QString      getComment() { return commentBox->text(); }

private:
    QSpinBox *idBox;
    QDoubleSpinBox *latBox;
    QDoubleSpinBox *lonBox;
    QLineEdit *commentBox;
};
