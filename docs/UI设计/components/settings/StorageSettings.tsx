export function StorageSettings() {
  return (
    <div className="space-y-6">
      <div className="grid grid-cols-2 gap-6">
        {/* Storage Path */}
        <div className="col-span-2">
          <label className="block text-sm text-gray-600 mb-2">存储路径</label>
          <div className="flex gap-2">
            <input
              type="text"
              defaultValue="D:\InspectionData"
              className="flex-1 border border-gray-300 rounded px-3 py-2"
            />
            <button className="px-4 py-2 border border-gray-300 rounded hover:bg-gray-100 transition-colors">
              浏览...
            </button>
          </div>
        </div>

        {/* Save Original Images */}
        <div className="flex items-center gap-3">
          <input type="checkbox" id="saveOriginal" className="w-4 h-4" defaultChecked />
          <label htmlFor="saveOriginal" className="text-sm">保存原始图像</label>
        </div>

        {/* Save Annotated Images */}
        <div className="flex items-center gap-3">
          <input type="checkbox" id="saveAnnotated" className="w-4 h-4" defaultChecked />
          <label htmlFor="saveAnnotated" className="text-sm">保存标注图像</label>
        </div>

        {/* Save Only NG */}
        <div className="flex items-center gap-3">
          <input type="checkbox" id="saveOnlyNG" className="w-4 h-4" />
          <label htmlFor="saveOnlyNG" className="text-sm">仅保存 NG 产品</label>
        </div>

        {/* Generate Report */}
        <div className="flex items-center gap-3">
          <input type="checkbox" id="generateReport" className="w-4 h-4" defaultChecked />
          <label htmlFor="generateReport" className="text-sm">生成检测报告</label>
        </div>

        {/* Image Format */}
        <div>
          <label className="block text-sm text-gray-600 mb-2">图像格式</label>
          <select className="w-full border border-gray-300 rounded px-3 py-2">
            <option>JPEG (压缩)</option>
            <option>PNG (无损)</option>
            <option>BMP (原始)</option>
            <option>TIFF (高质量)</option>
          </select>
        </div>

        {/* Compression Quality */}
        <div>
          <label className="block text-sm text-gray-600 mb-2">压缩质量</label>
          <div className="flex items-center gap-4">
            <input
              type="range"
              min="0"
              max="100"
              defaultValue="85"
              className="flex-1"
            />
            <span className="text-sm w-12">85%</span>
          </div>
        </div>
      </div>

      {/* Auto Cleanup */}
      <div className="border-t border-gray-200 pt-6">
        <h4 className="mb-4 text-gray-700">自动清理</h4>
        <div className="grid grid-cols-2 gap-6">
          <div className="flex items-center gap-3">
            <input type="checkbox" id="autoCleanup" className="w-4 h-4" defaultChecked />
            <label htmlFor="autoCleanup" className="text-sm">启用自动清理</label>
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">保留天数</label>
            <input
              type="number"
              defaultValue="30"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>

          <div>
            <label className="block text-sm text-gray-600 mb-2">最小可用空间 (GB)</label>
            <input
              type="number"
              defaultValue="10"
              className="w-full border border-gray-300 rounded px-3 py-2"
            />
          </div>
        </div>
      </div>

      {/* Storage Info */}
      <div className="border-t border-gray-200 pt-6">
        <h4 className="mb-4 text-gray-700">存储信息</h4>
        <div className="grid grid-cols-2 gap-4">
          <div className="p-4 bg-blue-50 border border-blue-200 rounded">
            <div className="text-sm text-gray-600 mb-1">总容量</div>
            <div className="text-xl">500 GB</div>
          </div>
          <div className="p-4 bg-green-50 border border-green-200 rounded">
            <div className="text-sm text-gray-600 mb-1">可用空间</div>
            <div className="text-xl">125 GB</div>
          </div>
          <div className="p-4 bg-yellow-50 border border-yellow-200 rounded">
            <div className="text-sm text-gray-600 mb-1">已用空间</div>
            <div className="text-xl">375 GB (75%)</div>
          </div>
          <div className="p-4 bg-purple-50 border border-purple-200 rounded">
            <div className="text-sm text-gray-600 mb-1">图像数量</div>
            <div className="text-xl">12,456</div>
          </div>
        </div>
      </div>

      {/* Actions */}
      <div className="border-t border-gray-200 pt-6 flex gap-3">
        <button className="px-6 py-2 bg-blue-600 text-white rounded hover:bg-blue-700 transition-colors">
          打开存储目录
        </button>
        <button className="px-6 py-2 border border-gray-300 rounded hover:bg-gray-100 transition-colors">
          立即清理
        </button>
      </div>
    </div>
  );
}
