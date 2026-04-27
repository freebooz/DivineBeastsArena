# Copyright Freebooz Studio. All Rights Reserved.
#
# GenerateVFXSFXAssets.py
#
# UE5 Niagara VFX / SoundWave SFX 资源生成脚本
#
# 在 UE Editor Python Console 中执行：
# py D:/DivineBeastsArena/Scripts/GenerateVFXSFXAssets.py
#
# 说明：
# 1. 创建前会删除同路径同名资产。
# 2. NiagaraSystem 这里只创建占位资源，复杂 Niagara 节点图需要后续在 Niagara Editor 中制作。
# 3. SoundWave 会由脚本自动生成简单 wav 文件并导入 UE。
# 4. 本脚本不递归保存整个 /Game，只保存生成的单个资产。
#

import unreal
import os
import math
import wave
import struct


# ============================================
# 基础配置
# ============================================

CONTENT_PATH = "/Game"
PROJECT_NAME = "DivineBeastsArena"

DELETE_EXISTING_ASSETS_BEFORE_CREATE = True
COLLECT_GARBAGE_AFTER_DELETE = True

GENERATE_NIAGARA_SYSTEMS = True
GENERATE_SOUND_WAVES = True

SAMPLE_RATE = 44100
DEFAULT_DURATION = 0.75
DEFAULT_VOLUME = 0.35


# ============================================
# 日志
# ============================================

def log_info(msg):
    unreal.log("[GenerateVFXSFXAssets][INFO] {}".format(msg))


def log_warn(msg):
    unreal.log_warning("[GenerateVFXSFXAssets][WARN] {}".format(msg))


def log_error(msg):
    unreal.log_error("[GenerateVFXSFXAssets][ERROR] {}".format(msg))


# ============================================
# 路径工具
# ============================================

def normalize_path(path):
    if not path:
        return CONTENT_PATH

    if not path.startswith("/"):
        path = "/" + path

    return path.rstrip("/")


def make_asset_path(package_path, asset_name):
    return "{}/{}".format(normalize_path(package_path), asset_name)


def ensure_folder_exists(folder_path):
    folder_path = normalize_path(folder_path)

    if unreal.EditorAssetLibrary.does_directory_exist(folder_path):
        return True

    try:
        success = unreal.EditorAssetLibrary.make_directory(folder_path)

        if success:
            log_info("创建文件夹: {}".format(folder_path))
            return True

        log_error("创建文件夹失败: {}".format(folder_path))
        return False

    except Exception as e:
        log_error("创建文件夹异常: {} - {}".format(folder_path, str(e)))
        return False


def get_generated_audio_dir():
    project_saved = unreal.Paths.project_saved_dir()
    audio_dir = os.path.join(project_saved, "GeneratedAudio", "DBA_SFX")

    if not os.path.exists(audio_dir):
        os.makedirs(audio_dir)

    return audio_dir


# ============================================
# 删除 / GC / 保存
# ============================================

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

    log_warn("资产已存在，准备删除: {}".format(asset_path))

    try:
        deleted = unreal.EditorAssetLibrary.delete_asset(asset_path)

        if deleted:
            log_info("删除资产成功: {}".format(asset_path))
            collect_garbage()
            return True

        log_error("删除资产失败: {}".format(asset_path))
        return False

    except Exception as e:
        log_error("删除资产异常: {} - {}".format(asset_path, str(e)))
        return False


def save_loaded_asset_safely(asset, asset_path):
    if not asset:
        log_error("保存失败，asset 为 None: {}".format(asset_path))
        return False

    try:
        saved = unreal.EditorAssetLibrary.save_loaded_asset(
            asset,
            only_if_is_dirty=False
        )

        if saved:
            log_info("保存资产成功: {}".format(asset_path))
            return True

        log_warn("保存资产返回 False: {}".format(asset_path))
        return False

    except Exception as e:
        log_error("保存资产异常: {} - {}".format(asset_path, str(e)))
        return False


# ============================================
# Niagara 创建
# ============================================

