# Copyright Freebooz Studio. All Rights Reserved.
#
# CreateTypedDataAssets.py
#
# 创建并写入真实 C++ DataAsset 子类内容
#
# 推荐执行方式:
# UnrealEditor.exe "D:/DivineBeastsArena/DivineBeastsArena.uproject" -ExecutePythonScript="D:/DivineBeastsArena/Scripts/CreateTypedDataAssets.py"
#

import unreal


CONTENT_PATH = "/Game"
PROJECT_MODULE = "DivineBeastsArena"

DELETE_EXISTING_ASSETS_BEFORE_CREATE = True
COLLECT_GARBAGE_AFTER_DELETE = True

# 是否在正式创建前打印 C++ 类诊断信息
PRINT_CLASS_DIAGNOSTICS = True


# ============================================
# 全局 C++ 类变量
# ============================================

DBAFiveCampDataAsset = None
DBAElementDataAsset = None
DBAZodiacDataAsset = None
DBAHeroDataAsset = None
DBAAbilitySetDataAsset = None
DBAObjectPoolConfigDataAsset = None
DBAFixedSkillGroupDataAsset = None


# ============================================
# 日志
# ============================================

def log_info(msg):
    unreal.log("[CreateTypedDataAssets][INFO] {}".format(msg))


def log_warn(msg):
    unreal.log_warning("[CreateTypedDataAssets][WARN] {}".format(msg))


def log_error(msg):
    unreal.log_error("[CreateTypedDataAssets][ERROR] {}".format(msg))


# ============================================
# 路径 / 类
# ============================================

def normalize_path(path):
    if not path:
        return CONTENT_PATH
    if not path.startswith("/"):
        path = "/" + path
    return path.rstrip("/")


def make_asset_path(package_path, asset_name):
    return "{}/{}".format(normalize_path(package_path), asset_name)


def get_class_candidate_paths(class_name):
    """
    Unreal C++ 类:
        UDBAFiveCampDataAsset
    Python 反射路径:
        /Script/DivineBeastsArena.DBAFiveCampDataAsset

    这里额外尝试 Editor 模块，方便排查类是否被错误放进 Editor 模块。
    """
    return [
        "/Script/{}.{}".format(PROJECT_MODULE, class_name),
        "/Script/{}Editor.{}".format(PROJECT_MODULE, class_name),
    ]


def load_class_required(class_name):
    candidates = get_class_candidate_paths(class_name)

    for path in candidates:
        cls = unreal.load_class(None, path)
        if cls:
            log_info("加载 C++ 类成功: {}".format(path))
            return cls

    log_error("加载 C++ 类失败: {}".format(class_name))
    log_error("尝试过以下路径:")
    for path in candidates:
        log_error("  {}".format(path))

    log_error("请检查以下事项:")
    log_error("1. C++ 类是否存在，例如 U{}".format(class_name))
    log_error("2. 类是否有 UCLASS(BlueprintType) 或 UCLASS()")
    log_error("3. 类是否有模块导出宏，例如 DIVINEBEASTSARENA_API")
    log_error("4. 类是否放在 Source/{}/ 而不是 Editor 模块".format(PROJECT_MODULE))
    log_error("5. 项目是否完整重新编译，且重启过 Unreal Editor")
    log_error("6. 类名是否和脚本中的名字完全一致")

    raise RuntimeError("加载 C++ 类失败: {}".format(class_name))


def print_class_diagnostics():
    log_info("=" * 80)
    log_info("开始诊断 C++ DataAsset 类")
    log_info("=" * 80)

    class_names = [
        "DBAFiveCampDataAsset",
        "DBAElementDataAsset",
        "DBAZodiacDataAsset",
        "DBAHeroDataAsset",
        "DBAAbilitySetDataAsset",
        "DBAObjectPoolConfigDataAsset",
        "DBAFixedSkillGroupDataAsset",
    ]

    for class_name in class_names:
        found = False
        for path in get_class_candidate_paths(class_name):
            cls = unreal.load_class(None, path)
            if cls:
                found = True
                log_info("FOUND: {} -> {}".format(path, cls))
            else:
                log_warn("NOT FOUND: {}".format(path))

        if not found:
            log_error("类未找到: {}".format(class_name))

    log_info("=" * 80)
    log_info("C++ DataAsset 类诊断结束")
    log_info("=" * 80)


