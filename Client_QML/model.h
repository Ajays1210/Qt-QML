#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <QVariantMap>

class Model : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        UniqueIdRole = Qt::UserRole + 1,
        LatRole,
        LongiRole,
        CommentRole
    };

    explicit Model(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addOrReplace(unsigned int uniqueId, float lat, float longi, const QString &comment);
    void removeById(unsigned int uniqueId);

    Q_INVOKABLE QVariantMap get(int row) const;

private:
    struct Item
    {
        unsigned int uniqueId;
        float lat;
        float longi;
        QString comment;
    };

    QList<Item> m_items;
    int indexOfId(unsigned int uniqueId) const;
};

#endif