def create_niagara_system(package_path, asset_name):
    package_path = normalize_path(package_path)
    asset_path = make_asset_path(package_path, asset_name)

    log_info("-" * 80)
    log_info("准备创建 NiagaraSystem: {}".format(asset_path))

    if not GENERATE_NIAGARA_SYSTEMS:
        log_warn("GENERATE_NIAGARA_SYSTEMS = False，跳过: {}".format(asset_path))
        return None

    if not ensure_folder_exists(package_path):
        return None

    if not delete_asset_if_exists(asset_path):
        log_error("删除已有 NiagaraSystem 失败，跳过创建: {}".format(asset_path))
        return None

    if not hasattr(unreal, "NiagaraSystemFactoryNew"):
        log_error("当前 UE Python 环境未暴露 NiagaraSystemFactoryNew。请确认 Niagara 插件已启用。")
        return None

    if not hasattr(unreal, "NiagaraSystem"):
        log_error("当前 UE Python 环境未暴露 NiagaraSystem。请确认 Niagara 插件已启用。")
        return None

    try:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        factory = unreal.NiagaraSystemFactoryNew()

        niagara_system = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=package_path,
            asset_class=unreal.NiagaraSystem,
            factory=factory
        )

        if not niagara_system:
            log_error("创建 NiagaraSystem 失败: {}".format(asset_path))
            return None

        log_info("创建 NiagaraSystem 成功: {}".format(asset_path))
        save_loaded_asset_safely(niagara_system, asset_path)
        return niagara_system

    except Exception as e:
        log_error("创建 NiagaraSystem 异常: {} - {}".format(asset_path, str(e)))
        return None


# ============================================
# WAV 生成 / SoundWave 导入
# ============================================

def envelope(index, total):
    if total <= 0:
        return 0.0

    t = float(index) / float(total)

    attack = 0.06
    release = 0.18

    if t < attack:
        return t / attack

    if t > 1.0 - release:
        return max(0.0, (1.0 - t) / release)

    return 1.0


def waveform_sample(t, freq, style):
    if style == "fire":
        base = math.sin(2.0 * math.pi * freq * t)
        grit = 0.35 * math.sin(2.0 * math.pi * freq * 2.5 * t)
        noise = 0.18 * math.sin(2.0 * math.pi * freq * 9.0 * t)
        return base + grit + noise

    if style == "water":
        return (
            math.sin(2.0 * math.pi * freq * t) * 0.7 +
            math.sin(2.0 * math.pi * (freq * 1.5) * t) * 0.25 +
            math.sin(2.0 * math.pi * (freq * 0.5) * t) * 0.2
        )

    if style == "earth":
        return (
            math.sin(2.0 * math.pi * freq * t) * 0.8 +
            math.sin(2.0 * math.pi * (freq * 0.5) * t) * 0.5
        )

    if style == "gold":
        return (
            math.sin(2.0 * math.pi * freq * t) * 0.65 +
            math.sin(2.0 * math.pi * (freq * 2.0) * t) * 0.4 +
            math.sin(2.0 * math.pi * (freq * 3.0) * t) * 0.2
        )

    if style == "wood":
        return (
            math.sin(2.0 * math.pi * freq * t) * 0.55 +
            math.sin(2.0 * math.pi * (freq * 1.25) * t) * 0.35 +
            math.sin(2.0 * math.pi * (freq * 0.75) * t) * 0.2
        )

    if style == "impact":
        return (
            math.sin(2.0 * math.pi * freq * t) * 0.8 +
            math.sin(2.0 * math.pi * (freq * 0.25) * t) * 0.5
        )

    return math.sin(2.0 * math.pi * freq * t)


