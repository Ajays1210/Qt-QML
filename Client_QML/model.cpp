#include "model.h"

Model::Model(QObject *parent)
    : QAbstractListModel(parent)
{
}

// Number of rows. For a flat list, a valid parent means "asking about children
// of a row", and a list row has none, so return 0.
int Model::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

// Called by the ListView for every field it shows. "role" says which field.
// Returns an empty QVariant if the row is invalid or the role is unknown.
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

// Gives each role number a name. These names (uniqueId, lat, longi, comment)
// are the words used inside the delegate in Main.qml.
QHash<int, QByteArray> Model::roleNames() const
{
    return {
        { UniqueIdRole, "uniqueId" },
        { LatRole, "lat" },
        { LongiRole, "longi" },
        { CommentRole, "comment" }
    };
}

// Linear search for the row that has this ID. Returns -1 if not found.
int Model::indexOfId(unsigned int uniqueId) const
{
    for (int i = 0; i < m_items.size(); ++i)
    {
        if (m_items.at(i).uniqueId == uniqueId)
            return i;
    }
    return -1;
}

// If the ID already exists, replace that row (this is the Update case).
// If not, append a new row (this is the Add case).
void Model::addOrReplace(unsigned int uniqueId, float lat, float longi, const QString &comment)
{
    int row = indexOfId(uniqueId);

    if (row >= 0)
    {
        m_items[row] = { uniqueId, lat, longi, comment };
        emit dataChanged(index(row), index(row));   // tells the ListView: redraw this row
    }
    else
    {
        // beginInsertRows/endInsertRows MUST wrap the insert so the ListView
        // knows a row was added.
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
        m_items.append({ uniqueId, lat, longi, comment });
        endInsertRows();
    }
}

// Removes the row with this ID (nothing happens if it is not found).
void Model::removeById(unsigned int uniqueId)
{
    int row = indexOfId(uniqueId);
    if (row < 0)
        return;

    beginRemoveRows(QModelIndex(), row, row);   // same idea as beginInsertRows
    m_items.removeAt(row);
    endRemoveRows();
}

// Used by QML: returns one row as a map {uniqueId, lat, longi, comment}.
// Returns an empty map if the row number is invalid.
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