#include "DefectScorer.h"

DefectScorer::DefectScorer() {
  initDefaultWeights();
}

void DefectScorer::initDefaultWeights() {
  // 默认权重：不同类型缺陷的扣分系数
  m_weights["Scratch"] = 1.0;    // 划痕
  m_weights["Crack"] = 2.0;      // 裂纹（更严重）
  m_weights["Foreign"] = 1.5;    // 异物
  m_weights["Dimension"] = 2.5;  // 尺寸超差（最严重）
}

void DefectScorer::setWeight(const QString& defectType, double weight) {
  m_weights[defectType] = std::max(0.0, weight);
}

double DefectScorer::weight(const QString& defectType) const {
  auto it = m_weights.find(defectType);
  return (it != m_weights.end()) ? it->second : 1.0;
}

void DefectScorer::setThresholds(double minor, double major, double critical) {
  m_minorThreshold = minor;
  m_majorThreshold = major;
  m_criticalThreshold = critical;
}

void DefectScorer::setPassThreshold(double threshold) {
  m_passThreshold = qBound(0.0, threshold, 100.0);
}

double DefectScorer::scoreDefect(const DefectInfo& defect) {
  // 基础扣分 = 严重度 * 权重 * 基础分值
  double baseDeduction = 10.0;  // 每个缺陷基础扣分
  
  double typeWeight = weight(defect.className);
  double severityFactor = defect.severity;
  double confidenceFactor = defect.confidence;
  
  // 面积因子（可选，基于缺陷大小）
  double areaFactor = 1.0;
  if (defect.attributes.contains("area")) {
    double area = defect.attributes["area"].toDouble();
    areaFactor = std::min(2.0, 1.0 + area / 1000.0);
  }
  
  return baseDeduction * typeWeight * severityFactor * confidenceFactor * areaFactor;
}

ScoringResult DefectScorer::score(const std::vector<DefectInfo>& defects) {
  ScoringResult result;
  result.totalScore = 100.0;  // 满分开始

  if (defects.empty()) {
    result.grade = SeverityGrade::OK;
    result.gradeText = gradeToText(result.grade);
    result.isPass = true;
    result.summary = "No defects detected";
    return result;
  }

  // 按类别统计
  std::map<QString, std::vector<const DefectInfo*>> byCategory;
  for (const auto& d : defects) {
    byCategory[d.className].push_back(&d);
  }

  // 计算各类别扣分
  double totalDeduction = 0.0;
  for (const auto& pair : byCategory) {
    double categoryDeduction = 0.0;
    for (const auto* defect : pair.second) {
      categoryDeduction += scoreDefect(*defect);
    }
    result.categoryScores[pair.first] = categoryDeduction;
    totalDeduction += categoryDeduction;
  }

  // 计算总分
  result.totalScore = std::max(0.0, 100.0 - totalDeduction);

  // 确定等级
  if (result.totalScore >= m_minorThreshold) {
    result.grade = SeverityGrade::OK;
  } else if (result.totalScore >= m_majorThreshold) {
    result.grade = SeverityGrade::Minor;
  } else if (result.totalScore >= m_criticalThreshold) {
    result.grade = SeverityGrade::Major;
  } else {
    result.grade = SeverityGrade::Critical;
  }

  result.gradeText = gradeToText(result.grade);
  result.isPass = result.totalScore >= m_passThreshold;

  // 生成摘要
  QStringList parts;
  parts << QString("Total Score: %1").arg(result.totalScore, 0, 'f', 1);
  parts << QString("Grade: %1").arg(result.gradeText);
  parts << QString("Defects: %1").arg(defects.size());
  
  for (const auto& pair : result.categoryScores) {
    parts << QString("%1: -%2 pts").arg(pair.first).arg(pair.second, 0, 'f', 1);
  }
  
  result.summary = parts.join(", ");

  return result;
}

QString DefectScorer::gradeToText(SeverityGrade grade) {
  switch (grade) {
    case SeverityGrade::OK: return "OK";
    case SeverityGrade::Minor: return "Minor";
    case SeverityGrade::Major: return "Major";
    case SeverityGrade::Critical: return "Critical";
    default: return "Unknown";
  }
}

SeverityGrade DefectScorer::textToGrade(const QString& text) {
  if (text == "OK") return SeverityGrade::OK;
  if (text == "Minor") return SeverityGrade::Minor;
  if (text == "Major") return SeverityGrade::Major;
  if (text == "Critical") return SeverityGrade::Critical;
  return SeverityGrade::OK;
}
