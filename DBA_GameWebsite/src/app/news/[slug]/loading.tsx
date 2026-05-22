/*
中文阅读说明：
- 所属应用：DBA_GameWebsite 游戏官网。
- 文件职责：Next.js App Router 页面，负责官网路由、内容组织和响应式展示。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

export default function Loading() {
  return (
    <div className="min-h-screen bg-gray-900 py-24 px-4">
      <div className="max-w-4xl mx-auto">
        <div className="animate-pulse space-y-8">
          <div className="h-4 w-24 bg-gray-800 rounded" />
          <div className="h-8 w-48 bg-gray-800 rounded" />
          <div className="h-12 w-96 bg-gray-800 rounded" />
          <div className="space-y-4">
            <div className="h-4 bg-gray-800 rounded w-full" />
            <div className="h-4 bg-gray-800 rounded w-full" />
            <div className="h-4 bg-gray-800 rounded w-3/4" />
          </div>
        </div>
      </div>
    </div>
  );
}