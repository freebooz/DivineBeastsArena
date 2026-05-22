#!/usr/bin/env python3
# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import sys
import unreal


PAIRS = [
    (
        "/Game/DBA/Characters/Mannequins/Meshes/SKM_Manny_Simple",
        "/Game/DBA/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C",
    ),
    (
        "/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin",
        "/Game/DBA/Characters/Mannequins/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C",
    ),
    (
        "/Game/DBA/Characters/Mannequins/Meshes/SKM_Manny_Simple",
        "/Game/DBA/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C",
    ),
]


def main():
    ok = False
    for mesh_path, anim_path in PAIRS:
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        anim_class = unreal.load_class(None, anim_path)
        unreal.log(f"Candidate mesh={mesh_path} loaded={bool(mesh)} anim={anim_path} loaded={bool(anim_class)}")
        if mesh and anim_class:
            ok = True
    if not ok:
        sys.exit(1)


if __name__ == "__main__":
    main()