def load_all_required_classes():
    global DBAFiveCampDataAsset
    global DBAElementDataAsset
    global DBAZodiacDataAsset
    global DBAHeroDataAsset
    global DBAAbilitySetDataAsset
    global DBAObjectPoolConfigDataAsset
    global DBAFixedSkillGroupDataAsset

    DBAFiveCampDataAsset = load_class_required("DBAFiveCampDataAsset")
    DBAElementDataAsset = load_class_required("DBAElementDataAsset")
    DBAZodiacDataAsset = load_class_required("DBAZodiacDataAsset")
    DBAHeroDataAsset = load_class_required("DBAHeroDataAsset")
    DBAAbilitySetDataAsset = load_class_required("DBAAbilitySetDataAsset")
    DBAObjectPoolConfigDataAsset = load_class_required("DBAObjectPoolConfigDataAsset")
    DBAFixedSkillGroupDataAsset = load_class_required("DBAFixedSkillGroupDataAsset")


def ensure_folder_exists(path):
    path = normalize_path(path)
    if unreal.EditorAssetLibrary.does_directory_exist(path):
        return True

    ok = unreal.EditorAssetLibrary.make_directory(path)
    if ok:
        log_info("创建目录: {}".format(path))
    else:
        log_error("创建目录失败: {}".format(path))
    return ok


def collect_garbage():
    if not COLLECT_GARBAGE_AFTER_DELETE:
        return

    try:
        unreal.SystemLibrary.collect_garbage()
    except Exception as e:
        log_warn("GC 失败，可忽略: {}".format(str(e)))


def delete_asset_if_exists(asset_path):
    if not DELETE_EXISTING_ASSETS_BEFORE_CREATE:
        return True

    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return True

    log_warn("删除已存在资产: {}".format(asset_path))

    try:
        ok = unreal.EditorAssetLibrary.delete_asset(asset_path)
        if ok:
            collect_garbage()
            return True

        log_error("删除资产失败: {}".format(asset_path))
        return False
    except Exception as e:
        log_error("删除资产异常: {} - {}".format(asset_path, str(e)))
        return False


def make_text(value):
    return unreal.Text(value)


def make_color(r, g, b, a=1.0):
    return unreal.LinearColor(r, g, b, a)


def request_tag(tag_name):
    if not tag_name:
        return unreal.GameplayTag()

    try:
        return unreal.GameplayTag.request_gameplay_tag(tag_name, False)
    except Exception:
        log_warn("GameplayTag 不存在或请求失败: {}".format(tag_name))
        return unreal.GameplayTag()


def set_prop(asset, prop, value):
    try:
        asset.set_editor_property(prop, value)
        return True
    except Exception as e:
        log_warn("设置属性失败: {}.{} = {} | {}".format(asset.get_name(), prop, value, str(e)))
        return False


def save_asset(asset, asset_path):
    try:
        ok = unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
        if ok:
            log_info("保存成功: {}".format(asset_path))
        else:
            log_error("保存失败: {}".format(asset_path))
        return ok
    except Exception as e:
        log_error("保存异常: {} - {}".format(asset_path, str(e)))
        return False


def create_data_asset(package_path, asset_name, data_asset_class):
    package_path = normalize_path(package_path)
    asset_path = make_asset_path(package_path, asset_name)

    if not ensure_folder_exists(package_path):
        return None

    if not delete_asset_if_exists(asset_path):
        return None

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", data_asset_class)

    asset = asset_tools.create_asset(
        asset_name=asset_name,
        package_path=package_path,
        asset_class=data_asset_class,
        factory=factory
    )

    if not asset:
        log_error("创建资产失败: {}".format(asset_path))
        return None

    log_info("创建资产成功: {}".format(asset_path))
    return asset


def load_asset_optional(path):
    if not path:
        return None

    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        log_warn("引用资产不存在或加载失败: {}".format(path))
    return asset


# ============================================
# 枚举转换
# ============================================

def enum_value(enum_type, value):
    try:
        return getattr(enum_type, value)
    except Exception as e:
        log_error("枚举值不存在: {}.{} | {}".format(enum_type, value, str(e)))
        raise


def hero_role(value):
    return enum_value(unreal.EDBAHeroRole, value)


def ability_slot(value):
    return enum_value(unreal.EDBAAbilitySlot, value)


def ability_type(value):
    return enum_value(unreal.EDBAAbilityType, value)


def pool_type(value):
    return enum_value(unreal.EDBAObjectPoolType, value)


# ============================================
# 创建 FiveCamp
# ============================================

