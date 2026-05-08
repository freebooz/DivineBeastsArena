import unreal

root = "/Game/_AutoPlaceholdersTemp"
tools = unreal.AssetToolsHelpers.get_asset_tools()

def ensure_path(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

ensure_path(root)

results = []

def try_create(name, cls, factory):
    try:
        asset = tools.create_asset(name, root, cls, factory)
        results.append((name, bool(asset), str(asset.get_class().get_name()) if asset else "None"))
    except Exception as e:
        results.append((name, False, f"ERR:{e}"))

try:
    try_create("P_Test", unreal.ParticleSystem, unreal.ParticleSystemFactoryNew())
except Exception as e:
    results.append(("P_Test_ctor", False, f"ERR:{e}"))

try:
    try_create("SK_Test", unreal.SkeletalMesh, unreal.SkeletalMeshFactory())
except Exception as e:
    results.append(("SK_Test_ctor", False, f"ERR:{e}"))

try:
    try_create("ABP_Test", unreal.AnimBlueprint, unreal.AnimBlueprintFactory())
except Exception as e:
    results.append(("ABP_Test_ctor", False, f"ERR:{e}"))

try:
    try_create("AM_Test", unreal.AnimMontage, unreal.AnimMontageFactory())
except Exception as e:
    results.append(("AM_Test_ctor", False, f"ERR:{e}"))

for item in results:
    unreal.log_warning(f"[ProbeCreate] {item[0]} ok={item[1]} detail={item[2]}")

unreal.EditorAssetLibrary.save_directory(root, False, True)
