#ifndef ERRORCODE_H
#define ERRORCODE_H

enum class ErrorCode {
  Ok = 0,
  InvalidConfig,
  CameraNotFound,
  PlcDisconnected,
  DatabaseError,
  IoError,
  Unknown
};

#endif // ERRORCODE_H
