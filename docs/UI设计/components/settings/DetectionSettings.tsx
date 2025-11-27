export function DetectionSettings() {
  return (
    <div className="space-y-6">
      {/* Global Settings */}
      <div>
        <h4 className="mb-4 text-gray-700">全局设置</h4>
        <div className="grid grid-cols-2 gap-6">
          <div>
            <label className="block text-sm text-gray-600 mb-2">检测模式</label>
            <select className="w-full border border-gray-300 rounded px-3 py-2">
              <option>标准模式</option>
              <option>高速模式</option>
              <option>高精度模式</option>
            </select>
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">置信度阈值</label>
            <div className="flex items-center gap-4">
              <input
                type="range"
                min="0"
                max="100"
                defaultValue="75"
                className="flex-1"
              />
              <span className="text-sm w-12">75%</span>
            </div>
          </div>

          <div className="flex items-center gap-3">
            <input type="checkbox" id="enableScratch" className="w-4 h-4" defaultChecked />
            <label htmlFor="enableScratch" className="text-sm">启用划痕检测</label>
          </div>

          <div className="flex items-center gap-3">
            <input type="checkbox" id="enableCrack" className="w-4 h-4" defaultChecked />
            <label htmlFor="enableCrack" className="text-sm">启用裂纹检测</label>
          </div>

          <div className="flex items-center gap-3">
            <input type="checkbox" id="enableForeign" className="w-4 h-4" defaultChecked />
            <label htmlFor="enableForeign" className="text-sm">启用异物检测</label>
          </div>

          <div className="flex items-center gap-3">
            <input type="checkbox" id="enableDimension" className="w-4 h-4" defaultChecked />
            <label htmlFor="enableDimension" className="text-sm">启用尺寸测量</label>
          </div>
        </div>
      </div>

      {/* Scratch Detection Parameters */}
      <div className="border-t border-gray-200 pt-6">
        <h4 className="mb-4 text-gray-700">划痕检测参数</h4>
        <div className="grid grid-cols-2 gap-6">
          <div>
            <label className="block text-sm text-gray-600 mb-2">最小长度 (像素)</label>
            <input
              type="number"
              defaultValue="50"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">最大宽度 (像素)</label>
            <input
              type="number"
              defaultValue="5"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">灵敏度</label>
            <div className="flex items-center gap-4">
              <input
                type="range"
                min="0"
                max="100"
                defaultValue="80"
                className="flex-1"
              />
              <span className="text-sm w-12">80</span>
            </div>
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">对比度阈值</label>
            <input
              type="number"
              defaultValue="30"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>
        </div>
      </div>

      {/* Crack Detection Parameters */}
      <div className="border-t border-gray-200 pt-6">
        <h4 className="mb-4 text-gray-700">裂纹检测参数</h4>
        <div className="grid grid-cols-2 gap-6">
          <div>
            <label className="block text-sm text-gray-600 mb-2">最小面积 (像素²)</label>
            <input
              type="number"
              defaultValue="100"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">二值化阈值</label>
            <input
              type="number"
              defaultValue="128"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">形态学核大小</label>
            <input
              type="number"
              defaultValue="3"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>
        </div>
      </div>

      {/* Foreign Material Detection Parameters */}
      <div className="border-t border-gray-200 pt-6">
        <h4 className="mb-4 text-gray-700">异物检测参数</h4>
        <div className="grid grid-cols-2 gap-6">
          <div>
            <label className="block text-sm text-gray-600 mb-2">最小尺寸 (像素)</label>
            <input
              type="number"
              defaultValue="10"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">颜色差异阈值</label>
            <input
              type="number"
              defaultValue="40"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>
        </div>
      </div>

      {/* Dimension Measurement Parameters */}
      <div className="border-t border-gray-200 pt-6">
        <h4 className="mb-4 text-gray-700">尺寸测量参数</h4>
        <div className="grid grid-cols-2 gap-6">
          <div>
            <label className="block text-sm text-gray-600 mb-2">标准尺寸 (mm)</label>
            <input
              type="number"
              defaultValue="100.00"
              step="0.01"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">允许公差 (mm)</label>
            <input
              type="number"
              defaultValue="0.50"
              step="0.01"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">像素比例 (mm/pixel)</label>
            <input
              type="number"
              defaultValue="0.05"
              step="0.001"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">边缘检测方法</label>
            <select className="w-full border border-gray-300 rounded px-3 py-2">
              <option>Canny</option>
              <option>Sobel</option>
              <option>Laplacian</option>
            </select>
          </div>
        </div>
      </div>
    </div>
  );
}
