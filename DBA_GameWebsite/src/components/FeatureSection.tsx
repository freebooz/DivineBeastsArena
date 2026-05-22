/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：前端可复用 UI 组件，负责将页面拆成可维护的展示/交互单元。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

'use client';

import { type FC } from 'react';

interface Feature {
  title: string;
  description: string;
  icon: string;
}

interface FeatureSectionProps {
  features?: Feature[];
}

const defaultFeatures: Feature[] = [
  {
    title: 'Real-time Combat',
    description: 'Experience intense PvP battles with responsive controls and smooth animations.',
    icon: '⚔️',
  },
  {
    title: 'Global Leaderboards',
    description: 'Compete with players worldwide and climb the ranks to become a legend.',
    icon: '🏆',
  },
  {
    title: 'Cross-Platform Play',
    description: 'Play with friends on any platform. Windows, Mac, or Linux - everyone can join.',
    icon: '🌐',
  },
];

const FeatureSection: FC<FeatureSectionProps> = ({
  features = defaultFeatures,
}) => {
  return (
    <section className="py-24 bg-gray-900 px-4">
      <div className="max-w-6xl mx-auto">
        <h2 className="text-3xl md:text-4xl font-bold text-white text-center mb-16">
          Features
        </h2>
        <div className="grid grid-cols-1 md:grid-cols-3 gap-8">
          {features.map((feature, index) => (
            <div
              key={index}
              className="bg-gray-800 rounded-xl p-8 hover:bg-gray-700 transition-colors duration-200"
            >
              <div className="text-5xl mb-6">{feature.icon}</div>
              <h3 className="text-xl font-bold text-white mb-3">
                {feature.title}
              </h3>
              <p className="text-gray-400 leading-relaxed">
                {feature.description}
              </p>
            </div>
          ))}
        </div>
      </div>
    </section>
  );
};

export default FeatureSection;