def generate_wav_file(file_path, frequency=440.0, duration=DEFAULT_DURATION, volume=DEFAULT_VOLUME, style="default"):
    total_samples = int(SAMPLE_RATE * duration)

    with wave.open(file_path, "w") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(SAMPLE_RATE)

        for i in range(total_samples):
            t = float(i) / float(SAMPLE_RATE)

            freq_sweep = frequency

            if style == "fire":
                freq_sweep = frequency + 80.0 * t

            elif style == "water":
                freq_sweep = frequency + math.sin(t * 12.0) * 35.0

            elif style == "earth":
                freq_sweep = max(40.0, frequency - 40.0 * t)

            elif style == "gold":
                freq_sweep = frequency + 220.0 * t

            elif style == "wood":
                freq_sweep = frequency + math.sin(t * 5.0) * 20.0

            elif style == "impact":
                freq_sweep = max(40.0, frequency * (1.0 - t * 0.7))

            amp = envelope(i, total_samples) * volume
            sample = waveform_sample(t, freq_sweep, style) * amp
            sample = max(-1.0, min(1.0, sample))

            wav.writeframes(struct.pack("<h", int(sample * 32767.0)))

    return file_path


def import_wav_as_sound_wave(wav_file_path, package_path, asset_name):
    package_path = normalize_path(package_path)
    asset_path = make_asset_path(package_path, asset_name)

    log_info("-" * 80)
    log_info("准备导入 SoundWave: {}".format(asset_path))

    if not GENERATE_SOUND_WAVES:
        log_warn("GENERATE_SOUND_WAVES = False，跳过: {}".format(asset_path))
        return None

    if not ensure_folder_exists(package_path):
        return None

    if not delete_asset_if_exists(asset_path):
        log_error("删除已有 SoundWave 失败，跳过导入: {}".format(asset_path))
        return None

    try:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", wav_file_path)
        task.set_editor_property("destination_path", package_path)
        task.set_editor_property("destination_name", asset_name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)

        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        asset_tools.import_asset_tasks([task])

        imported_asset = unreal.EditorAssetLibrary.load_asset(asset_path)

        if imported_asset:
            log_info("导入 SoundWave 成功: {}".format(asset_path))
            save_loaded_asset_safely(imported_asset, asset_path)
            return imported_asset

        log_error("导入后无法加载 SoundWave: {}".format(asset_path))
        return None

    except Exception as e:
        log_error("导入 SoundWave 异常: {} - {}".format(asset_path, str(e)))
        return None


def create_sound_wave(package_path, asset_name, frequency, duration, style):
    audio_dir = get_generated_audio_dir()
    wav_file_path = os.path.join(audio_dir, "{}.wav".format(asset_name))

    generate_wav_file(
        wav_file_path,
        frequency=frequency,
        duration=duration,
        volume=DEFAULT_VOLUME,
        style=style
    )

    return import_wav_as_sound_wave(
        wav_file_path,
        package_path,
        asset_name
    )


# ============================================
# 资源清单
# ============================================
#
# 字段:
# hero, category, vfx_name, sfx_name, style, frequency, duration, description
#

