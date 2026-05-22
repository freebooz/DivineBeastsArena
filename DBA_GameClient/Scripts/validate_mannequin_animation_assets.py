# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


CLASS_CANDIDATES = [
    "/Game/DBA/Characters/Mannequins/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C",
    "/Game/DBA/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C",
    "/Game/DBA/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C",
]

ANIM_CANDIDATES = [
    "/Game/DBA/Characters/Mannequins/Animations/ThirdPersonIdle.ThirdPersonIdle",
    "/Game/DBA/Characters/Mannequins/Animations/Manny/MM_Idle.MM_Idle",
    "/Game/DBA/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle",
]


for path in CLASS_CANDIDATES:
    loaded = unreal.load_class(None, path)
    unreal.log(f"anim class candidate: {path} -> {loaded.get_name() if loaded else '<failed>'}")

for path in ANIM_CANDIDATES:
    loaded = unreal.EditorAssetLibrary.load_asset(path)
    skeleton = None
    if loaded:
        try:
            skeleton = loaded.get_editor_property("skeleton")
        except Exception:
            skeleton = None
    unreal.log(f"animation asset candidate: {path} -> {loaded.get_name() if loaded else '<failed>'}; skeleton={skeleton.get_path_name() if skeleton else '<none>'}")
