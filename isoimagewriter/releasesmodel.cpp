#include "releasesmodel.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>

ReleasesModel::ReleasesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    loadReleases();
}

int ReleasesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_releases.count();
}

QVariant ReleasesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_releases.count())
        return QVariant();

    const Release &release = m_releases[index.row()];

    switch (role) {
    case NameRole:
        return release.name;
    case VersionRole:
        return release.version;
    case EditionRole:
        return release.edition;
    case DescriptionRole:
        return release.description;
    case UrlRole:
        return release.url;
    case HashRole:
        return release.hash;
    case HashAlgoRole:
        return release.hashAlgo;
    }

    return QVariant();
}

QHash<int, QByteArray> ReleasesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[VersionRole] = "version";
    roles[EditionRole] = "edition";
    roles[DescriptionRole] = "description";
    roles[UrlRole] = "url";
    roles[HashRole] = "hash";
    roles[HashAlgoRole] = "hashAlgo";
    return roles;
}

void ReleasesModel::loadReleases()
{
    QFile file(":/releases.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open releases.json";
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON format in releases.json";
        return;
    }

    beginResetModel();
    m_releases.clear();
    parseJsonData(doc.object());
    endResetModel();
}

void ReleasesModel::parseJsonData(const QJsonObject &json)
{
    for (auto it = json.begin(); it != json.end(); ++it) {
        QJsonObject releaseObj = it.value().toObject();
        
        Release release;
        release.name = releaseObj["name"].toString();
        release.version = releaseObj["version"].toString();
        release.edition = releaseObj["edition"].toString();
        release.description = releaseObj["description"].toString();
        release.url = releaseObj["url"].toString();
        release.hash = releaseObj["hash"].toString();
        release.hashAlgo = releaseObj["hash_algo"].toString();
        
        m_releases.append(release);
    }
}

Release ReleasesModel::getReleaseAt(int index) const
{
    if (index >= 0 && index < m_releases.count()) {
        return m_releases[index];
    }
    return Release();
}

// ReleasesFilterModel implementation
ReleasesFilterModel::ReleasesFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(new ReleasesModel(this));
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

QString ReleasesFilterModel::filterText() const
{
    return m_filterText;
}

void ReleasesFilterModel::setFilterText(const QString &text)
{
    if (m_filterText != text) {
        m_filterText = text;
        setFilterFixedString(text);
        emit filterTextChanged();
    }
}

bool ReleasesFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_filterText.isEmpty()) {
        return true;
    }
    
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    QString name = sourceModel()->data(index, ReleasesModel::NameRole).toString();
    QString description = sourceModel()->data(index, ReleasesModel::DescriptionRole).toString();
    
    return name.contains(m_filterText, Qt::CaseInsensitive) || 
           description.contains(m_filterText, Qt::CaseInsensitive);
}

Release ReleasesFilterModel::getReleaseAt(int index) const
{
    QModelIndex sourceIndex = mapToSource(this->index(index, 0));
    ReleasesModel *sourceModel = qobject_cast<ReleasesModel*>(this->sourceModel());
    if (sourceModel) {
        return sourceModel->getReleaseAt(sourceIndex.row());
    }
    return Release();
}