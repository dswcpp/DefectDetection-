interface StatusBarProps {
  detectionSpeed: number;
  totalCount: number;
  okCount: number;
  ngCount: number;
}

export function StatusBar({ detectionSpeed, totalCount, okCount, ngCount }: StatusBarProps) {
  const passRate = totalCount > 0 ? ((okCount / totalCount) * 100).toFixed(1) : '0.0';

  return (
    <div className="bg-gray-800 text-white px-4 py-2 border-t border-gray-700">
      <div className="flex gap-8 text-sm">
        <span>检测速度: <span className="text-green-400">{detectionSpeed}ms</span></span>
        <span>总数: <span className="text-blue-400">{totalCount}</span></span>
        <span>OK: <span className="text-green-400">{okCount}</span></span>
        <span>NG: <span className="text-red-400">{ngCount}</span></span>
        <span>良率: <span className="text-yellow-400">{passRate}%</span></span>
      </div>
    </div>
  );
}
