import { useState } from 'react';
import { ImageWithFallback } from './figma/ImageWithFallback';

interface ImagePreviewProps {
  imageUrl: string;
  isRunning: boolean;
}

export function ImagePreview({ imageUrl, isRunning }: ImagePreviewProps) {
  const [viewMode, setViewMode] = useState<'original' | 'annotated'>('annotated');
  const [roiVisible, setRoiVisible] = useState(true);

  return (
    <div className="bg-gray-900 h-full flex flex-col">
      <div className="bg-gray-800 px-4 py-2 border-b border-gray-700 flex gap-4 items-center">
        <div className="flex gap-2">
          <button
            onClick={() => setViewMode('original')}
            className={`px-3 py-1 rounded transition-colors ${
              viewMode === 'original'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-700 text-gray-300 hover:bg-gray-600'
            }`}
          >
            原图
          </button>
          <button
            onClick={() => setViewMode('annotated')}
            className={`px-3 py-1 rounded transition-colors ${
              viewMode === 'annotated'
                ? 'bg-blue-600 text-white'
                : 'bg-gray-700 text-gray-300 hover:bg-gray-600'
            }`}
          >
            标注图
          </button>
        </div>
        <label className="flex items-center gap-2 text-gray-300 cursor-pointer">
          <input
            type="checkbox"
            checked={roiVisible}
            onChange={(e) => setRoiVisible(e.target.checked)}
            className="w-4 h-4"
          />
          显示ROI
        </label>
      </div>
      
      <div className="flex-1 relative overflow-hidden flex items-center justify-center p-4">
        <div className="relative max-w-full max-h-full">
          <ImageWithFallback
            src={imageUrl}
            alt="检测图像"
            className="max-w-full max-h-full object-contain"
          />
          
          {viewMode === 'annotated' && roiVisible && (
            <>
              {/* ROI Rectangle 1 */}
              <div
                className="absolute border-2 border-green-500"
                style={{
                  left: '15%',
                  top: '20%',
                  width: '30%',
                  height: '25%',
                }}
              >
                <div className="absolute -top-6 left-0 bg-green-500 text-white px-2 py-0.5 text-xs">
                  ROI-1
                </div>
              </div>
              
              {/* ROI Rectangle 2 */}
              <div
                className="absolute border-2 border-yellow-500"
                style={{
                  left: '55%',
                  top: '30%',
                  width: '25%',
                  height: '20%',
                }}
              >
                <div className="absolute -top-6 left-0 bg-yellow-500 text-white px-2 py-0.5 text-xs">
                  ROI-2
                </div>
              </div>
              
              {/* Defect marker */}
              <div
                className="absolute w-12 h-12 border-2 border-red-500 rounded-full animate-pulse"
                style={{
                  left: '62%',
                  top: '38%',
                }}
              >
                <div className="absolute -bottom-6 left-1/2 -translate-x-1/2 bg-red-500 text-white px-2 py-0.5 text-xs whitespace-nowrap">
                  划痕
                </div>
              </div>
            </>
          )}
        </div>
        
        {isRunning && (
          <div className="absolute top-4 right-4 bg-green-600 text-white px-3 py-1 rounded-full flex items-center gap-2">
            <div className="w-2 h-2 bg-white rounded-full animate-pulse"></div>
            运行中
          </div>
        )}
      </div>
    </div>
  );
}