def create_five_camps():
    data = [
        {
            "asset": "DA_FiveCamp_Fire",
            "id": "FiveCamp_Fire",
            "name": "火之阵营",
            "english": "Fire Camp",
            "element": "Element.Fire",
            "camp_tag": "FiveCamp.Fire",
            "guardian": "炽焰狮",
            "style": "爆发 / 进攻 / 压制",
            "desc": "崇尚烈焰与勇气的阵营，擅长爆发伤害、持续灼烧和正面压制。",
            "color": (1.0, 0.18, 0.08),
            "heroes": ["Hero_FireLion"],
        },
        {
            "asset": "DA_FiveCamp_Water",
            "id": "FiveCamp_Water",
            "name": "水之阵营",
            "english": "Water Camp",
            "element": "Element.Water",
            "camp_tag": "FiveCamp.Water",
            "guardian": "碧波龙",
            "style": "治疗 / 控制 / 持续作战",
            "desc": "掌控水流与恢复之力的阵营，擅长治疗、减速、保护和团队续航。",
            "color": (0.1, 0.45, 1.0),
            "heroes": ["Hero_WaterDragon"],
        },
        {
            "asset": "DA_FiveCamp_Earth",
            "id": "FiveCamp_Earth",
            "name": "土之阵营",
            "english": "Earth Camp",
            "element": "Element.Earth",
            "camp_tag": "FiveCamp.Earth",
            "guardian": "岩甲熊",
            "style": "防御 / 护盾 / 坦克",
            "desc": "以大地之力守护队友的阵营，擅长护盾、减伤、控制与阵地防守。",
            "color": (0.55, 0.38, 0.16),
            "heroes": ["Hero_EarthBear"],
        },
        {
            "asset": "DA_FiveCamp_Gold",
            "id": "FiveCamp_Gold",
            "name": "金之阵营",
            "english": "Gold Camp",
            "element": "Element.Gold",
            "camp_tag": "FiveCamp.Gold",
            "guardian": "金光凤",
            "style": "精准 / 爆发 / 反击",
            "desc": "象征秩序与锋锐的阵营，擅长暴击、破甲、反击和瞬间斩杀。",
            "color": (1.0, 0.78, 0.12),
            "heroes": ["Hero_GoldPhoenix"],
        },
        {
            "asset": "DA_FiveCamp_Wood",
            "id": "FiveCamp_Wood",
            "name": "木之阵营",
            "english": "Wood Camp",
            "element": "Element.Wood",
            "camp_tag": "FiveCamp.Wood",
            "guardian": "灵木鹤",
            "style": "治愈 / 召唤 / 增益",
            "desc": "代表生命与成长的阵营，擅长持续治疗、召唤、增益和战场支援。",
            "color": (0.15, 0.65, 0.25),
            "heroes": ["Hero_WoodCrane"],
        },
    ]

    for row in data:
        asset = create_data_asset("/Game/DBA/FiveCamps", row["asset"], DBAFiveCampDataAsset)
        if not asset:
            continue

        set_prop(asset, "data_id", row["id"])
        set_prop(asset, "camp_id", row["id"])
        set_prop(asset, "display_name", make_text(row["name"]))
        set_prop(asset, "english_name", make_text(row["english"]))
        set_prop(asset, "description", make_text(row["desc"]))
        set_prop(asset, "primary_tag", request_tag(row["camp_tag"]))
        set_prop(asset, "camp_tag", request_tag(row["camp_tag"]))
        set_prop(asset, "element_tag", request_tag(row["element"]))
        set_prop(asset, "guardian_beast", make_text(row["guardian"]))
        set_prop(asset, "combat_style", make_text(row["style"]))
        set_prop(asset, "recommended_hero_ids", row["heroes"])
        set_prop(asset, "theme_color", make_color(*row["color"]))

        save_asset(asset, "/Game/DBA/FiveCamps/{}".format(row["asset"]))


# ============================================
# 创建 Elements
# ============================================

