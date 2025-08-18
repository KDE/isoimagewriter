#ifndef RELEASESMODEL_H
#define RELEASESMODEL_H

#include <QAbstractListModel>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QObject>

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

#endif // RELEASESMODEL_H