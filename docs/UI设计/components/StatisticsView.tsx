import { useState } from 'react';
import { X, Search, Download, ChevronLeft, ChevronRight } from 'lucide-react';
import { ImageWithFallback } from './figma/ImageWithFallback';

interface DetectionRecord {
  id: string;
  timestamp: string;
  productId: string;
  result: 'OK' | 'NG';
  defectType: string;
  severity: '轻微' | '中等' | '严重' | '-';
  imageUrl: string;
  details: {
    location: string;
    confidence: number;
    size: string;
    operator: string;
  };
}

interface StatisticsViewProps {
  onClose: () => void;
}

export function StatisticsView({ onClose }: StatisticsViewProps) {
  const [selectedRecord, setSelectedRecord] = useState<DetectionRecord | null>(null);
  const [currentPage, setCurrentPage] = useState(1);
  const recordsPerPage = 15;

  // Mock data
  const mockRecords: DetectionRecord[] = Array.from({ length: 50 }, (_, i) => {
    const isNG = Math.random() > 0.7;
    const defectTypes = ['划痕', '裂纹', '异物', '尺寸偏差'];
    const severities: Array<'轻微' | '中等' | '严重' | '-'> = ['轻微', '中等', '严重', '-'];
    
    return {
      id: `REC${String(i + 1).padStart(6, '0')}`,
      timestamp: new Date(Date.now() - Math.random() * 7 * 24 * 60 * 60 * 1000).toLocaleString('zh-CN'),
      productId: `PRD${String(Math.floor(Math.random() * 10000)).padStart(6, '0')}`,
      result: isNG ? 'NG' : 'OK',
      defectType: isNG ? defectTypes[Math.floor(Math.random() * defectTypes.length)] : '-',
      severity: isNG ? severities[Math.floor(Math.random() * 3)] : '-',
      imageUrl: 'https://images.unsplash.com/photo-1707852710487-8f7837975208?crop=entropy&cs=tinysrgb&fit=max&fm=jpg&ixid=M3w3Nzg4Nzd8MHwxfHNlYXJjaHwxfHxpbmR1c3RyaWFsJTIwbWFudWZhY3R1cmluZyUyMHF1YWxpdHl8ZW58MXx8fHwxNzY0MjA5MDg5fDA&ixlib=rb-4.1.0&q=80&w=1080',
      details: {
        location: `(${Math.floor(Math.random() * 1000)}, ${Math.floor(Math.random() * 1000)})`,
        confidence: 0.75 + Math.random() * 0.24,
        size: `${Math.floor(Math.random() * 50 + 10)}px`,
        operator: 'admin',
      },
    };
  });

  const [records] = useState(mockRecords);
  const [filteredRecords, setFilteredRecords] = useState(mockRecords);

  const totalPages = Math.ceil(filteredRecords.length / recordsPerPage);
  const startIndex = (currentPage - 1) * recordsPerPage;
  const currentRecords = filteredRecords.slice(startIndex, startIndex + recordsPerPage);

  const handleSearch = () => {
    // In a real application, this would filter based on form inputs
    alert('搜索功能');
  };

  const handleExport = () => {
    alert('导出数据到 Excel');
  };

  return (
    <div className="fixed inset-0 bg-white z-50 flex flex-col">
      {/* Header */}
      <div className="flex items-center justify-between px-6 py-4 border-b border-gray-200 bg-gray-50">
        <h2>检测记录统计</h2>
        <button
          onClick={onClose}
          className="p-2 hover:bg-gray-200 rounded-full transition-colors"
        >
          <X className="w-5 h-5" />
        </button>
      </div>

      {/* Filter Section */}
      <div className="px-6 py-4 border-b border-gray-200 bg-white">
        <div className="grid grid-cols-8 gap-3">
          {/* Start Date */}
          <div>
            <label className="block text-xs text-gray-600 mb-1">开始日期</label>
            <input
              type="date"
              className="w-full border border-gray-300 rounded px-2 py-1.5 text-sm"
            />
          </div>

          {/* End Date */}
          <div>
            <label className="block text-xs text-gray-600 mb-1">结束日期</label>
            <input
              type="date"
              className="w-full border border-gray-300 rounded px-2 py-1.5 text-sm"
            />
          </div>

          {/* Result Filter */}
          <div>
            <label className="block text-xs text-gray-600 mb-1">结果</label>
            <select className="w-full border border-gray-300 rounded px-2 py-1.5 text-sm">
              <option value="">全部</option>
              <option value="OK">OK</option>
              <option value="NG">NG</option>
            </select>
          </div>

          {/* Defect Type Filter */}
          <div>
            <label className="block text-xs text-gray-600 mb-1">缺陷类型</label>
            <select className="w-full border border-gray-300 rounded px-2 py-1.5 text-sm">
              <option value="">全部</option>
              <option value="scratch">划痕</option>
              <option value="crack">裂纹</option>
              <option value="foreign">异物</option>
              <option value="dimension">尺寸偏差</option>
            </select>
          </div>

          {/* Severity Filter */}
          <div>
            <label className="block text-xs text-gray-600 mb-1">严重度</label>
            <select className="w-full border border-gray-300 rounded px-2 py-1.5 text-sm">
              <option value="">全部</option>
              <option value="minor">轻微</option>
              <option value="moderate">中等</option>
              <option value="severe">严重</option>
            </select>
          </div>

          {/* Keyword Search */}
          <div className="col-span-2">
            <label className="block text-xs text-gray-600 mb-1">关键词</label>
            <input
              type="text"
              placeholder="产品ID、操作员..."
              className="w-full border border-gray-300 rounded px-2 py-1.5 text-sm"
            />
          </div>

          {/* Action Buttons */}
          <div className="flex gap-2 items-end">
            <button
              onClick={handleSearch}
              className="flex items-center gap-1 px-3 py-1.5 bg-blue-600 text-white rounded hover:bg-blue-700 transition-colors text-sm"
            >
              <Search className="w-4 h-4" />
              搜索
            </button>
            <button
              onClick={handleExport}
              className="flex items-center gap-1 px-3 py-1.5 bg-green-600 text-white rounded hover:bg-green-700 transition-colors text-sm"
            >
              <Download className="w-4 h-4" />
              导出
            </button>
          </div>
        </div>
      </div>

      {/* Main Content */}
      <div className="flex-1 flex overflow-hidden">
        {/* Table Section */}
        <div className="flex-1 flex flex-col border-r border-gray-200">
          {/* Table */}
          <div className="flex-1 overflow-auto">
            <table className="w-full">
              <thead className="bg-gray-50 sticky top-0">
                <tr className="border-b border-gray-200">
                  <th className="px-4 py-3 text-left text-sm text-gray-600">记录ID</th>
                  <th className="px-4 py-3 text-left text-sm text-gray-600">时间</th>
                  <th className="px-4 py-3 text-left text-sm text-gray-600">产品ID</th>
                  <th className="px-4 py-3 text-left text-sm text-gray-600">结果</th>
                  <th className="px-4 py-3 text-left text-sm text-gray-600">缺陷类型</th>
                  <th className="px-4 py-3 text-left text-sm text-gray-600">严重度</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-gray-200">
                {currentRecords.map((record) => (
                  <tr
                    key={record.id}
                    onClick={() => setSelectedRecord(record)}
                    className={`hover:bg-blue-50 cursor-pointer transition-colors ${
                      selectedRecord?.id === record.id ? 'bg-blue-100' : ''
                    }`}
                  >
                    <td className="px-4 py-3 text-sm">{record.id}</td>
                    <td className="px-4 py-3 text-sm">{record.timestamp}</td>
                    <td className="px-4 py-3 text-sm">{record.productId}</td>
                    <td className="px-4 py-3">
                      <span
                        className={`inline-flex px-2 py-1 rounded text-xs ${
                          record.result === 'OK'
                            ? 'bg-green-100 text-green-700'
                            : 'bg-red-100 text-red-700'
                        }`}
                      >
                        {record.result}
                      </span>
                    </td>
                    <td className="px-4 py-3 text-sm">{record.defectType}</td>
                    <td className="px-4 py-3">
                      {record.severity !== '-' && (
                        <span
                          className={`inline-flex px-2 py-1 rounded text-xs ${
                            record.severity === '轻微'
                              ? 'bg-yellow-100 text-yellow-700'
                              : record.severity === '中等'
                              ? 'bg-orange-100 text-orange-700'
                              : 'bg-red-100 text-red-700'
                          }`}
                        >
                          {record.severity}
                        </span>
                      )}
                      {record.severity === '-' && <span className="text-gray-400">-</span>}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>

          {/* Pagination */}
          <div className="border-t border-gray-200 px-6 py-3 bg-gray-50 flex items-center justify-between">
            <div className="text-sm text-gray-600">
              显示 {startIndex + 1} - {Math.min(startIndex + recordsPerPage, filteredRecords.length)} 条，
              共 {filteredRecords.length} 条记录
            </div>
            <div className="flex items-center gap-2">
              <button
                onClick={() => setCurrentPage((p) => Math.max(1, p - 1))}
                disabled={currentPage === 1}
                className="p-1 border border-gray-300 rounded hover:bg-gray-100 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
              >
                <ChevronLeft className="w-4 h-4" />
              </button>
              <span className="text-sm px-3">
                {currentPage} / {totalPages}
              </span>
              <button
                onClick={() => setCurrentPage((p) => Math.min(totalPages, p + 1))}
                disabled={currentPage === totalPages}
                className="p-1 border border-gray-300 rounded hover:bg-gray-100 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
              >
                <ChevronRight className="w-4 h-4" />
              </button>
            </div>
          </div>
        </div>

        {/* Detail Panel */}
        <div className="w-96 bg-gray-50 flex flex-col overflow-hidden">
          {selectedRecord ? (
            <>
              {/* Image Preview */}
              <div className="p-4 border-b border-gray-200 bg-white">
                <h4 className="mb-3 text-gray-700">图像预览</h4>
                <div className="bg-gray-900 rounded overflow-hidden">
                  <ImageWithFallback
                    src={selectedRecord.imageUrl}
                    alt="检测图像"
                    className="w-full h-64 object-cover"
                  />
                </div>
              </div>

              {/* Detailed Information */}
              <div className="flex-1 p-4 overflow-y-auto">
                <h4 className="mb-3 text-gray-700">详细信息</h4>
                <div className="space-y-3">
                  <div className="bg-white p-3 rounded border border-gray-200">
                    <div className="text-xs text-gray-500 mb-1">记录ID</div>
                    <div className="text-sm">{selectedRecord.id}</div>
                  </div>

                  <div className="bg-white p-3 rounded border border-gray-200">
                    <div className="text-xs text-gray-500 mb-1">产品ID</div>
                    <div className="text-sm">{selectedRecord.productId}</div>
                  </div>

                  <div className="bg-white p-3 rounded border border-gray-200">
                    <div className="text-xs text-gray-500 mb-1">检测时间</div>
                    <div className="text-sm">{selectedRecord.timestamp}</div>
                  </div>

                  <div className="bg-white p-3 rounded border border-gray-200">
                    <div className="text-xs text-gray-500 mb-1">检测结果</div>
                    <div>
                      <span
                        className={`inline-flex px-2 py-1 rounded text-sm ${
                          selectedRecord.result === 'OK'
                            ? 'bg-green-100 text-green-700'
                            : 'bg-red-100 text-red-700'
                        }`}
                      >
                        {selectedRecord.result}
                      </span>
                    </div>
                  </div>

                  {selectedRecord.result === 'NG' && (
                    <>
                      <div className="bg-white p-3 rounded border border-gray-200">
                        <div className="text-xs text-gray-500 mb-1">缺陷类型</div>
                        <div className="text-sm">{selectedRecord.defectType}</div>
                      </div>

                      <div className="bg-white p-3 rounded border border-gray-200">
                        <div className="text-xs text-gray-500 mb-1">严重度</div>
                        <div>
                          <span
                            className={`inline-flex px-2 py-1 rounded text-sm ${
                              selectedRecord.severity === '轻微'
                                ? 'bg-yellow-100 text-yellow-700'
                                : selectedRecord.severity === '中等'
                                ? 'bg-orange-100 text-orange-700'
                                : 'bg-red-100 text-red-700'
                            }`}
                          >
                            {selectedRecord.severity}
                          </span>
                        </div>
                      </div>

                      <div className="bg-white p-3 rounded border border-gray-200">
                        <div className="text-xs text-gray-500 mb-1">缺陷位置</div>
                        <div className="text-sm">{selectedRecord.details.location}</div>
                      </div>

                      <div className="bg-white p-3 rounded border border-gray-200">
                        <div className="text-xs text-gray-500 mb-1">置信度</div>
                        <div className="text-sm">
                          {(selectedRecord.details.confidence * 100).toFixed(1)}%
                        </div>
                      </div>

                      <div className="bg-white p-3 rounded border border-gray-200">
                        <div className="text-xs text-gray-500 mb-1">缺陷大小</div>
                        <div className="text-sm">{selectedRecord.details.size}</div>
                      </div>
                    </>
                  )}

                  <div className="bg-white p-3 rounded border border-gray-200">
                    <div className="text-xs text-gray-500 mb-1">操作员</div>
                    <div className="text-sm">{selectedRecord.details.operator}</div>
                  </div>
                </div>
              </div>
            </>
          ) : (
            <div className="flex-1 flex items-center justify-center text-gray-400">
              <div className="text-center">
                <div className="mb-2">请选择一条记录</div>
                <div className="text-sm">以查看详细信息</div>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