def create_elements():
    data = [
        ("DA_Element_Fire", "Element_Fire", "火", "Fire", "Element.Fire", "Gold", "Water", "灼烧 / 爆发", "火元素代表毁灭与爆发，擅长持续灼烧和瞬间高伤害。", (1.0, 0.15, 0.05)),
        ("DA_Element_Water", "Element_Water", "水", "Water", "Element.Water", "Fire", "Earth", "治疗 / 减速", "水元素代表流动与恢复，擅长治疗、减速和控制敌方节奏。", (0.05, 0.45, 1.0)),
        ("DA_Element_Earth", "Element_Earth", "土", "Earth", "Element.Earth", "Water", "Wood", "护盾 / 减伤", "土元素代表稳定与守护，擅长护盾、护甲和范围控制。", (0.55, 0.38, 0.18)),
        ("DA_Element_Gold", "Element_Gold", "金", "Gold", "Element.Gold", "Wood", "Fire", "暴击 / 破甲", "金元素代表锋锐与秩序，擅长暴击、穿透和精准打击。", (1.0, 0.78, 0.1)),
        ("DA_Element_Wood", "Element_Wood", "木", "Wood", "Element.Wood", "Earth", "Gold", "恢复 / 生长", "木元素代表生命与成长，擅长持续恢复、召唤和增益。", (0.12, 0.62, 0.25)),
    ]

    for asset_name, data_id, name, english, tag, strong, weak, mechanic, desc, color in data:
        asset = create_data_asset("/Game/DBA/Elements", asset_name, DBAElementDataAsset)
        if not asset:
            continue

        set_prop(asset, "data_id", data_id)
        set_prop(asset, "element_id", data_id)
        set_prop(asset, "display_name", make_text(name))
        set_prop(asset, "english_name", make_text(english))
        set_prop(asset, "description", make_text(desc))
        set_prop(asset, "primary_tag", request_tag(tag))
        set_prop(asset, "element_tag", request_tag(tag))
        set_prop(asset, "strength_against", strong)
        set_prop(asset, "weak_against", weak)
        set_prop(asset, "core_mechanic", make_text(mechanic))
        set_prop(asset, "theme_color", make_color(*color))

        save_asset(asset, "/Game/DBA/Elements/{}".format(asset_name))


# ============================================
# 创建 Zodiac
# ============================================

def create_zodiacs():
    data = [
        ("DA_Zodiac_Aries", "Zodiac_Aries", "白羊座", "Aries", "Element_Fire", "攻击速度 +3%", 0.03, "象征冲锋与勇气，适合高进攻英雄。"),
        ("DA_Zodiac_Taurus", "Zodiac_Taurus", "金牛座", "Taurus", "Element_Earth", "最大生命 +3%", 0.03, "象征坚韧与稳定，适合坦克和守护者。"),
        ("DA_Zodiac_Gemini", "Zodiac_Gemini", "双子座", "Gemini", "Element_Wood", "技能冷却 -2%", 0.02, "象征灵动与变化，适合机动型英雄。"),
        ("DA_Zodiac_Cancer", "Zodiac_Cancer", "巨蟹座", "Cancer", "Element_Water", "治疗效果 +3%", 0.03, "象征守护与恢复，适合辅助型英雄。"),
        ("DA_Zodiac_Leo", "Zodiac_Leo", "狮子座", "Leo", "Element_Fire", "暴击伤害 +3%", 0.03, "象征王者与威严，适合爆发型英雄。"),
        ("DA_Zodiac_Virgo", "Zodiac_Virgo", "处女座", "Virgo", "Element_Gold", "精准 +3%", 0.03, "象征秩序与分析，适合精准打击。"),
        ("DA_Zodiac_Libra", "Zodiac_Libra", "天秤座", "Libra", "Element_Gold", "护盾强度 +3%", 0.03, "象征平衡与协调，适合团队保护。"),
        ("DA_Zodiac_Scorpio", "Zodiac_Scorpio", "天蝎座", "Scorpio", "Element_Water", "控制持续时间 +2%", 0.02, "象征毒性与压制，适合控制型英雄。"),
        ("DA_Zodiac_Sagittarius", "Zodiac_Sagittarius", "射手座", "Sagittarius", "Element_Fire", "远程伤害 +3%", 0.03, "象征追击与远射，适合远程输出。"),
        ("DA_Zodiac_Capricorn", "Zodiac_Capricorn", "摩羯座", "Capricorn", "Element_Earth", "受到伤害 -2%", 0.02, "象征忍耐与防御，适合前排英雄。"),
        ("DA_Zodiac_Aquarius", "Zodiac_Aquarius", "水瓶座", "Aquarius", "Element_Water", "能量恢复 +3%", 0.03, "象征智慧与流动，适合法术型英雄。"),
        ("DA_Zodiac_Pisces", "Zodiac_Pisces", "双鱼座", "Pisces", "Element_Wood", "生命恢复 +3%", 0.03, "象征梦境与生命，适合治疗和支援。"),
    ]

    for asset_name, data_id, name, english, element, bonus, value, desc in data:
        asset = create_data_asset("/Game/DBA/Zodiacs", asset_name, DBAZodiacDataAsset)
        if not asset:
            continue

        set_prop(asset, "data_id", data_id)
        set_prop(asset, "zodiac_id", data_id)
        set_prop(asset, "display_name", make_text(name))
        set_prop(asset, "english_name", make_text(english))
        set_prop(asset, "description", make_text(desc))
        set_prop(asset, "default_element_id", element)
        set_prop(asset, "passive_bonus", make_text(bonus))
        set_prop(asset, "bonus_value", value)

        save_asset(asset, "/Game/DBA/Zodiacs/{}".format(asset_name))


