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
