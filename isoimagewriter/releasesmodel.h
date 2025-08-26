#ifndef RELEASESMODEL_H
#define RELEASESMODEL_H

#include <QAbstractListModel>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QObject>
#include <QSortFilterProxyModel>

struct Release {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString version MEMBER version)
    Q_PROPERTY(QString edition MEMBER edition)
    Q_PROPERTY(QString description MEMBER description)
    Q_PROPERTY(QString url MEMBER url)
    Q_PROPERTY(QString hash MEMBER hash)
    Q_PROPERTY(QString hashAlgo MEMBER hashAlgo)

public:
    QString name;
    QString version;
    QString edition;
    QString description;
    QString url;
    QString hash;
    QString hashAlgo;
};

class ReleasesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ReleaseRoles {
        NameRole = Qt::UserRole + 1,
        VersionRole,
        EditionRole,
        DescriptionRole,
        UrlRole,
        HashRole,
        HashAlgoRole
    };

    explicit ReleasesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadReleases();
    Q_INVOKABLE Release getReleaseAt(int index) const;

private:
    QList<Release> m_releases;
    void parseJsonData(const QJsonObject &json);
};

class ReleasesFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

public:
    explicit ReleasesFilterModel(QObject *parent = nullptr);
    
    QString filterText() const;
    void setFilterText(const QString &text);
    
    Q_INVOKABLE Release getReleaseAt(int index) const;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

signals:
    void filterTextChanged();

private:
    QString m_filterText;
};

#endif // RELEASESMODEL_H