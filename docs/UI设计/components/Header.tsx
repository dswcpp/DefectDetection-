export function Header() {
  return (
    <div className="bg-gray-800 text-white px-4 py-2 border-b border-gray-700">
      <div className="flex gap-6">
        <button className="hover:bg-gray-700 px-3 py-1 rounded transition-colors">
          文件
        </button>
        <button className="hover:bg-gray-700 px-3 py-1 rounded transition-colors">
          设置
        </button>
        <button className="hover:bg-gray-700 px-3 py-1 rounded transition-colors">
          帮助
        </button>
      </div>
    </div>
  );
}