# ============================================
# 创建 AbilitySets
# ============================================

def create_ability_sets():
    data = [
        ("DA_AS_FireLion_Passive", "AS_FireLion_Passive", "Hero_FireLion", "Passive", "Passive", "炽焰血脉", "Input.Passive", "Element.Fire", "Ability.FireLion.Passive", "", 0, 0, "技能命中敌人后叠加灼烧，达到层数后引发爆燃。"),
        ("DA_AS_FireLion_Q", "AS_FireLion_Q", "Hero_FireLion", "Q", "Active", "烈焰爪击", "Input.Ability.Q", "Element.Fire", "Ability.FireLion.Q", "Cooldown.FireLion.Q", 6, 40, "向前方挥出火焰爪击，对敌人造成伤害并附加灼烧。"),
        ("DA_AS_FireLion_W", "AS_FireLion_W", "Hero_FireLion", "W", "Active", "狮吼护体", "Input.Ability.W", "Element.Earth", "Ability.FireLion.W", "Cooldown.FireLion.W", 12, 60, "发出战吼获得护盾，并短时间提升韧性。"),
        ("DA_AS_FireLion_E", "AS_FireLion_E", "Hero_FireLion", "E", "Mobility", "炽焰跃击", "Input.Ability.E", "Element.Fire", "Ability.FireLion.E", "Cooldown.FireLion.E", 14, 70, "跃向目标区域，对落点敌人造成伤害和短暂击飞。"),
        ("DA_AS_FireLion_R", "AS_FireLion_R", "Hero_FireLion", "R", "Ultimate", "神兽降临·炎狮", "Input.Ability.R", "Element.Fire", "Ability.FireLion.R", "Cooldown.FireLion.R", 90, 100, "进入炎狮神兽形态，大幅提升伤害并强化技能。"),

        ("DA_AS_WaterDragon_Passive", "AS_WaterDragon_Passive", "Hero_WaterDragon", "Passive", "Passive", "碧波灵息", "Input.Passive", "Element.Water", "Ability.WaterDragon.Passive", "", 0, 0, "技能命中友方时附加恢复，命中敌方时施加潮湿。"),
        ("DA_AS_WaterDragon_Q", "AS_WaterDragon_Q", "Hero_WaterDragon", "Q", "Active", "水龙冲击", "Input.Ability.Q", "Element.Water", "Ability.WaterDragon.Q", "Cooldown.WaterDragon.Q", 7, 45, "释放水流冲击，对敌人造成伤害和减速，命中友方可治疗。"),

        ("DA_AS_EarthBear_Passive", "AS_EarthBear_Passive", "Hero_EarthBear", "Passive", "Passive", "岩甲守护", "Input.Passive", "Element.Earth", "Ability.EarthBear.Passive", "", 0, 0, "受到伤害时获得岩甲层数，层数满时触发护盾。"),
        ("DA_AS_EarthBear_Q", "AS_EarthBear_Q", "Hero_EarthBear", "Q", "Active", "裂地重击", "Input.Ability.Q", "Element.Earth", "Ability.EarthBear.Q", "Cooldown.EarthBear.Q", 8, 35, "猛击地面，对前方敌人造成伤害并短暂击飞。"),

        ("DA_AS_GoldPhoenix_Passive", "AS_GoldPhoenix_Passive", "Hero_GoldPhoenix", "Passive", "Passive", "金羽涅槃", "Input.Passive", "Element.Gold", "Ability.GoldPhoenix.Passive", "", 0, 0, "致命伤害时短暂化为金羽，若期间未被击破则恢复生命。"),
        ("DA_AS_GoldPhoenix_Q", "AS_GoldPhoenix_Q", "Hero_GoldPhoenix", "Q", "Active", "金羽穿刺", "Input.Ability.Q", "Element.Gold", "Ability.GoldPhoenix.Q", "Cooldown.GoldPhoenix.Q", 6, 45, "发射金羽，对直线敌人造成穿透伤害并降低护甲。"),

        ("DA_AS_WoodCrane_Passive", "AS_WoodCrane_Passive", "Hero_WoodCrane", "Passive", "Passive", "灵木生息", "Input.Passive", "Element.Wood", "Ability.WoodCrane.Passive", "", 0, 0, "周期性为附近生命最低的友方恢复生命。"),
        ("DA_AS_WoodCrane_Q", "AS_WoodCrane_Q", "Hero_WoodCrane", "Q", "Heal", "灵木治愈", "Input.Ability.Q", "Element.Wood", "Ability.WoodCrane.Q", "Cooldown.WoodCrane.Q", 5, 50, "治疗目标友方，并在目标周围生成短暂恢复区域。"),
    ]

    for asset_name, sid, hero, slot, atype, name, input_tag, element_tag, ability_tag, cooldown_tag, cd, cost, desc in data:
        asset = create_data_asset("/Game/DBA/AbilitySets", asset_name, DBAAbilitySetDataAsset)
        if not asset:
            continue

        set_prop(asset, "data_id", sid)
        set_prop(asset, "ability_set_id", sid)
        set_prop(asset, "hero_id", hero)
        set_prop(asset, "slot", ability_slot(slot))
        set_prop(asset, "ability_type", ability_type(atype))
        set_prop(asset, "display_name", make_text(name))
        set_prop(asset, "english_name", make_text(sid))
        set_prop(asset, "description", make_text(desc))
        set_prop(asset, "input_tag", request_tag(input_tag))
        set_prop(asset, "element_tag", request_tag(element_tag))
        set_prop(asset, "ability_tag", request_tag(ability_tag))

        if cooldown_tag:
            set_prop(asset, "cooldown_tag", request_tag(cooldown_tag))

        set_prop(asset, "ability_level", 1)
        set_prop(asset, "cooldown", float(cd))
        set_prop(asset, "mana_cost", float(cost))

        save_asset(asset, "/Game/DBA/AbilitySets/{}".format(asset_name))


