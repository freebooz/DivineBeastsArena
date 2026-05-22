# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


PACKAGE_PATH = "/Game/DBA/Materials"
MATERIAL_NAME = "M_DBA_RuntimeTint"
MATERIAL_PATH = f"{PACKAGE_PATH}/{MATERIAL_NAME}.{MATERIAL_NAME}"


def main():
    unreal.EditorAssetLibrary.make_directory(PACKAGE_PATH)
    material = unreal.load_asset(MATERIAL_PATH)
    if not material:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        factory = unreal.MaterialFactoryNew()
        material = asset_tools.create_asset(MATERIAL_NAME, PACKAGE_PATH, unreal.Material, factory)
        if not material:
            raise RuntimeError(f"Failed to create {MATERIAL_PATH}")

    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)

    # Recreate a tiny, explicit parameterized graph. These parameter names match the runtime code.
    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)

    texture = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -760, -140)
    texture.set_editor_property("parameter_name", "AlbedoTexture")
    default_texture = unreal.load_asset("/Game/DBA/Characters/Rosales/Meshes/T_Rosales_Diffuse.T_Rosales_Diffuse")
    if default_texture:
        texture.set_editor_property("texture", default_texture)

    color = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -760, 80)
    color.set_editor_property("parameter_name", "BaseColor")
    color.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

    multiply = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionMultiply, -420, -60)
    unreal.MaterialEditingLibrary.connect_material_expressions(texture, "RGB", multiply, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(color, "", multiply, "B")
    unreal.MaterialEditingLibrary.connect_material_property(multiply, "", unreal.MaterialProperty.MP_BASE_COLOR)

    roughness = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -420, 120)
    roughness.set_editor_property("parameter_name", "Roughness")
    roughness.set_editor_property("default_value", 0.58)
    unreal.MaterialEditingLibrary.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)

    specular = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -420, 240)
    specular.set_editor_property("parameter_name", "Specular")
    specular.set_editor_property("default_value", 0.35)
    unreal.MaterialEditingLibrary.connect_material_property(specular, "", unreal.MaterialProperty.MP_SPECULAR)

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    unreal.log(f"Runtime tint material ready: {material.get_path_name()}")


if __name__ == "__main__":
    main()
