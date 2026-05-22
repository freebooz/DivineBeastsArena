# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal

paths = [
    "/Game/Animation/Zodiac/Dog/ABP_Dog.ABP_Dog",
    "/Game/Models/Zodiac/Dog/SK_Dog_Mesh.SK_Dog_Mesh",
    "/Game/Animation/Zodiac/Dog/Montages/AM_Dog_Idle.AM_Dog_Idle",
]

for p in paths:
    obj = unreal.load_asset(p)
    if obj:
        unreal.log_warning(f"[CheckLoad] OK {p} class={obj.get_class().get_name()} name={obj.get_name()}")
    else:
        unreal.log_warning(f"[CheckLoad] MISS {p}")
