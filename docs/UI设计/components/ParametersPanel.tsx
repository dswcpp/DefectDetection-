import { ChevronDown, ChevronRight } from 'lucide-react';
import { useState } from 'react';

export function ParametersPanel() {
  const [expanded, setExpanded] = useState({
    scratch: true,
    crack: false,
    foreign: false,
    dimension: false,
  });

  const toggleSection = (key: keyof typeof expanded) => {
    setExpanded((prev) => ({ ...prev, [key]: !prev[key] }));
  };

  return (
    <div className="bg-white p-4 overflow-y-auto">
      <h3 className="mb-3">参数面板</h3>
      
      {/* Scratch Detection */}
      <div className="mb-3">
        <button
          onClick={() => toggleSection('scratch')}
          className="flex items-center gap-2 w-full text-left py-2 hover:bg-gray-50 rounded px-2"
        >
          {expanded.scratch ? (
            <ChevronDown className="w-4 h-4" />
          ) : (
            <ChevronRight className="w-4 h-4" />
          )}
          <span>划痕检测</span>
        </button>
        {expanded.scratch && (
          <div className="pl-6 pr-2 py-2 space-y-3">
            <div>
              <label className="block text-sm text-gray-600 mb-1">灵敏度</label>
              <input
                type="range"
                min="0"
                max="100"
                defaultValue="75"
                className="w-full"
              />
              <div className="text-xs text-gray-500 text-right">75</div>
            </div>
            <div>
              <label className="block text-sm text-gray-600 mb-1">最小长度</label>
              <input
                type="number"
                defaultValue="10"
                className="w-full border border-gray-300 rounded px-2 py-1 text-sm"
              />
            </div>
          </div>
        )}
      </div>

      {/* Crack Detection */}
      <div className="mb-3">
        <button
          onClick={() => toggleSection('crack')}
          className="flex items-center gap-2 w-full text-left py-2 hover:bg-gray-50 rounded px-2"
        >
          {expanded.crack ? (
            <ChevronDown className="w-4 h-4" />
          ) : (
            <ChevronRight className="w-4 h-4" />
          )}
          <span>裂纹检测</span>
        </button>
        {expanded.crack && (
          <div className="pl-6 pr-2 py-2 space-y-3">
            <div>
              <label className="block text-sm text-gray-600 mb-1">阈值</label>
              <input
                type="range"
                min="0"
                max="100"
                defaultValue="80"
                className="w-full"
              />
              <div className="text-xs text-gray-500 text-right">80</div>
            </div>
          </div>
        )}
      </div>

      {/* Foreign Material Detection */}
      <div className="mb-3">
        <button
          onClick={() => toggleSection('foreign')}
          className="flex items-center gap-2 w-full text-left py-2 hover:bg-gray-50 rounded px-2"
        >
          {expanded.foreign ? (
            <ChevronDown className="w-4 h-4" />
          ) : (
            <ChevronRight className="w-4 h-4" />
          )}
          <span>异物检测</span>
        </button>
        {expanded.foreign && (
          <div className="pl-6 pr-2 py-2 space-y-3">
            <div>
              <label className="block text-sm text-gray-600 mb-1">最小面积</label>
              <input
                type="number"
                defaultValue="5"
                className="w-full border border-gray-300 rounded px-2 py-1 text-sm"
              />
            </div>
          </div>
        )}
      </div>

      {/* Dimension Measurement */}
      <div className="mb-3">
        <button
          onClick={() => toggleSection('dimension')}
          className="flex items-center gap-2 w-full text-left py-2 hover:bg-gray-50 rounded px-2"
        >
          {expanded.dimension ? (
            <ChevronDown className="w-4 h-4" />
          ) : (
            <ChevronRight className="w-4 h-4" />
          )}
          <span>尺寸测量</span>
        </button>
        {expanded.dimension && (
          <div className="pl-6 pr-2 py-2 space-y-3">
            <div>
              <label className="block text-sm text-gray-600 mb-1">公差 (mm)</label>
              <input
                type="number"
                defaultValue="0.5"
                step="0.1"
                className="w-full border border-gray-300 rounded px-2 py-1 text-sm"
              />
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