ABILITY_EFFECT_ASSETS = [
    # FireLion
    ("FireLion", "Abilities", "NS_FireLion_Passive_BurnAura", "SFX_FireLion_Passive_BurnAura", "fire", 180, 1.2, "炽焰血脉持续燃烧光环"),
    ("FireLion", "Abilities", "NS_FireLion_Q_FlameClaw_Slash", "SFX_FireLion_Q_FlameClaw_Slash", "fire", 320, 0.55, "烈焰爪击挥砍"),
    ("FireLion", "Abilities", "NS_FireLion_Q_FlameClaw_Impact", "SFX_FireLion_Q_FlameClaw_Impact", "impact", 160, 0.45, "烈焰爪击命中爆裂"),
    ("FireLion", "Abilities", "NS_FireLion_W_RoarShield", "SFX_FireLion_W_RoarShield", "fire", 140, 0.9, "狮吼护体火焰护盾"),
    ("FireLion", "Abilities", "NS_FireLion_E_FlameLeap_Trail", "SFX_FireLion_E_FlameLeap_Jump", "fire", 260, 0.5, "炽焰跃击空中拖尾"),
    ("FireLion", "Abilities", "NS_FireLion_E_FlameLeap_Impact", "SFX_FireLion_E_FlameLeap_Impact", "impact", 120, 0.65, "炽焰跃击落地冲击"),
    ("FireLion", "Abilities", "NS_FireLion_R_DivineBeastTransform", "SFX_FireLion_R_DivineBeastTransform", "fire", 100, 1.8, "炎狮神兽形态变身"),

    # WaterDragon
    ("WaterDragon", "Abilities", "NS_WaterDragon_Passive_WaterBreathAura", "SFX_WaterDragon_Passive_WaterBreathAura", "water", 260, 1.1, "碧波灵息水流光环"),
    ("WaterDragon", "Abilities", "NS_WaterDragon_Q_WaterBlast_Projectile", "SFX_WaterDragon_Q_WaterBlast_Cast", "water", 360, 0.65, "水龙冲击弹道"),
    ("WaterDragon", "Abilities", "NS_WaterDragon_Q_WaterBlast_Impact", "SFX_WaterDragon_Q_WaterBlast_Impact", "water", 220, 0.5, "水龙冲击命中水爆"),
    ("WaterDragon", "Abilities", "NS_WaterDragon_Q_WaterBlast_HealImpact", "SFX_WaterDragon_Q_WaterBlast_HealImpact", "water", 520, 0.7, "水龙冲击治疗命中"),

    # EarthBear
    ("EarthBear", "Abilities", "NS_EarthBear_Passive_RockArmor", "SFX_EarthBear_Passive_RockArmor", "earth", 90, 1.0, "岩甲守护护甲凝聚"),
    ("EarthBear", "Abilities", "NS_EarthBear_Q_GroundSlam_Crack", "SFX_EarthBear_Q_GroundSlam_Cast", "earth", 80, 0.55, "裂地重击地裂前摇"),
    ("EarthBear", "Abilities", "NS_EarthBear_Q_GroundSlam_Impact", "SFX_EarthBear_Q_GroundSlam_Impact", "impact", 70, 0.8, "裂地重击冲击"),

    # GoldPhoenix
    ("GoldPhoenix", "Abilities", "NS_GoldPhoenix_Passive_RebirthFeathers", "SFX_GoldPhoenix_Passive_Rebirth", "gold", 620, 1.4, "金羽涅槃羽毛环绕"),
    ("GoldPhoenix", "Abilities", "NS_GoldPhoenix_Q_GoldFeather_Projectile", "SFX_GoldPhoenix_Q_GoldFeather_Cast", "gold", 760, 0.45, "金羽穿刺弹道"),
    ("GoldPhoenix", "Abilities", "NS_GoldPhoenix_Q_GoldFeather_Impact", "SFX_GoldPhoenix_Q_GoldFeather_Impact", "gold", 480, 0.45, "金羽穿刺命中"),

    # WoodCrane
    ("WoodCrane", "Abilities", "NS_WoodCrane_Passive_LifePulse", "SFX_WoodCrane_Passive_LifePulse", "wood", 440, 1.2, "灵木生息生命脉冲"),
    ("WoodCrane", "Abilities", "NS_WoodCrane_Q_HealingSeed_Projectile", "SFX_WoodCrane_Q_HealingSeed_Cast", "wood", 520, 0.65, "灵木治愈种子弹道"),
    ("WoodCrane", "Abilities", "NS_WoodCrane_Q_HealingGrove_Area", "SFX_WoodCrane_Q_HealingGrove_Area", "wood", 360, 1.6, "灵木治愈区域"),
    ("WoodCrane", "Abilities", "NS_WoodCrane_Q_HealingBurst_Impact", "SFX_WoodCrane_Q_HealingBurst_Impact", "wood", 640, 0.6, "灵木治疗爆发"),
]

