import { useState } from 'react';
import { Camera, Lightbulb, Cable, Database, Target, User, X } from 'lucide-react';
import { CameraSettings } from './settings/CameraSettings';
import { LightSettings } from './settings/LightSettings';
import { PLCSettings } from './settings/PLCSettings';
import { StorageSettings } from './settings/StorageSettings';
import { DetectionSettings } from './settings/DetectionSettings';
import { UserSettings } from './settings/UserSettings';

type SettingsSection = 'camera' | 'light' | 'plc' | 'storage' | 'detection' | 'user';

interface SettingsDialogProps {
  onClose: () => void;
}

export function SettingsDialog({ onClose }: SettingsDialogProps) {
  const [activeSection, setActiveSection] = useState<SettingsSection>('camera');

  const sections = [
    { id: 'camera' as const, label: '相机设置', icon: Camera },
    { id: 'light' as const, label: '光源设置', icon: Lightbulb },
    { id: 'plc' as const, label: 'PLC 通信', icon: Cable },
    { id: 'storage' as const, label: '存储设置', icon: Database },
    { id: 'detection' as const, label: '检测参数', icon: Target },
    { id: 'user' as const, label: '用户权限', icon: User },
  ];

  const getSectionTitle = () => {
    const section = sections.find(s => s.id === activeSection);
    const Icon = section?.icon;
    return (
      <div className="flex items-center gap-2">
        {Icon && <Icon className="w-5 h-5" />}
        <span>{section?.label}</span>
      </div>
    );
  };

  const handleRestore = () => {
    if (confirm('确定要恢复默认设置吗？')) {
      alert('已恢复默认设置');
    }
  };

  const handleApply = () => {
    alert('设置已应用');
  };

  const handleCancel = () => {
    onClose();
  };

  const handleConfirm = () => {
    alert('设置已保存');
    onClose();
  };

  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-8">
      <div className="bg-white rounded-lg shadow-2xl w-full max-w-6xl h-[80vh] flex flex-col">
        {/* Header */}
        <div className="flex items-center justify-between px-6 py-4 border-b border-gray-200">
          <h2>系统设置</h2>
          <button
            onClick={onClose}
            className="p-2 hover:bg-gray-100 rounded-full transition-colors"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Main Content */}
        <div className="flex-1 flex overflow-hidden">
          {/* Left Navigation */}
          <div className="w-64 bg-gray-50 border-r border-gray-200 p-4">
            <nav className="space-y-1">
              <div className="text-xs text-gray-500 px-3 py-2">导航列表</div>
              <div className="border-t border-gray-300 my-2"></div>
              {sections.map(({ id, label, icon: Icon }) => (
                <button
                  key={id}
                  onClick={() => setActiveSection(id)}
                  className={`w-full flex items-center gap-3 px-3 py-2.5 rounded transition-colors ${
                    activeSection === id
                      ? 'bg-blue-600 text-white'
                      : 'text-gray-700 hover:bg-gray-200'
                  }`}
                >
                  <Icon className="w-4 h-4" />
                  <span>{label}</span>
                </button>
              ))}
            </nav>
          </div>

          {/* Right Content Area */}
          <div className="flex-1 flex flex-col">
            {/* Content Header */}
            <div className="px-6 py-4 border-b border-gray-200 bg-gray-50">
              <div className="flex items-center gap-2">
                {getSectionTitle()}
              </div>
            </div>

            {/* Content Body */}
            <div className="flex-1 overflow-y-auto p-6">
              {activeSection === 'camera' && <CameraSettings />}
              {activeSection === 'light' && <LightSettings />}
              {activeSection === 'plc' && <PLCSettings />}
              {activeSection === 'storage' && <StorageSettings />}
              {activeSection === 'detection' && <DetectionSettings />}
              {activeSection === 'user' && <UserSettings />}
            </div>

            {/* Footer Buttons */}
            <div className="px-6 py-4 border-t border-gray-200 bg-gray-50">
              <div className="flex justify-end gap-3">
                <button
                  onClick={handleRestore}
                  className="px-4 py-2 border border-gray-300 rounded hover:bg-gray-100 transition-colors"
                >
                  恢复默认
                </button>
                <button
                  onClick={handleApply}
                  className="px-4 py-2 bg-blue-600 text-white rounded hover:bg-blue-700 transition-colors"
                >
                  应用
                </button>
                <button
                  onClick={handleCancel}
                  className="px-4 py-2 border border-gray-300 rounded hover:bg-gray-100 transition-colors"
                >
                  取消
                </button>
                <button
                  onClick={handleConfirm}
                  className="px-4 py-2 bg-green-600 text-white rounded hover:bg-green-700 transition-colors"
                >
                  确定
                </button>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
