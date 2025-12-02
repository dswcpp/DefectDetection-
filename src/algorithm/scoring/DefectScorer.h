#ifndef DEFECTSCORER_H
#define DEFECTSCORER_H

#include "../algorithm_global.h"
#include "../IDefectDetector.h"
#include <QString>
#include <QVariantMap>
#include <vector>
#include <map>

// 严重度等级
enum class SeverityGrade {
  OK = 0,       // 无缺陷
  Minor = 1,    // 轻微
  Major = 2,    // 严重
  Critical = 3  // 致命
};

// 评分结果
struct ALGORITHM_LIBRARY ScoringResult {
  double totalScore = 0.0;        // 总分 [0-100]
  SeverityGrade grade = SeverityGrade::OK;
  bool isPass = true;             // 是否通过
  QString gradeText;              // 等级文本
  std::map<QString, double> categoryScores;  // 各类别得分
  QString summary;                // 评分摘要
};

// 缺陷评分器
class ALGORITHM_LIBRARY DefectScorer {
public:
  DefectScorer();
  ~DefectScorer() = default;

  // 设置评分权重
  void setWeight(const QString& defectType, double weight);
  double weight(const QString& defectType) const;

  // 设置等级阈值
  void setThresholds(double minor, double major, double critical);

  // 设置合格阈值
  void setPassThreshold(double threshold);
  double passThreshold() const { return m_passThreshold; }

  // 计算总分
  ScoringResult score(const std::vector<DefectInfo>& defects);

  // 计算单个缺陷的扣分
  double scoreDefect(const DefectInfo& defect);

  // 获取等级文本
  static QString gradeToText(SeverityGrade grade);
  static SeverityGrade textToGrade(const QString& text);

private:
  std::map<QString, double> m_weights;
  double m_passThreshold = 60.0;
  double m_minorThreshold = 80.0;
  double m_majorThreshold = 50.0;
  double m_criticalThreshold = 20.0;

  void initDefaultWeights();
};

#endif // DEFECTSCORER_H