COMMON_EFFECT_ASSETS = [
    ("Common", "Status", "NS_Status_Burning", "SFX_Status_Burning_Loop", "fire", 180, 1.4, "灼烧状态"),
    ("Common", "Status", "NS_Status_Wet", "SFX_Status_Wet", "water", 280, 0.9, "潮湿状态"),
    ("Common", "Status", "NS_Status_Shielded", "SFX_Status_Shielded", "earth", 160, 0.8, "护盾状态"),
    ("Common", "Status", "NS_Status_Slowed", "SFX_Status_Slowed", "water", 190, 0.7, "减速状态"),
    ("Common", "Status", "NS_Status_Stunned", "SFX_Status_Stunned", "impact", 110, 0.6, "眩晕状态"),
    ("Common", "Status", "NS_Status_ArmorBroken", "SFX_Status_ArmorBroken", "gold", 350, 0.55, "破甲状态"),
    ("Common", "Status", "NS_Status_HealingOverTime", "SFX_Status_HealingOverTime", "wood", 520, 1.2, "持续治疗状态"),

    ("Common", "Impact", "NS_Impact_Generic_Hit", "SFX_Impact_Generic_Hit", "impact", 130, 0.35, "通用命中"),
    ("Common", "Impact", "NS_Impact_Critical_Hit", "SFX_Impact_Critical_Hit", "gold", 700, 0.45, "暴击命中"),
    ("Common", "Impact", "NS_Impact_Heal_Burst", "SFX_Impact_Heal_Burst", "wood", 560, 0.55, "治疗爆发"),
]


# ============================================
# 目录构建
# ============================================

def get_vfx_package_path(hero_or_common, category):
    if hero_or_common == "Common":
        return "{}/DBA/VFX/Common/{}".format(CONTENT_PATH, category)

    return "{}/DBA/VFX/Abilities/{}".format(CONTENT_PATH, hero_or_common)


def get_sfx_package_path(hero_or_common, category):
    if hero_or_common == "Common":
        return "{}/DBA/Audio/SFX/Common/{}".format(CONTENT_PATH, category)

    return "{}/DBA/Audio/SFX/Abilities/{}".format(CONTENT_PATH, hero_or_common)


def create_required_folders():
    folders = [
        "{}/DBA".format(CONTENT_PATH),
        "{}/DBA/VFX".format(CONTENT_PATH),
        "{}/DBA/VFX/Abilities".format(CONTENT_PATH),
        "{}/DBA/VFX/Abilities/FireLion".format(CONTENT_PATH),
        "{}/DBA/VFX/Abilities/WaterDragon".format(CONTENT_PATH),
        "{}/DBA/VFX/Abilities/EarthBear".format(CONTENT_PATH),
        "{}/DBA/VFX/Abilities/GoldPhoenix".format(CONTENT_PATH),
        "{}/DBA/VFX/Abilities/WoodCrane".format(CONTENT_PATH),
        "{}/DBA/VFX/Common".format(CONTENT_PATH),
        "{}/DBA/VFX/Common/Status".format(CONTENT_PATH),
        "{}/DBA/VFX/Common/Impact".format(CONTENT_PATH),

        "{}/DBA/Audio".format(CONTENT_PATH),
        "{}/DBA/Audio/SFX".format(CONTENT_PATH),
        "{}/DBA/Audio/SFX/Abilities".format(CONTENT_PATH),
        "{}/DBA/Audio/SFX/Abilities/FireLion".format(CONTENT_PATH),
        "{}/DBA/Audio/SFX/Abilities/WaterDragon".format(CONTENT_PATH),
        "{}/DBA/Audio/SFX/Abilities/EarthBear".format(CONTENT_PATH),
        "{}/DBA/Audio/SFX/Abilities/GoldPhoenix".format(CONTENT_PATH),
        "{}/DBA/Audio/SFX/Abilities/WoodCrane".format(CONTENT_PATH),
        "{}/DBA/Audio/SFX/Common".format(CONTENT_PATH),
        "{}/DBA/Audio/SFX/Common/Status".format(CONTENT_PATH),
        "{}/DBA/Audio/SFX/Common/Impact".format(CONTENT_PATH),
    ]

    for folder in folders:
        ensure_folder_exists(folder)


# ============================================
# 生成逻辑
# ============================================

