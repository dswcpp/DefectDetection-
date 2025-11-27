import { useState } from 'react';
import { ImageWithFallback } from '../figma/ImageWithFallback';

export function CameraSettings() {
  const [exposureValue, setExposureValue] = useState(5000);
  const [gainValue, setGainValue] = useState(0);
  const [isConnected, setIsConnected] = useState(false);
  const [previewImage, setPreviewImage] = useState<string | null>(null);

  const handleTestCamera = () => {
    setIsConnected(true);
    setPreviewImage('https://images.unsplash.com/photo-1707852710487-8f7837975208?crop=entropy&cs=tinysrgb&fit=max&fm=jpg&ixid=M3w3Nzg4Nzd8MHwxfHNlYXJjaHwxfHxpbmR1c3RyaWFsJTIwbWFudWZhY3R1cmluZyUyMHF1YWxpdHl8ZW58MXx8fHwxNzY0MjA5MDg5fDA&ixlib=rb-4.1.0&q=80&w=1080');
    alert('相机连接成功');
  };

  const handleCaptureFrame = () => {
    if (!isConnected) {
      alert('请先连接相机');
      return;
    }
    alert('已抓取一帧');
  };

  const handleScanCameras = () => {
    alert('正在扫描网络中的相机...\n找到 2 台相机');
  };

  return (
    <div className="space-y-6">
      {/* Camera Configuration Group */}
      <div className="border border-gray-300 rounded">
        <div className="bg-gray-100 px-4 py-2 border-b border-gray-300">
          <h4 className="text-gray-700">相机配置</h4>
        </div>
        <div className="p-4 space-y-4">
          {/* Camera Type */}
          <div className="grid grid-cols-3 gap-4 items-center">
            <label className="text-sm text-gray-700">相机类型:</label>
            <select className="col-span-2 border border-gray-300 rounded px-3 py-2">
              <option>GigE Vision</option>
              <option>USB3 Vision</option>
              <option>海康 SDK</option>
              <option>大恒 SDK</option>
              <option>文件输入</option>
            </select>
          </div>

          {/* Camera IP / Device ID */}
          <div className="grid grid-cols-3 gap-4 items-center">
            <label className="text-sm text-gray-700">相机 IP:</label>
            <div className="col-span-2 flex gap-2">
              <input
                type="text"
                defaultValue="192.168.1.100"
                placeholder="192.168.1.100"
                className="flex-1 border border-gray-300 rounded px-3 py-2"
              />
              <button
                onClick={handleScanCameras}
                className="px-4 py-2 border border-gray-300 rounded hover:bg-gray-100 transition-colors"
              >
                扫描
              </button>
            </div>
          </div>

          {/* Exposure Time with Slider and SpinBox */}
          <div className="grid grid-cols-3 gap-4 items-center">
            <label className="text-sm text-gray-700">曝光时间:</label>
            <div className="col-span-2 flex gap-3 items-center">
              <input
                type="range"
                min="100"
                max="100000"
                value={exposureValue}
                onChange={(e) => setExposureValue(Number(e.target.value))}
                className="flex-1"
              />
              <input
                type="number"
                min="100"
                max="100000"
                value={exposureValue}
                onChange={(e) => setExposureValue(Number(e.target.value))}
                className="w-28 border border-gray-300 rounded px-2 py-1.5 text-sm text-right"
              />
              <span className="text-sm text-gray-600 w-8">μs</span>
            </div>
          </div>

          {/* Gain with Slider and SpinBox */}
          <div className="grid grid-cols-3 gap-4 items-center">
            <label className="text-sm text-gray-700">增益:</label>
            <div className="col-span-2 flex gap-3 items-center">
              <input
                type="range"
                min="0"
                max="24"
                value={gainValue}
                onChange={(e) => setGainValue(Number(e.target.value))}
                className="flex-1"
              />
              <input
                type="number"
                min="0"
                max="24"
                value={gainValue}
                onChange={(e) => setGainValue(Number(e.target.value))}
                className="w-28 border border-gray-300 rounded px-2 py-1.5 text-sm text-right"
              />
              <span className="text-sm text-gray-600 w-8">dB</span>
            </div>
          </div>

          {/* Trigger Mode */}
          <div className="grid grid-cols-3 gap-4 items-center">
            <label className="text-sm text-gray-700">触发模式:</label>
            <select className="col-span-2 border border-gray-300 rounded px-3 py-2" defaultValue="hardware">
              <option value="continuous">连续采集</option>
              <option value="software">软触发</option>
              <option value="hardware">硬触发</option>
            </select>
          </div>

          {/* Image Format */}
          <div className="grid grid-cols-3 gap-4 items-center">
            <label className="text-sm text-gray-700">图像格式:</label>
            <select className="col-span-2 border border-gray-300 rounded px-3 py-2">
              <option>Mono8</option>
              <option>Mono12</option>
              <option>BayerRG8</option>
              <option>RGB8</option>
            </select>
          </div>

          {/* Resolution */}
          <div className="grid grid-cols-3 gap-4 items-center">
            <label className="text-sm text-gray-700">分辨率:</label>
            <select className="col-span-2 border border-gray-300 rounded px-3 py-2">
              <option>2592×1944</option>
              <option>1920×1080</option>
              <option>1280×720</option>
              <option>640×480</option>
            </select>
          </div>
        </div>
      </div>

      {/* Camera Preview Group */}
      <div className="border border-gray-300 rounded">
        <div className="bg-gray-100 px-4 py-2 border-b border-gray-300">
          <h4 className="text-gray-700">相机预览</h4>
        </div>
        <div className="p-4 space-y-4">
          {/* Preview Area */}
          <div className="bg-gray-800 rounded flex items-center justify-center min-h-[240px]">
            {previewImage ? (
              <ImageWithFallback
                src={previewImage}
                alt="相机预览"
                className="max-w-full max-h-60 object-contain"
              />
            ) : (
              <div className="text-gray-400 text-center">
                <div className="mb-2">未连接</div>
                <div className="text-sm">请先连接相机以查看预览</div>
              </div>
            )}
          </div>

          {/* Preview Control Buttons */}
          <div className="flex gap-3">
            <button
              onClick={handleTestCamera}
              className="flex items-center gap-2 px-4 py-2 bg-blue-600 text-white rounded hover:bg-blue-700 transition-colors"
            >
              📷 连接测试
            </button>
            <button
              onClick={handleCaptureFrame}
              className="flex items-center gap-2 px-4 py-2 border border-gray-300 rounded hover:bg-gray-100 transition-colors"
            >
              🔄 抓取一帧
            </button>
          </div>
        </div>
      </div>
    </div>
  );
}