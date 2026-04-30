# Divine Beasts Arena 用户界面蓝图配置清单

## 概述

本文档列出所有用户界面蓝图及其继承关系，方便设计师编辑。

---

## 一、蓝图架构层级

```
WidgetController (抽象基类)
    │
    ├── MenuWidgetController (菜单控制器)
    │       └── DBAClientWidgetController (客户端控制器)
    │
    └── BattleWidgetController (战斗控制器)
            └── DBAClientWidgetController (客户端控制器)
```

---

## 二、Widget Controller 一览

### 2.1 客户端控制器

| 类名 | 头文件 | 说明 |
|------|--------|------|
| UDBAClientWidgetController | DBA/WidgetController/DBAClientWidgetController.h | 客户端通用控制器 |

**继承关系**: `UDBAClientWidgetController` ← `UMenuWidgetController` ← `UWidgetController`

### 2.2 菜单控制器

| 类名 | 头文件 | 说明 |
|------|--------|------|
| UMenuWidgetController | DBA/WidgetController/MenuWidgetController.h | 菜单系统控制器 |

---

## 三、Widget Component 一览

### 3.1 属性组件

| 类名 | 头文件 | 说明 |
|------|--------|------|
| UDBAHealthWidgetComponent | DBA/UI/WidgetComponent/DBAHealthWidgetComponent.h | 生命值显示组件 |
| UDBAEnergyWidgetComponent | DBA/UI/WidgetComponent/DBAEnergyWidgetComponent.h | 能量显示组件 |
| UDBAFeedbackPopupComponent | DBA/UI/WidgetComponent/DBAFeedbackPopupComponent.h | 反馈弹出组件 |

---

## 四、Widget 一览

### 4.1 用户头像 Widget

| 类名 | 头文件 | 说明 |
|------|--------|------|
| UDBAUserAvatarWidget | DBA/UI/Widget/DBAUserAvatarWidget.h | 用户头像显示 |

---

## 五、UI 文件路径对照

### 5.1 Widget Controller

```
Source/DivineBeastsArena/Public/DBA/WidgetController/
├── WidgetController.h              (基类)
├── MenuWidgetController.h          (菜单控制器)
└── DBAClientWidgetController.h    (客户端控制器)
```

### 5.2 Widget Component

```
Source/DivineBeastsArena/Public/DBA/UI/WidgetComponent/
├── DBAHealthWidgetComponent.h      (生命值组件)
├── DBAEnergyWidgetComponent.h      (能量组件)
├── DBAFeedbackPopupComponent.h     (反馈弹出组件)
└── DBAWidgetComponent.h            (基类)
```

### 5.3 Widget

```
Source/DivineBeastsArena/Public/DBA/UI/Widget/
└── DBAUserAvatarWidget.h           (用户头像)
```

---

## 六、蓝图生成说明

### 6.1 蓝图继承关系
设计师在 UE 编辑器中创建蓝图时：
1. **WidgetController 蓝图**: 继承自 `WidgetController` 或其子类
2. **Widget 蓝图**: 继承自 `UUserWidget`

### 6.2 Widget Controller 使用流程
1. 在 Actor 或 Pawn 中添加 `UWidgetComponent`
2. 设置 Widget Class 为对应的 Widget 蓝图
3. 通过 `SetWidgetController()` 设置控制器

---

## 七、设计师编辑指南

### 7.1 创建新的 Widget Controller
1. 创建新类继承自 `UWidgetController` 或其子类
2. 在 `.h` 中声明 `TSubclassOf<UUserWidget> WidgetClass`
3. 在 `.cpp` 中使用 `CreateWidget()` 创建 Widget 实例

### 7.2 修改现有 Widget
1. 在 UE 编辑器中打开对应的 Widget 蓝图
2. 修改布局、外观、动画等
3. 重要：保持 `WidgetController` 的绑定接口不变

### 7.3 数据绑定
Widget 通过 WidgetController 获取数据：
```cpp
// 在 Widget 中
void UDBAUserAvatarWidget::NativeConstruct() {
    Super::NativeConstruct();
    if (WidgetController) {
        // 绑定数据
    }
}
```

---

*本文档由脚本自动生成，最后更新: 2026-04-30*