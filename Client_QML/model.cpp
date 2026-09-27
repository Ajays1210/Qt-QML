#include "model.h"

Model::Model(QObject *parent)
    : QAbstractListModel(parent)
{
}

int Model::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const Item &item = m_items.at(index.row());

    switch (role)
    {
    case UniqueIdRole:
        return item.uniqueId;
    case LatRole:
        return item.lat;
    case LongiRole:
        return item.longi;
    case CommentRole:
        return item.comment;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> Model::roleNames() const
{
    return {
        { UniqueIdRole, "uniqueId" },
        { LatRole, "lat" },
        { LongiRole, "longi" },
        { CommentRole, "comment" }
    };
}

int Model::indexOfId(unsigned int uniqueId) const
{
    for (int i = 0; i < m_items.size(); ++i)
    {
        if (m_items.at(i).uniqueId == uniqueId)
            return i;
    }
    return -1;
}

void Model::addOrReplace(unsigned int uniqueId, float lat, float longi, const QString &comment)
{
    int row = indexOfId(uniqueId);

    if (row >= 0)
    {
        m_items[row] = { uniqueId, lat, longi, comment };
        emit dataChanged(index(row), index(row));
    }
    else
    {
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
        m_items.append({ uniqueId, lat, longi, comment });
        endInsertRows();
    }
}

void Model::removeById(unsigned int uniqueId)
{
    int row = indexOfId(uniqueId);
    if (row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();
}

QVariantMap Model::get(int row) const
{
    QVariantMap map;
    if (row < 0 || row >= m_items.size())
        return map;

    const Item &item = m_items.at(row);
    map["uniqueId"] = item.uniqueId;
    map["lat"] = item.lat;
    map["longi"] = item.longi;
    map["comment"] = item.comment;

    return map;
}