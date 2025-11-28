#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QString>

class ReportGenerator {
public:
  ReportGenerator() = default;

  // TODO: 生成日报/周报/月报
  bool generate(const QString &path);
};

#endif // REPORTGENERATOR_H