# ============================================
# 创建 Heroes
# ============================================

def create_heroes():
    data = [
        {
            "asset": "DA_Hero_FireLion",
            "id": "Hero_FireLion",
            "name": "炽焰狮",
            "english": "Fire Lion",
            "camp": "FiveCamp_Fire",
            "element": "Element_Fire",
            "role": "Fighter",
            "difficulty": 3,
            "primary": "Strength",
            "desc": "火之阵营代表英雄，擅长冲锋、灼烧和正面爆发。",
            "stats": dict(max_hp=1650, max_mana=420, attack=85, ability_power=45, armor=42, magic_resist=30, move_speed=360, attack_range=175),
            "bp": "/Game/DBA/Heroes/BP_Hero_FireLion.BP_Hero_FireLion_C",
            "passive": "/Game/DBA/AbilitySets/DA_AS_FireLion_Passive",
            "q": "/Game/DBA/AbilitySets/DA_AS_FireLion_Q",
            "w": "/Game/DBA/AbilitySets/DA_AS_FireLion_W",
            "e": "/Game/DBA/AbilitySets/DA_AS_FireLion_E",
            "r": "/Game/DBA/AbilitySets/DA_AS_FireLion_R",
        },
        {
            "asset": "DA_Hero_WaterDragon",
            "id": "Hero_WaterDragon",
            "name": "碧波龙",
            "english": "Water Dragon",
            "camp": "FiveCamp_Water",
            "element": "Element_Water",
            "role": "Support",
            "difficulty": 3,
            "primary": "Spirit",
            "desc": "水之阵营代表英雄，擅长治疗、减速、群体控制和保护队友。",
            "stats": dict,
            "bp": "/Game/DBA/Heroes/BP_Hero_WaterDragon.BP_Hero_WaterDragon_C",
            "passive": "/Game/DBA/AbilitySets/DA_AS_WaterDragon_Passive",
            "q": "/Game/DBA/AbilitySets/DA_AS_WaterDragon_Q",
            "w": "",
            "e": "",
            "r": "",
        },
        {
            "asset": "DA_Hero_EarthBear",
            "id": "Hero_EarthBear",
            "name": "岩甲熊",
            "english": "Earth Bear",
            "camp": "FiveCamp_Earth",
            "element": "Element_Earth",
            "role": "Tank",
            "difficulty": 2,
            "primary": "Vitality",
            "desc": "土之阵营代表英雄，拥有高生命、高护甲和强力控制。",
            "stats": dict(max_hp=2100, max_mana=350, attack=65, ability_power=35, armor=58, magic_resist=38, move_speed=330, attack_range=150),
            "bp": "/Game/DBA/Heroes/BP_Hero_EarthBear.BP_Hero_EarthBear_C",
            "passive": "/Game/DBA/AbilitySets/DA_AS_EarthBear_Passive",
            "q": "/Game/DBA/AbilitySets/DA_AS_EarthBear_Q",
            "w": "",
            "e": "",
            "r": "",
        },
        {
            "asset": "DA_Hero_GoldPhoenix",
            "id": "Hero_GoldPhoenix",
            "name": "金光凤",
            "english": "Gold Phoenix",
            "camp": "FiveCamp_Gold",
            "element": "Element_Gold",
            "role": "Assassin",
            "difficulty": 4,
            "primary": "Agility",
            "desc": "金之阵营代表英雄，擅长精准爆发、破甲和复活反击。",
            "stats": dict(max_hp=1250, max_mana=480, attack=95, ability_power=70, armor=30, magic_resist=32, move_speed=375, attack_range=425),
            "bp": "/Game/DBA/Heroes/BP_Hero_GoldPhoenix.BP_Hero_GoldPhoenix_C",
            "passive": "/Game/DBA/AbilitySets/DA_AS_GoldPhoenix_Passive",
            "q": "/Game/DBA/AbilitySets/DA_AS_GoldPhoenix_Q",
            "w": "",
            "e": "",
            "r": "",
        },
        {
            "asset": "DA_Hero_WoodCrane",
            "id": "Hero_WoodCrane",
            "name": "灵木鹤",
            "english": "Wood Crane",
            "camp": "FiveCamp_Wood",
            "element": "Element_Wood",
            "role": "Healer",
            "difficulty": 3,
            "primary": "Wisdom",
            "desc": "木之阵营代表英雄，擅长治疗、召唤、增益和持续作战。",
            "stats": dict(max_hp=1400, max_mana=680, attack=40, ability_power=85, armor=26, magic_resist=46, move_speed=350, attack_range=575),
            "bp": "/Game/DBA/Heroes/BP_Hero_WoodCrane.BP_Hero_WoodCrane_C",
            "passive": "/Game/DBA/AbilitySets/DA_AS_WoodCrane_Passive",
            "q": "/Game/DBA/AbilitySets/DA_AS_WoodCrane_Q",
            "w": "",
            "e": "",
            "r": "",
        },
    ]

    for row in data:
        asset = create_data_asset("/Game/DBA/Heroes", row["asset"], DBAHeroDataAsset)
        if not asset:
            continue

        set_prop(asset, "data_id", row["id"])
        set_prop(asset, "hero_id", row["id"])
        set_prop(asset, "display_name", make_text(row["name"]))
        set_prop(asset, "english_name", make_text(row["english"]))
        set_prop(asset, "description", make_text(row["desc"]))
        set_prop(asset, "camp_id", row["camp"])
        set_prop(asset, "element_id", row["element"])
        set_prop(asset, "role", hero_role(row["role"]))
        set_prop(asset, "difficulty", row["difficulty"])
        set_prop(asset, "primary_attribute", row["primary"])

        stats = unreal.FDBAHeroBaseStats()
        for k, v in row["stats"].items():
            try:
                stats.set_editor_property(k, float(v))
            except Exception as e:
                log_warn("设置英雄基础属性失败: {}.{} = {} | {}".format(row["asset"], k, v, str(e)))

        set_prop(asset, "base_stats", stats)

        if row["bp"]:
            try:
                set_prop(asset, "hero_blueprint_class", unreal.SoftClassPath(row["bp"]))
            except Exception as e:
                log_warn("设置 Hero Blueprint Class 失败: {} | {}".format(row["bp"], str(e)))

        ability_prop_map = {
            "passive": "passive_ability_set",
            "q": "qability_set",
            "w": "wability_set",
            "e": "eability_set",
            "r": "rability_set",
        }

        for key, prop_name in ability_prop_map.items():
            path = row[key]
            if not path:
                continue

            loaded = load_asset_optional(path)
            if loaded:
                set_prop(asset, prop_name, loaded)

        save_asset(asset, "/Game/DBA/Heroes/{}".format(row["asset"]))


