import { UserPlus, Edit2, Trash2 } from 'lucide-react';

export function UserSettings() {
  const users = [
    { id: 1, username: 'admin', role: '管理员', status: '在线' },
    { id: 2, username: 'operator1', role: '操作员', status: '离线' },
    { id: 3, username: 'engineer', role: '工程师', status: '在线' },
  ];

  return (
    <div className="space-y-6">
      {/* Current User */}
      <div>
        <h4 className="mb-4 text-gray-700">当前用户</h4>
        <div className="p-4 bg-blue-50 border border-blue-200 rounded">
          <div className="flex items-center gap-4">
            <div className="w-12 h-12 bg-blue-600 rounded-full flex items-center justify-center text-white">
              A
            </div>
            <div>
              <div>admin</div>
              <div className="text-sm text-gray-600">管理员 - 完全权限</div>
            </div>
          </div>
        </div>
      </div>

      {/* User List */}
      <div className="border-t border-gray-200 pt-6">
        <div className="flex items-center justify-between mb-4">
          <h4 className="text-gray-700">用户列表</h4>
          <button className="flex items-center gap-2 px-4 py-2 bg-blue-600 text-white rounded hover:bg-blue-700 transition-colors">
            <UserPlus className="w-4 h-4" />
            添加用户
          </button>
        </div>

        <div className="border border-gray-300 rounded overflow-hidden">
          <table className="w-full">
            <thead className="bg-gray-50">
              <tr>
                <th className="px-4 py-3 text-left text-sm text-gray-600">用户名</th>
                <th className="px-4 py-3 text-left text-sm text-gray-600">角色</th>
                <th className="px-4 py-3 text-left text-sm text-gray-600">状态</th>
                <th className="px-4 py-3 text-left text-sm text-gray-600">操作</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-gray-200">
              {users.map((user) => (
                <tr key={user.id} className="hover:bg-gray-50">
                  <td className="px-4 py-3">{user.username}</td>
                  <td className="px-4 py-3">{user.role}</td>
                  <td className="px-4 py-3">
                    <span
                      className={`inline-flex items-center gap-1 px-2 py-1 rounded text-xs ${
                        user.status === '在线'
                          ? 'bg-green-100 text-green-700'
                          : 'bg-gray-100 text-gray-700'
                      }`}
                    >
                      <div
                        className={`w-1.5 h-1.5 rounded-full ${
                          user.status === '在线' ? 'bg-green-500' : 'bg-gray-500'
                        }`}
                      ></div>
                      {user.status}
                    </span>
                  </td>
                  <td className="px-4 py-3">
                    <div className="flex gap-2">
                      <button className="p-1 text-blue-600 hover:bg-blue-50 rounded transition-colors">
                        <Edit2 className="w-4 h-4" />
                      </button>
                      <button className="p-1 text-red-600 hover:bg-red-50 rounded transition-colors">
                        <Trash2 className="w-4 h-4" />
                      </button>
                    </div>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

      {/* Role Permissions */}
      <div className="border-t border-gray-200 pt-6">
        <h4 className="mb-4 text-gray-700">角色权限</h4>
        <div className="space-y-4">
          {/* Admin Role */}
          <div className="border border-gray-300 rounded p-4">
            <div className="mb-3">管理员</div>
            <div className="grid grid-cols-2 gap-3">
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked disabled className="w-4 h-4" />
                <span className="text-sm">查看检测结果</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked disabled className="w-4 h-4" />
                <span className="text-sm">修改检测参数</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked disabled className="w-4 h-4" />
                <span className="text-sm">系统设置</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked disabled className="w-4 h-4" />
                <span className="text-sm">用户管理</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked disabled className="w-4 h-4" />
                <span className="text-sm">数据导出</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked disabled className="w-4 h-4" />
                <span className="text-sm">历史记录查询</span>
              </div>
            </div>
          </div>

          {/* Operator Role */}
          <div className="border border-gray-300 rounded p-4">
            <div className="mb-3">操作员</div>
            <div className="grid grid-cols-2 gap-3">
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked className="w-4 h-4" />
                <span className="text-sm">查看检测结果</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" className="w-4 h-4" />
                <span className="text-sm">修改检测参数</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" className="w-4 h-4" />
                <span className="text-sm">系统设置</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" className="w-4 h-4" />
                <span className="text-sm">用户管理</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked className="w-4 h-4" />
                <span className="text-sm">数据导出</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked className="w-4 h-4" />
                <span className="text-sm">历史记录查询</span>
              </div>
            </div>
          </div>

          {/* Engineer Role */}
          <div className="border border-gray-300 rounded p-4">
            <div className="mb-3">工程师</div>
            <div className="grid grid-cols-2 gap-3">
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked className="w-4 h-4" />
                <span className="text-sm">查看检测结果</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked className="w-4 h-4" />
                <span className="text-sm">修改检测参数</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked className="w-4 h-4" />
                <span className="text-sm">系统设置</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" className="w-4 h-4" />
                <span className="text-sm">用户管理</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked className="w-4 h-4" />
                <span className="text-sm">数据导出</span>
              </div>
              <div className="flex items-center gap-2">
                <input type="checkbox" defaultChecked className="w-4 h-4" />
                <span className="text-sm">历史记录查询</span>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
