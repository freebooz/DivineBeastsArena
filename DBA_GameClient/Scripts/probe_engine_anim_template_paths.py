# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


def try_load(path: str):
    obj = unreal.load_asset(path)
    if obj:
        unreal.log_warning(f"[ProbeEngineTemplate] hit path={path} class={obj.get_class().get_name()}")
    else:
        unreal.log_warning(f"[ProbeEngineTemplate] miss path={path}")


skeletal_mesh_candidates = [
    "/Engine/EngineMeshes/SkeletalCube.SkeletalCube",
    "/Engine/EngineMeshes/SkeletalCube_Simple.SkeletalCube_Simple",
    "/Engine/EngineMeshes/Humanoid.Humanoid",
    "/Engine/EngineMeshes/Humanoid.Humanoid",
    "/Engine/EngineMeshes/SK_Mannequin.SK_Mannequin",
    "/Engine/EditorMeshes/EditorSkeletalMesh.EditorSkeletalMesh",
    "/Engine/EditorMeshes/PersonaMesh.PersonaMesh",
]

skeleton_candidates = [
    "/Engine/EngineMeshes/Humanoid.Humanoid",
    "/Engine/EngineMeshes/SK_Mannequin_Skeleton.SK_Mannequin_Skeleton",
    "/Engine/EditorMeshes/PersonaAnimEditorSkel.PersonaAnimEditorSkel",
]

for p in skeletal_mesh_candidates:
    try_load(p)

for p in skeleton_candidates:
    try_load(p)
