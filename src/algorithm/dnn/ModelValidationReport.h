#ifndef MODELVALIDATIONREPORT_H
#define MODELVALIDATIONREPORT_H
#include <QString>
#include <QStringList>
#include "opencv2/opencv.hpp"
struct ModelValidationReport {

  bool ok = false;

  QStringList errors;

  QStringList warnings;

  double warmupMs = 0.0;

  double singleRunMs = 0.0;

  QString backend;

  QString target;

  int opset = -1;

  cv::Size inputSize{640, 640};

  int inputChannels = 3;

  int numClasses = -1;

};

#endif // MODELVALIDATIONREPORT_H
