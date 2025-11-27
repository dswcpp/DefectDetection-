import { useState } from 'react';

export function LightSettings() {
  const [channels, setChannels] = useState([
    { name: '通道 1 (正面)', enabled: true, value: 200 },
    { name: '通道 2 (侧光)', enabled: true, value: 200 },
    { name: '通道 3 (背光)', enabled: false, value: 200 },
    { name: '通道 4 (备用)', enabled: false, value: 200 },
  ]);

  const [strobeEnabled, setStrobeEnabled] = useState(false);
  const [strobeDuration, setStrobeDuration] = useState(1000);

  const updateChannel = (index: number, field: 'enabled' | 'value', newValue: boolean | number) => {
    setChannels((prev) =>
      prev.map((ch, i) => (i === index ? { ...ch, [field]: newValue } : ch))
    );
  };

  const handleLightOn = () => {
    alert('光源已开启');
  };

  const handleLightOff = () => {
    alert('光源已关闭');
  };

  return (
    <div className="space-y-6">
      {/* Light Source Configuration Group */}
      <div className="border border-gray-300 rounded">
        <div className="bg-gray-100 px-4 py-2 border-b border-gray-300">
          <h4 className="text-gray-700">光源配置</h4>
        </div>
        <div className="p-4 space-y-4">
          {/* Control Type */}
          <div className="grid grid-cols-3 gap-4 items-center">
            <label className="text-sm text-gray-700">控制方式:</label>
            <select className="col-span-2 border border-gray-300 rounded px-3 py-2">
              <option>串口控制</option>
              <option>Modbus 控制</option>
              <option>GPIO 控制</option>
              <option>手动控制</option>
            </select>
          </div>

          {/* Serial Port Settings */}
          <div className="grid grid-cols-3 gap-4 items-center">
            <label className="text-sm text-gray-700">串口设置:</label>
            <div className="col-span-2 flex gap-2">
              <select className="flex-1 border border-gray-300 rounded px-3 py-2">
                <option>COM1</option>
                <option>COM2</option>
                <option>COM3</option>
                <option>/dev/ttyUSB0</option>
              </select>
              <select className="flex-1 border border-gray-300 rounded px-3 py-2" defaultValue="9600">
                <option value="9600">9600</option>
                <option value="19200">19200</option>
                <option value="38400">38400</option>
                <option value="115200">115200</option>
              </select>
            </div>
          </div>

          {/* Channel Configuration */}
          <div className="col-span-3">
            <div className="border border-gray-300 rounded mt-2">
              <div className="bg-gray-100 px-4 py-2 border-b border-gray-300">
                <h5 className="text-sm text-gray-700">通道配置</h5>
              </div>
              <div className="p-4 space-y-3">
                {channels.map((channel, index) => (
                  <div key={index} className="grid grid-cols-12 gap-3 items-center">
                    {/* Channel Name */}
                    <div className="col-span-3 text-sm text-gray-700">
                      {channel.name}
                    </div>
                    
                    {/* Enable Checkbox */}
                    <div className="col-span-2">
                      <label className="flex items-center gap-2 cursor-pointer">
                        <input
                          type="checkbox"
                          checked={channel.enabled}
                          onChange={(e) => updateChannel(index, 'enabled', e.target.checked)}
                          className="w-4 h-4"
                        />
                        <span className="text-sm">启用</span>
                      </label>
                    </div>
                    
                    {/* Slider */}
                    <div className="col-span-5">
                      <input
                        type="range"
                        min="0"
                        max="255"
                        value={channel.value}
                        onChange={(e) => updateChannel(index, 'value', Number(e.target.value))}
                        disabled={!channel.enabled}
                        className="w-full"
                      />
                    </div>
                    
                    {/* SpinBox */}
                    <div className="col-span-2">
                      <input
                        type="number"
                        min="0"
                        max="255"
                        value={channel.value}
                        onChange={(e) => updateChannel(index, 'value', Number(e.target.value))}
                        disabled={!channel.enabled}
                        className="w-full border border-gray-300 rounded px-2 py-1 text-sm text-center disabled:bg-gray-100 disabled:text-gray-400"
                      />
                    </div>
                  </div>
                ))}
              </div>
            </div>
          </div>

          {/* Strobe Settings */}
          <div className="col-span-3">
            <div className="border border-gray-300 rounded mt-2">
              <div className="bg-gray-100 px-4 py-2 border-b border-gray-300">
                <h5 className="text-sm text-gray-700">频闪设置</h5>
              </div>
              <div className="p-4 space-y-3">
                {/* Enable Strobe */}
                <div>
                  <label className="flex items-center gap-2 cursor-pointer">
                    <input
                      type="checkbox"
                      checked={strobeEnabled}
                      onChange={(e) => setStrobeEnabled(e.target.checked)}
                      className="w-4 h-4"
                    />
                    <span className="text-sm">启用频闪模式</span>
                  </label>
                </div>

                {/* Strobe Duration */}
                <div className="grid grid-cols-3 gap-4 items-center">
                  <label className="text-sm text-gray-700">频闪时长:</label>
                  <div className="col-span-2 flex items-center gap-2">
                    <input
                      type="number"
                      min="100"
                      max="10000"
                      value={strobeDuration}
                      onChange={(e) => setStrobeDuration(Number(e.target.value))}
                      disabled={!strobeEnabled}
                      className="flex-1 border border-gray-300 rounded px-3 py-2 disabled:bg-gray-100 disabled:text-gray-400"
                    />
                    <span className="text-sm text-gray-600">μs</span>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Test Buttons */}
      <div className="flex gap-3">
        <button
          onClick={handleLightOn}
          className="flex items-center gap-2 px-6 py-2 bg-yellow-500 text-white rounded hover:bg-yellow-600 transition-colors"
        >
          💡 开启光源
        </button>
        <button
          onClick={handleLightOff}
          className="flex items-center gap-2 px-6 py-2 border border-gray-300 rounded hover:bg-gray-100 transition-colors"
        >
          关闭光源
        </button>
      </div>
    </div>
  );
}