def create_effect_asset_row(row):
    hero_or_common, category, vfx_name, sfx_name, style, frequency, duration, desc = row

    log_info("=" * 80)
    log_info("生成特效/音效: {} | {}".format(vfx_name, sfx_name))
    log_info("说明: {}".format(desc))
    log_info("=" * 80)

    vfx_package = get_vfx_package_path(hero_or_common, category)
    sfx_package = get_sfx_package_path(hero_or_common, category)

    create_niagara_system(vfx_package, vfx_name)
    create_sound_wave(sfx_package, sfx_name, frequency, duration, style)


def create_all_effect_assets():
    for row in ABILITY_EFFECT_ASSETS:
        create_effect_asset_row(row)

    for row in COMMON_EFFECT_ASSETS:
        create_effect_asset_row(row)


# ============================================
# 汇总
# ============================================

def print_summary():
    log_info("=" * 80)
    log_info("VFX / SFX 生成结果汇总")
    log_info("=" * 80)

    all_rows = ABILITY_EFFECT_ASSETS + COMMON_EFFECT_ASSETS

    vfx_exists_count = 0
    sfx_exists_count = 0

    for row in all_rows:
        hero_or_common, category, vfx_name, sfx_name, style, frequency, duration, desc = row

        vfx_path = make_asset_path(get_vfx_package_path(hero_or_common, category), vfx_name)
        sfx_path = make_asset_path(get_sfx_package_path(hero_or_common, category), sfx_name)

        vfx_exists = unreal.EditorAssetLibrary.does_asset_exist(vfx_path)
        sfx_exists = unreal.EditorAssetLibrary.does_asset_exist(sfx_path)

        if vfx_exists:
            vfx_exists_count += 1

        if sfx_exists:
            sfx_exists_count += 1

        log_info("[VFX:{}] {}".format("存在" if vfx_exists else "缺失", vfx_path))
        log_info("[SFX:{}] {}".format("存在" if sfx_exists else "缺失", sfx_path))

    log_info("=" * 80)
    log_info("预期 VFX 数量: {}".format(len(all_rows)))
    log_info("存在 VFX 数量: {}".format(vfx_exists_count))
    log_info("预期 SFX 数量: {}".format(len(all_rows)))
    log_info("存在 SFX 数量: {}".format(sfx_exists_count))
    log_info("=" * 80)


# ============================================
# 主函数
# ============================================

def main():
    log_info("=" * 80)
    log_info("{} GenerateVFXSFXAssets.py 开始执行".format(PROJECT_NAME))
    log_info("=" * 80)

    log_warn("注意：本脚本会在创建前删除同名 NiagaraSystem / SoundWave 资产。")
    log_warn("NiagaraSystem 是占位资源，需要后续在 Niagara Editor 中制作发射器和模块。")
    log_warn("SoundWave 是脚本生成的临时音效，可作为占位音效，后续应替换为正式音频。")

    create_required_folders()
    create_all_effect_assets()
    print_summary()

    log_info("=" * 80)
    log_info("GenerateVFXSFXAssets.py 执行完成")
    log_info("=" * 80)

    log_info("后续建议：")
    log_info("1. 打开每个 NiagaraSystem，添加对应元素主题的 Emitter。")
    log_info("2. 将 Fire 系特效做成火焰、火星、热浪、灼烧残留。")
    log_info("3. 将 Water 系特效做成水流、泡沫、涟漪、治疗水波。")
    log_info("4. 将 Earth 系特效做成碎石、地裂、尘土、岩甲凝聚。")
    log_info("5. 将 Gold 系特效做成金羽、闪光、穿刺轨迹、锐利命中。")
    log_info("6. 将 Wood 系特效做成绿叶、藤蔓、生命粒子、治疗区域。")
    log_info("7. 用正式音频替换脚本生成的临时 wav。")
    log_info("8. 在 AbilitySet DataAsset 中增加 VFX/SFX 字段并引用这些资源。")


# ============================================
# 入口
# ============================================

if __name__ == "__main__":
    main()
