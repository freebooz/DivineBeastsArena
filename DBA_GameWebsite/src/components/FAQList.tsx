/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：前端可复用 UI 组件，负责将页面拆成可维护的展示/交互单元。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

'use client';

import { type FC, useState } from 'react';

export interface FAQItem {
  question: string;
  answer: string;
}

interface FAQListProps {
  items: FAQItem[];
}

const FAQList: FC<FAQListProps> = ({ items }) => {
  const [openIndex, setOpenIndex] = useState<number | null>(null);

  const toggleItem = (index: number) => {
    setOpenIndex(openIndex === index ? null : index);
  };

  return (
    <div className="space-y-4">
      {items.map((item, index) => (
        <div
          key={index}
          className="bg-gray-800 rounded-xl overflow-hidden"
        >
          <button
            onClick={() => toggleItem(index)}
            className="w-full flex items-center justify-between p-6 text-left"
          >
            <span className="text-lg font-semibold text-white">
              {item.question}
            </span>
            <span
              className={`text-purple-400 text-2xl transition-transform duration-200 ${
                openIndex === index ? 'rotate-45' : ''
              }`}
            >
              +
            </span>
          </button>
          {openIndex === index && (
            <div className="px-6 pb-6">
              <p className="text-gray-400 leading-relaxed">{item.answer}</p>
            </div>
          )}
        </div>
      ))}
    </div>
  );
};

export default FAQList;