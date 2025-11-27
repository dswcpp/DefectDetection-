export function PLCSettings() {
  return (
    <div className="space-y-6">
      <div className="grid grid-cols-2 gap-6">
        {/* Communication Type */}
        <div>
          <label className="block text-sm text-gray-600 mb-2">通信方式</label>
          <select className="w-full border border-gray-300 rounded px-3 py-2">
            <option>Modbus TCP</option>
            <option>Modbus RTU</option>
            <option>Ethernet/IP</option>
            <option>Profinet</option>
          </select>
        </div>

        {/* Connection Status */}
        <div>
          <label className="block text-sm text-gray-600 mb-2">连接状态</label>
          <div className="flex items-center gap-2 px-3 py-2 bg-yellow-50 border border-yellow-300 rounded">
            <div className="w-2 h-2 bg-yellow-500 rounded-full"></div>
            <span className="text-yellow-700">未连接</span>
          </div>
        </div>

        {/* IP Address */}
        <div>
          <label className="block text-sm text-gray-600 mb-2">IP 地址</label>
          <input
            type="text"
            defaultValue="192.168.1.100"
            className="w-full border border-gray-300 rounded px-3 py-2"
          />
        </div>

        {/* Port */}
        <div>
          <label className="block text-sm text-gray-600 mb-2">端口号</label>
          <input
            type="number"
            defaultValue="502"
            className="w-full border border-gray-300 rounded px-3 py-2"
          />
        </div>

        {/* Timeout */}
        <div>
          <label className="block text-sm text-gray-600 mb-2">超时时间 (ms)</label>
          <input
            type="number"
            defaultValue="3000"
            className="w-full border border-gray-300 rounded px-3 py-2"
          />
        </div>

        {/* Retry Times */}
        <div>
          <label className="block text-sm text-gray-600 mb-2">重试次数</label>
          <input
            type="number"
            defaultValue="3"
            className="w-full border border-gray-300 rounded px-3 py-2"
          />
        </div>
      </div>

      {/* Signal Mapping */}
      <div className="border-t border-gray-200 pt-6">
        <h4 className="mb-4 text-gray-700">信号映射</h4>
        <div className="space-y-3">
          <div className="grid grid-cols-3 gap-4 p-3 bg-gray-50 rounded">
            <div className="text-sm text-gray-600">触发信号 (输入)</div>
            <div className="text-sm">寄存器地址</div>
            <input
              type="text"
              defaultValue="M100"
              className="border border-gray-300 rounded px-2 py-1 text-sm"
            />
          </div>
          <div className="grid grid-cols-3 gap-4 p-3 bg-gray-50 rounded">
            <div className="text-sm text-gray-600">检测完成 (输出)</div>
            <div className="text-sm">寄存器地址</div>
            <input
              type="text"
              defaultValue="M200"
              className="border border-gray-300 rounded px-2 py-1 text-sm"
            />
          </div>
          <div className="grid grid-cols-3 gap-4 p-3 bg-gray-50 rounded">
            <div className="text-sm text-gray-600">检测结果 OK (输出)</div>
            <div className="text-sm">寄存器地址</div>
            <input
              type="text"
              defaultValue="M201"
              className="border border-gray-300 rounded px-2 py-1 text-sm"
            />
          </div>
          <div className="grid grid-cols-3 gap-4 p-3 bg-gray-50 rounded">
            <div className="text-sm text-gray-600">检测结果 NG (输出)</div>
            <div className="text-sm">寄存器地址</div>
            <input
              type="text"
              defaultValue="M202"
              className="border border-gray-300 rounded px-2 py-1 text-sm"
            />
          </div>
        </div>
      </div>

      {/* Test Connection */}
      <div className="border-t border-gray-200 pt-6 flex gap-3">
        <button className="px-6 py-2 bg-blue-600 text-white rounded hover:bg-blue-700 transition-colors">
          测试连接
        </button>
        <button className="px-6 py-2 bg-green-600 text-white rounded hover:bg-green-700 transition-colors">
          连接 PLC
        </button>
        <button className="px-6 py-2 border border-gray-300 rounded hover:bg-gray-100 transition-colors">
          断开连接
        </button>
      </div>
    </div>
  );
}