# ============================================
# 创建 Pool Configs
# ============================================

def create_pool_configs():
    data = [
        ("DA_OPT_Projectile", "OPT_Projectile", "弹道对象池", "Projectile", 64, 256, True, True, 10.0, "管理技能弹道、飞行物、投射物对象。"),
        ("DA_OPT_Effect", "OPT_Effect", "特效对象池", "Effect", 128, 512, True, True, 8.0, "管理命中特效、范围特效、持续特效对象。"),
        ("DA_OPT_UI", "OPT_UI", "UI 对象池", "UI", 32, 128, True, True, 3.0, "管理浮动战斗文字、Buff 图标、临时 UI 元素。"),
    ]

    for asset_name, pid, name, ptype, default_size, max_size, expandable, auto_return, lifetime, desc in data:
        asset = create_data_asset("/Game/DBA/Data/PoolConfigs", asset_name, DBAObjectPoolConfigDataAsset)
        if not asset:
            continue

        set_prop(asset, "data_id", pid)
        set_prop(asset, "pool_id", pid)
        set_prop(asset, "display_name", make_text(name))
        set_prop(asset, "english_name", make_text(pid))
        set_prop(asset, "description", make_text(desc))
        set_prop(asset, "pool_type", pool_type(ptype))
        set_prop(asset, "default_size", default_size)
        set_prop(asset, "max_size", max_size)
        set_prop(asset, "b_expandable", expandable)
        set_prop(asset, "b_auto_return", auto_return)
        set_prop(asset, "max_lifetime", lifetime)

        save_asset(asset, "/Game/DBA/Data/PoolConfigs/{}".format(asset_name))


