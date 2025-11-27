import { Play, Square, Camera, Settings, BarChart3 } from 'lucide-react';

interface ToolbarProps {
  isRunning: boolean;
  onStart: () => void;
  onStop: () => void;
  onCapture: () => void;
  onSettings: () => void;
  onStats: () => void;
}

export function Toolbar({ isRunning, onStart, onStop, onCapture, onSettings, onStats }: ToolbarProps) {
  return (
    <div className="bg-gray-100 border-b border-gray-300 px-4 py-3">
      <div className="flex gap-3">
        <button
          onClick={onStart}
          disabled={isRunning}
          className="flex items-center gap-2 px-4 py-2 bg-green-600 text-white rounded hover:bg-green-700 disabled:bg-gray-400 disabled:cursor-not-allowed transition-colors"
        >
          <Play className="w-4 h-4" />
          启动
        </button>
        <button
          onClick={onStop}
          disabled={!isRunning}
          className="flex items-center gap-2 px-4 py-2 bg-red-600 text-white rounded hover:bg-red-700 disabled:bg-gray-400 disabled:cursor-not-allowed transition-colors"
        >
          <Square className="w-4 h-4" />
          停止
        </button>
        <button
          onClick={onCapture}
          className="flex items-center gap-2 px-4 py-2 bg-blue-600 text-white rounded hover:bg-blue-700 transition-colors"
        >
          <Camera className="w-4 h-4" />
          单拍
        </button>
        <button
          onClick={onSettings}
          className="flex items-center gap-2 px-4 py-2 bg-gray-600 text-white rounded hover:bg-gray-700 transition-colors"
        >
          <Settings className="w-4 h-4" />
          参数
        </button>
        <button
          onClick={onStats}
          className="flex items-center gap-2 px-4 py-2 bg-purple-600 text-white rounded hover:bg-purple-700 transition-colors"
        >
          <BarChart3 className="w-4 h-4" />
          统计
        </button>
      </div>
    </div>
  );
}
