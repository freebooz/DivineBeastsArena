#!/bin/bash
# Copyright FreeboozStudio. All Rights Reserved.
# 使用源代码引擎编译项目 (Linux/macOS)

# 引擎路径
ENGINE_ROOT="${ENGINE_ROOT:-/mnt/e/UnrealEngine-5.7.1-release}"
ENGINE_BUILD="$ENGINE_ROOT/Engine/Build/BatchFiles"

# 项目路径
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PROJECT_NAME="DivineBeastsArena"

echo "========================================"
echo " 神兽竞技场 - Development 构建脚本"
echo "========================================"
echo ""

# 检查引擎路径
if [ ! -f "$ENGINE_BUILD/Build.sh" ]; then
    echo "[错误] 未找到引擎 Build.sh: $ENGINE_BUILD/Build.sh"
    echo "请确认引擎路径配置正确 (ENGINE_ROOT=$ENGINE_ROOT)"
    exit 1
fi

# 编译参数
BUILD_CONFIG="${1:-Development}"
BUILD_TARGET="${2:-Editor}"

echo "[1/2] 正在编译 $PROJECT_NAME ($BUILD_CONFIG | $BUILD_TARGET)..."
echo ""

cd "$PROJECT_ROOT"

# 执行编译
"$ENGINE_BUILD/Build.sh" "$BUILD_TARGET" "$PROJECT_NAME" "$BUILD_CONFIG" \
    -Project="$PROJECT_ROOT/$PROJECT_NAME.uproject" \
    -BuildMachine \
    -NoEngineChanges

if [ $? -ne 0 ]; then
    echo ""
    echo "[错误] 编译失败"
    exit 1
fi

echo ""
echo "[2/2] 编译成功!"
echo ""
echo "========================================"
echo " 构建完成"
echo "========================================"
