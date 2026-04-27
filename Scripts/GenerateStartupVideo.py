# Copyright FreeboozStudio. All Rights Reserved.
#
# GenerateStartupVideo.py
#
# 启动视频相关资产自动生成脚本
# 功能：
# 1. 创建视频资源目录结构
# 2. 创建 MediaPlayer 和 MediaTexture 资产
# 3. 创建 DBAStartupVideoWidget Blueprint
#
# 在 UE Editor 的 Python Console 中执行：
# py D:/DivineBeastsArena/Scripts/GenerateStartupVideo.py
#

import unreal


# ============================================
# 基础配置
# ============================================

CONTENT_PATH = "/Game"
PROJECT_NAME = "DivineBeastsArena"

# 视频资源存放目录
VIDEO_PATH = "{}/Movies/Startup".format(CONTENT_PATH)

# UI 资源存放目录
UI_PATH = "{}/UI/Startup".format(CONTENT_PATH)

# Media 资产名称
MEDIA_PLAYER_NAME = "MP_StartupVideo"
MEDIA_TEXTURE_NAME = "MT_StartupVideo"

# Widget Blueprint 名称
STARTUP_WIDGET_BP_NAME = "WBP_StartupVideo"


# ============================================
# 日志工具
# ============================================

def log_info(msg):
    unreal.log("[GenerateStartupVideo][INFO] {}".format(msg))


def log_warn(msg):
    unreal.log_warning("[GenerateStartupVideo][WARN] {}".format(msg))


def log_error(msg):
    unreal.log_error("[GenerateStartupVideo][ERROR] {}".format(msg))


# ============================================
# 路径工具
# ============================================

def normalize_directory_path(path):
    """规范化 UE 目录路径"""
    if not path:
        return "/Game"
    if not path.startswith("/"):
        path = "/" + path
    return path.rstrip("/")


def make_asset_path(package_path, asset_name):
    """拼接资产路径"""
    package_path = normalize_directory_path(package_path)
    return "{}/{}".format(package_path, asset_name)


def ensure_folder_exists(path):
    """确保目录存在"""
    path = normalize_directory_path(path)
    if unreal.EditorAssetLibrary.does_directory_exist(path):
        return True
    result = unreal.EditorAssetLibrary.make_directory(path)
    if result:
        log_info("创建文件夹: {}".format(path))
        return True
    log_error("创建文件夹失败: {}".format(path))
    return False


def delete_asset_if_exists(asset_path):
    """删除已存在的资产"""
    asset_path = normalize_directory_path(asset_path)
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        result = unreal.EditorAssetLibrary.delete_asset(asset_path)
        if result:
            log_info("已删除资产: {}".format(asset_path))
            return True
        else:
            log_error("删除资产失败: {}".format(asset_path))
            return False
    return True


# ============================================
# 创建目录结构
# ============================================

def create_folders():
    """创建视频和 UI 相关目录结构"""
    log_info("=" * 60)
    log_info("开始创建目录结构")
    log_info("=" * 60)

    folders = [
        VIDEO_PATH,
        "{}/Videos".format(VIDEO_PATH),
        "{}/Thumbnails".format(VIDEO_PATH),
        UI_PATH,
    ]

    success = True
    for folder in folders:
        if not ensure_folder_exists(folder):
            success = False

    return success


# ============================================
# 资产创建工具 (UE5 API)
# ============================================

def create_asset_with_asset_tools(asset_path, asset_class, factory_class=None):
    """使用 AssetTools 创建资产 (UE5 兼容)"""
    asset_path = normalize_directory_path(asset_path)

    # 获取 AssetTools
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # 创建资产
    if factory_class:
        factory = unreal.Factory()
        result = asset_tools.create_asset(
            asset_name=asset_path.split("/")[-1],
            package_path=asset_path.rsplit("/", 1)[0],
            asset_class=asset_class,
            factory=factory
        )
    else:
        result = asset_tools.create_asset(
            asset_name=asset_path.split("/")[-1],
            package_path=asset_path.rsplit("/", 1)[0],
            asset_class=asset_class
        )

    return result


# ============================================
# 创建 MediaPlayer 资产
# ============================================

