# API 文档

## 1. REST API

### 1.1 基本信息

| 项目 | 说明 |
| --- | --- |
| **Base URL** | `http://{host}:8080/api/v1` |
| **协议** | HTTP/HTTPS |
| **数据格式** | JSON |
| **认证方式** | Bearer Token |

### 1.2 认证

```
POST /api/v1/auth/login
Content-Type: application/json

{
  "username": "admin",
  "password": "password123"
}
```

**响应**：

```json
{
  "code": 0,
  "data": {
    "token": "eyJhbGciOiJIUzI1NiIs...",
    "expires_in": 86400
  }
}
```

---

## 2. 检测接口

### 2.1 获取检测结果

```
GET /api/v1/inspection/{id}
Authorization: Bearer {token}
```

**响应**：

```json
{
  "code": 0,
  "data": {
    "id": "INS-20250115-001234",
    "timestamp": "2025-01-15T14:32:15.123Z",
    "result": "NG",
    "cycle_time_ms": 85,
    "defects": [
      {
        "type": "crack",
        "severity_score": 45,
        "severity_level": "moderate",
        "location": {"x": 150, "y": 200, "width": 30, "height": 15},
        "features": {
          "length": 28.5,
          "branch_count": 1
        }
      }
    ],
    "image_url": "/images/2025/01/15/001234.jpg"
  }
}
```

### 2.2 查询检测记录

```
GET /api/v1/inspections?start=2025-01-15&end=2025-01-16&result=NG&page=1&limit=20
Authorization: Bearer {token}
```

**响应**：

```json
{
  "code": 0,
  "data": {
    "total": 156,
    "page": 1,
    "limit": 20,
    "items": [
      {
        "id": "INS-20250115-001234",
        "timestamp": "2025-01-15T14:32:15.123Z",
        "result": "NG",
        "defect_types": ["crack"]
      }
    ]
  }
}
```

---

## 3. 统计接口

### 3.1 获取统计概览

```
GET /api/v1/statistics/overview?date=2025-01-15
Authorization: Bearer {token}
```

**响应**：

```json
{
  "code": 0,
  "data": {
    "date": "2025-01-15",
    "total": 12456,
    "ok": 12234,
    "ng": 222,
    "yield_rate": 0.982,
    "defect_distribution": {
      "scratch": 100,
      "crack": 49,
      "foreign": 40,
      "dimension": 33
    },
    "severity_distribution": {
      "minor": 151,
      "moderate": 56,
      "severe": 15
    }
  }
}
```

### 3.2 获取良率趋势

```
GET /api/v1/statistics/trend?start=2025-01-01&end=2025-01-15&granularity=day
Authorization: Bearer {token}
```

**响应**：

```json
{
  "code": 0,
  "data": [
    {"date": "2025-01-01", "yield_rate": 0.978},
    {"date": "2025-01-02", "yield_rate": 0.982},
    {"date": "2025-01-03", "yield_rate": 0.975}
  ]
}
```

---

## 4. 配置接口

### 4.1 获取检测器参数

```
GET /api/v1/config/detector/{name}
Authorization: Bearer {token}
```

**响应**：

```json
{
  "code": 0,
  "data": {
    "name": "scratch",
    "enabled": true,
    "params": {
      "canny_low": 50,
      "canny_high": 150,
      "min_length": 20
    }
  }
}
```

### 4.2 更新检测器参数

```
PUT /api/v1/config/detector/{name}
Authorization: Bearer {token}
Content-Type: application/json

{
  "enabled": true,
  "params": {
    "canny_low": 60,
    "canny_high": 160
  }
}
```

---

## 5. 系统接口

### 5.1 获取系统状态

```
GET /api/v1/system/status
Authorization: Bearer {token}
```

**响应**：

```json
{
  "code": 0,
  "data": {
    "status": "running",
    "uptime_seconds": 86400,
    "camera": {
      "connected": true,
      "fps": 10.2
    },
    "plc": {
      "connected": true,
      "last_heartbeat": "2025-01-15T14:32:15Z"
    },
    "cpu_usage": 45.2,
    "memory_usage": 62.5,
    "disk_usage": 35.8
  }
}
```

### 5.2 系统控制

```
POST /api/v1/system/control
Authorization: Bearer {token}
Content-Type: application/json

{
  "action": "start" // start | stop | restart
}
```

---

## 6. WebSocket 推送

### 6.1 连接

```jsx
const ws = new WebSocket('ws://{host}:8080/ws?token={token}');
```

### 6.2 检测结果推送

```json
{
  "type": "inspection_result",
  "data": {
    "id": "INS-20250115-001234",
    "result": "NG",
    "severity_score": 45
  }
}
```

### 6.3 系统告警推送

```json
{
  "type": "alarm",
  "data": {
    "level": "warning",
    "message": "相机连接异常",
    "timestamp": "2025-01-15T14:32:15Z"
  }
}
```

---

## 7. 错误码

| 错误码 | 说明 |
| --- | --- |
| 0 | 成功 |
| 1001 | 参数错误 |
| 1002 | 认证失败 |
| 1003 | 权限不足 |
| 2001 | 资源不存在 |
| 2002 | 资源冲突 |
| 3001 | 系统错误 |
| 3002 | 服务不可用 |