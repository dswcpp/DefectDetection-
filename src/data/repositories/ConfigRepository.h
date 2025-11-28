#ifndef CONFIGREPOSITORY_H
#define CONFIGREPOSITORY_H

#include "IRepository.h"
#include <QObject>
#include <QString>

class ConfigRepository : public QObject, public IRepository {
  Q_OBJECT
public:
  explicit ConfigRepository(QObject *parent = nullptr);

  QString name() const override { return "ConfigRepository"; }

  // TODO: 加载配置 JSON 到内存
  bool load(const QString &path);

  // TODO: 保存配置 JSON
  bool save(const QString &path) const;

signals:
  void configLoaded(const QString &path);
  void configSaved(const QString &path);
};

#endif // CONFIGREPOSITORY_H
