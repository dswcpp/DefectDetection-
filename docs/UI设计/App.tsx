import { useState, useEffect } from 'react';
import { Header } from './components/Header';
import { Toolbar } from './components/Toolbar';
import { ImagePreview } from './components/ImagePreview';
import { ResultsPanel } from './components/ResultsPanel';
import { ParametersPanel } from './components/ParametersPanel';
import { StatusBar } from './components/StatusBar';
import { SettingsDialog } from './components/SettingsDialog';
import { StatisticsView } from './components/StatisticsView';

interface DetectionResult {
  status: 'OK' | 'NG';
  defects: Array<{
    type: string;
    confidence: number;
  }>;
  timestamp: string;
}

export default function App() {
  const [isRunning, setIsRunning] = useState(false);
  const [totalCount, setTotalCount] = useState(1234);
  const [okCount, setOkCount] = useState(1200);
  const [ngCount, setNgCount] = useState(34);
  const [detectionSpeed, setDetectionSpeed] = useState(45);
  const [showSettings, setShowSettings] = useState(false);
  const [showStatistics, setShowStatistics] = useState(false);
  
  const [currentResult, setCurrentResult] = useState<DetectionResult>({
    status: 'NG',
    defects: [
      { type: '划痕', confidence: 0.92 },
    ],
    timestamp: new Date().toLocaleString('zh-CN'),
  });

  // Simulate detection when running
  useEffect(() => {
    if (!isRunning) return;

    const interval = setInterval(() => {
      const isOk = Math.random() > 0.15;
      const now = new Date();
      
      setTotalCount((prev) => prev + 1);
      
      if (isOk) {
        setOkCount((prev) => prev + 1);
        setCurrentResult({
          status: 'OK',
          defects: [],
          timestamp: now.toLocaleString('zh-CN'),
        });
      } else {
        setNgCount((prev) => prev + 1);
        const defectTypes = [
          { type: '划痕', confidence: 0.85 + Math.random() * 0.15 },
          { type: '裂纹', confidence: 0.75 + Math.random() * 0.2 },
          { type: '异物', confidence: 0.8 + Math.random() * 0.18 },
          { type: '尺寸偏差', confidence: 0.9 + Math.random() * 0.1 },
        ];
        const randomDefect = defectTypes[Math.floor(Math.random() * defectTypes.length)];
        
        setCurrentResult({
          status: 'NG',
          defects: [randomDefect],
          timestamp: now.toLocaleString('zh-CN'),
        });
      }
      
      // Randomize detection speed slightly
      setDetectionSpeed(40 + Math.floor(Math.random() * 15));
    }, 2000);

    return () => clearInterval(interval);
  }, [isRunning]);

  const handleStart = () => {
    setIsRunning(true);
  };

  const handleStop = () => {
    setIsRunning(false);
  };

  const handleCapture = () => {
    // Simulate single capture
    const isOk = Math.random() > 0.3;
    const now = new Date();
    
    setTotalCount((prev) => prev + 1);
    
    if (isOk) {
      setOkCount((prev) => prev + 1);
      setCurrentResult({
        status: 'OK',
        defects: [],
        timestamp: now.toLocaleString('zh-CN'),
      });
    } else {
      setNgCount((prev) => prev + 1);
      setCurrentResult({
        status: 'NG',
        defects: [{ type: '划痕', confidence: 0.92 }],
        timestamp: now.toLocaleString('zh-CN'),
      });
    }
  };

  const handleSettings = () => {
    setShowSettings(true);
  };

  const handleStats = () => {
    setShowStatistics(true);
  };

  return (
    <div className="h-screen flex flex-col bg-gray-100">
      <Header />
      <Toolbar
        isRunning={isRunning}
        onStart={handleStart}
        onStop={handleStop}
        onCapture={handleCapture}
        onSettings={handleSettings}
        onStats={handleStats}
      />
      
      <div className="flex-1 flex overflow-hidden">
        {/* Main image preview area - 2/3 width */}
        <div className="flex-1 min-w-0">
          <ImagePreview
            imageUrl="https://images.unsplash.com/photo-1707852710487-8f7837975208?crop=entropy&cs=tinysrgb&fit=max&fm=jpg&ixid=M3w3Nzg4Nzd8MHwxfHNlYXJjaHwxfHxpbmR1c3RyaWFsJTIwbWFudWZhY3R1cmluZyUyMHF1YWxpdHl8ZW58MXx8fHwxNzY0MjA5MDg5fDA&ixlib=rb-4.1.0&q=80&w=1080"
            isRunning={isRunning}
          />
        </div>
        
        {/* Sidebar - 1/3 width */}
        <div className="w-96 border-l border-gray-300 flex flex-col overflow-hidden">
          <ResultsPanel result={currentResult} />
          <div className="flex-1 overflow-hidden">
            <ParametersPanel />
          </div>
        </div>
      </div>
      
      <StatusBar
        detectionSpeed={detectionSpeed}
        totalCount={totalCount}
        okCount={okCount}
        ngCount={ngCount}
      />
      
      {showSettings && <SettingsDialog onClose={() => setShowSettings(false)} />}
      {showStatistics && <StatisticsView onClose={() => setShowStatistics(false)} />}
    </div>
  );
}