# ============================================
# 创建 FixedSkillGroups
# ============================================

def create_skill_groups():
    data = [
        ("DA_FSG_Standard_5v5_1", "FSG_Standard_5v5_1", "标准 5v5 配置组 1", 1, "Frontline", "防御被动", "控制 Q", "护盾 W", "位移 E", "团控 R", "Flash", "Guard", "适合前排开团英雄。"),
        ("DA_FSG_Standard_5v5_2", "FSG_Standard_5v5_2", "标准 5v5 配置组 2", 2, "BurstDamage", "增伤被动", "爆发 Q", "追击 W", "斩杀 E", "爆发 R", "Flash", "Execute", "适合爆发输出英雄。"),
        ("DA_FSG_Standard_5v5_3", "FSG_Standard_5v5_3", "标准 5v5 配置组 3", 3, "Support", "治疗被动", "单体治疗 Q", "群体护盾 W", "净化 E", "团队恢复 R", "Flash", "Heal", "适合支援辅助英雄。"),
        ("DA_FSG_Standard_5v5_4", "FSG_Standard_5v5_4", "标准 5v5 配置组 4", 4, "Control", "控制被动", "减速 Q", "禁锢 W", "击飞 E", "区域封锁 R", "Flash", "Exhaust", "适合控制型英雄。"),
        ("DA_FSG_Standard_5v5_5", "FSG_Standard_5v5_5", "标准 5v5 配置组 5", 5, "Flexible", "通用被动", "输出 Q", "防御 W", "位移 E", "战术 R", "Flash", "Flex", "适合灵活补位英雄。"),
    ]

    for asset_name, gid, name, index, role, passive, q, w, e, r, spell1, spell2, desc in data:
        asset = create_data_asset("/Game/DBA/Data/SkillGroups", asset_name, DBAFixedSkillGroupDataAsset)
        if not asset:
            continue

        set_prop(asset, "data_id", gid)
        set_prop(asset, "group_id", gid)
        set_prop(asset, "mode_id", "Standard_5v5")
        set_prop(asset, "display_name", make_text(name))
        set_prop(asset, "english_name", make_text(gid))
        set_prop(asset, "description", make_text(desc))
        set_prop(asset, "slot_index", index)
        set_prop(asset, "recommended_role", role)
        set_prop(asset, "passive_skill_id", passive)
        set_prop(asset, "qskill_id", q)
        set_prop(asset, "wskill_id", w)
        set_prop(asset, "eskill_id", e)
        set_prop(asset, "rskill_id", r)
        set_prop(asset, "battle_spell1", spell1)
        set_prop(asset, "battle_spell2", spell2)

        save_asset(asset, "/Game/DBA/Data/SkillGroups/{}".format(asset_name))


# ============================================
# 主函数
# ============================================

def main():
    log_info("=" * 80)
    log_info("开始创建并写入 Typed DataAssets")
    log_info("=" * 80)

    if PRINT_CLASS_DIAGNOSTICS:
        print_class_diagnostics()

    load_all_required_classes()

    create_five_camps()
    create_elements()
    create_zodiacs()

    # 先创建 AbilitySet，再创建 Hero。
    # 因为 Hero 会引用 AbilitySet。
    create_ability_sets()
    create_heroes()

    create_pool_configs()
    create_skill_groups()

    log_info("=" * 80)
    log_info("Typed DataAssets 创建并写入完成")
    log_info("=" * 80)


if __name__ == "__main__":
    main()
