/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：前端可复用 UI 组件，负责将页面拆成可维护的展示/交互单元。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

'use client';

import { type FC } from 'react';

export interface ChangelogEntry {
  version: string;
  date: string;
  changes: string[];
  type: 'major' | 'minor' | 'patch';
}

interface ChangelogListProps {
  entries: ChangelogEntry[];
}

const typeColors = {
  major: 'bg-red-500/20 text-red-400',
  minor: 'bg-yellow-500/20 text-yellow-400',
  patch: 'bg-green-500/20 text-green-400',
};

const ChangelogList: FC<ChangelogListProps> = ({ entries }) => {
  return (
    <div className="space-y-8">
      {entries.map((entry, index) => (
        <div
          key={index}
          className="bg-gray-800 rounded-xl p-6"
        >
          <div className="flex items-center gap-4 mb-4">
            <h3 className="text-2xl font-bold text-white">v{entry.version}</h3>
            <span
              className={`px-3 py-1 text-sm font-medium rounded-full ${typeColors[entry.type]}`}
            >
              {entry.type}
            </span>
            <span className="text-gray-500 text-sm">{entry.date}</span>
          </div>
          <ul className="space-y-2">
            {entry.changes.map((change, changeIndex) => (
              <li
                key={changeIndex}
                className="flex items-start gap-3 text-gray-300"
              >
                <span className="text-purple-400 mt-1">•</span>
                <span>{change}</span>
              </li>
            ))}
          </ul>
        </div>
      ))}
    </div>
  );
};

export default ChangelogList;