import { CheckCircle, XCircle } from 'lucide-react';

interface DetectionResult {
  status: 'OK' | 'NG';
  defects: Array<{
    type: string;
    confidence: number;
  }>;
  timestamp: string;
}

interface ResultsPanelProps {
  result: DetectionResult;
}

export function ResultsPanel({ result }: ResultsPanelProps) {
  return (
    <div className="bg-white border-b border-gray-300 p-4">
      <h3 className="mb-3">检测结果</h3>
      
      <div className="mb-4">
        {result.status === 'OK' ? (
          <div className="flex items-center gap-2 text-green-600">
            <CheckCircle className="w-8 h-8" />
            <span className="text-2xl">OK</span>
          </div>
        ) : (
          <div className="flex items-center gap-2 text-red-600">
            <XCircle className="w-8 h-8" />
            <span className="text-2xl">NG</span>
          </div>
        )}
      </div>
      
      {result.defects.length > 0 && (
        <div className="space-y-2">
          <div className="text-sm text-gray-600">缺陷列表:</div>
          {result.defects.map((defect, index) => (
            <div
              key={index}
              className="bg-red-50 border border-red-200 rounded p-2 text-sm"
            >
              <div className="flex justify-between items-center">
                <span className="text-red-800">{defect.type}</span>
                <span className="text-red-600">
                  {(defect.confidence * 100).toFixed(1)}%
                </span>
              </div>
            </div>
          ))}
        </div>
      )}
      
      <div className="mt-4 text-xs text-gray-500">
        {result.timestamp}
      </div>
    </div>
  );
}
