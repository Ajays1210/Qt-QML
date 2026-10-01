#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <QVariantMap>

// The list data shown by the QML ListView. Deriving from QAbstractListModel
// is the standard Qt way to feed a list to QML: when we add or remove items
// here, the ListView redraws by itself.
class Model : public QAbstractListModel
{
    Q_OBJECT

public:
    // One number per field. QML uses these through roleNames() below.
    // Qt::UserRole + 1 is the first number free for custom use.
    enum Roles
    {
        UniqueIdRole = Qt::UserRole + 1,
        LatRole,
        LongiRole,
        CommentRole
    };

    explicit Model(QObject *parent = nullptr);

    // The next three are required overrides. QML calls them automatically.
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;       // how many rows
    QVariant data(const QModelIndex &index, int role) const override;             // value of one field of one row
    QHash<int, QByteArray> roleNames() const override;                            // role number -> name used in QML

    // Called by Controller when a response arrives with Ack success.
    void addOrReplace(unsigned int uniqueId, float lat, float longi, const QString &comment);
    void removeById(unsigned int uniqueId);

    // Called from QML (Update and Delete buttons) to read one row by number.
    Q_INVOKABLE QVariantMap get(int row) const;

private:
    // One row of the list.
    struct Item
    {
        unsigned int uniqueId;
        float lat;
        float longi;
        QString comment;
    };

    QList<Item> m_items;                         // all rows
    int indexOfId(unsigned int uniqueId) const;  // finds the row with this ID, -1 if none
};

#endif