def create_media_player():
    """创建 MediaPlayer 资产"""
    log_info("=" * 60)
    log_info("创建 MediaPlayer 资产")
    log_info("=" * 60)

    asset_path = make_asset_path(VIDEO_PATH, MEDIA_PLAYER_NAME)

    # 删除已存在的资产
    delete_asset_if_exists(asset_path)

    # 使用 AssetTools 创建资产
    asset = create_asset_with_asset_tools(asset_path, unreal.MediaPlayer)

    if asset:
        log_info("创建 MediaPlayer 成功: {}".format(asset_path))
        return True
    else:
        log_error("创建 MediaPlayer 失败: {}".format(asset_path))
        return False


# ============================================
# 创建 MediaTexture 资产
# ============================================

def create_media_texture():
    """创建 MediaTexture 资产"""
    log_info("=" * 60)
    log_info("创建 MediaTexture 资产")
    log_info("=" * 60)

    asset_path = make_asset_path(VIDEO_PATH, MEDIA_TEXTURE_NAME)

    # 删除已存在的资产
    delete_asset_if_exists(asset_path)

    # 使用 AssetTools 创建资产
    asset = create_asset_with_asset_tools(asset_path, unreal.MediaTexture)

    if asset:
        log_info("创建 MediaTexture 成功: {}".format(asset_path))
        return True
    else:
        log_error("创建 MediaTexture 失败: {}".format(asset_path))
        return False


# ============================================
# 创建 Widget Blueprint
# ============================================

def create_startup_video_widget():
    """创建启动视频 Widget Blueprint"""
    log_info("=" * 60)
    log_info("创建启动视频 Widget Blueprint")
    log_info("=" * 60)

    asset_path = make_asset_path(UI_PATH, STARTUP_WIDGET_BP_NAME)

    # 删除已存在的资产
    delete_asset_if_exists(asset_path)

    # 获取 AssetTools
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # 使用 BlueprintFactory 创建 Widget Blueprint
    blueprint_factory = unreal.BlueprintFactory()

    # 创建 Widget Blueprint
    result = asset_tools.create_asset(
        asset_name=STARTUP_WIDGET_BP_NAME,
        package_path=UI_PATH,
        asset_class=unreal.WidgetBlueprint,
        factory=blueprint_factory
    )

    if result:
        log_info("创建 Widget Blueprint 成功: {}".format(asset_path))
        return True
    else:
        log_error("创建 Widget Blueprint 失败: {}".format(asset_path))
        return False


# ============================================
# 打印资源清单
# ============================================

def print_asset_list():
    """Print created asset list"""
    log_info("=" * 60)
    log_info("Asset List")
    log_info("=" * 60)

    log_info("Video Path: {}".format(VIDEO_PATH))
    log_info("  - Videos/: Store startup video files (.mp4)")
    log_info("  - Thumbnails/: Store video thumbnails")
    log_info("  - {}: MediaPlayer asset".format(MEDIA_PLAYER_NAME))
    log_info("  - {}: MediaTexture asset".format(MEDIA_TEXTURE_NAME))

    log_info("")
    log_info("UI Path: {}".format(UI_PATH))
    log_info("  - {}: Startup video widget blueprint".format(STARTUP_WIDGET_BP_NAME))

    log_info("")
    log_info("=" * 60)
    log_info("Next Steps")
    log_info("=" * 60)
    log_info("1. Put startup video file into: {}/Videos/".format(VIDEO_PATH))
    log_info("2. Set video URL in MediaPlayer asset")
    log_info("3. Design UI layout in WBP_StartupVideo")
    log_info("4. Configure StartupVideoPath in DBAStartupVideoSubsystem")


# ============================================
# 主函数
# ============================================

def main():
    log_info("=" * 60)
    log_info("Startup Video Asset Generation Script Started")
    log_info("=" * 60)

    # 1. Create folder structure
    if not create_folders():
        log_error("Failed to create folder structure")
        return

    # 2. Create MediaPlayer
    if not create_media_player():
        log_error("Failed to create MediaPlayer")

    # 3. Create MediaTexture
    if not create_media_texture():
        log_error("Failed to create MediaTexture")

    # 4. Create Widget Blueprint
    if not create_startup_video_widget():
        log_error("Failed to create Widget Blueprint")

    # 5. Print asset list
    print_asset_list()

    # 6. Force GC
    unreal.EditorAssetLibrary.collect_garbage()

    log_info("=" * 60)
    log_info("Startup Video Asset Generation Script Completed")
    log_info("=" * 60)


if __name__ == "__main__":
    main()
