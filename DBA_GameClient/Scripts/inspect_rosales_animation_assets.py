# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


ASSETS = [
    "/Game/DBA/Characters/Rosales/Meshes/SK_Rosales.SK_Rosales",
    "/Game/DBA/Characters/Rosales/Meshes/SKEL_Rosales.SKEL_Rosales",
    "/Game/DBA/Characters/Rosales/Animations/AN_Standing_Idle.AN_Standing_Idle",
    "/Game/DBA/Characters/Rosales/Animations/AN_Run_Forward.AN_Run_Forward",
    "/Game/DBA/Characters/Rosales/Animations/AN_Standing_2H_Magic_Attack_02.AN_Standing_2H_Magic_Attack_02",
]


for path in ASSETS:
    asset = unreal.load_asset(path)
    unreal.log(f"ASSET {path}: {asset}")
    if asset:
        for prop in ("skeleton", "preview_mesh"):
            try:
                value = asset.get_editor_property(prop)
                unreal.log(f"  {prop}: {value.get_path_name() if value else value}")
            except Exception:
                pass
