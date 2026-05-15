# 登录/选角/创角 UI 蓝图规范（关卡三维模型版）

## 1. 流程目标

- 登录界面使用蓝图 `WBP_DBA_Login`（含用户名/密码输入框和背景纹理）。
- 角色选择与角色创建使用蓝图UI承载交互控件。
- 角色三维模型和光照全部由关卡中的 `ADBACharacterPresentationActor` 提供，不走 UMG Viewport 渲染。

---

## 2. 登录界面（WBP_DBA_Login）

### 2.1 线框布局（1920x1080参考）

```text
+----------------------------------------------------------------------------------+
|  BackgroundTexture: T_DBA_LoginForestSanctuary                                  |
|                                                                                  |
|                                                            +------------------+  |
|                                                            | LoginPanel       |  |
|                                                            |------------------|  |
|                                                            | TitleText        |  |
|                                                            | EmailInput       |  |
|                                                            | PasswordInput    |  |
|                                                            | LoginButton      |  |
|                                                            | GuestLoginButton |  |
|                                                            | ErrorText        |  |
|                                                            | StatusText       |  |
|                                                            +------------------+  |
+----------------------------------------------------------------------------------+
```

### 2.2 组件层级（Widget Tree）

```text
WBP_DBA_Login (UserWidget)
└─ RootCanvas (CanvasPanel)
   ├─ BG_Login (Image)                            // 全屏背景纹理
   └─ LoginPanel (Border / Overlay)              // 右侧登录面板（BindWidgetOptional）
      └─ LoginVBox (VerticalBox)
         ├─ TitleText (TextBlock)                // BindWidgetOptional
         ├─ EmailInput (EditableTextBox)         // BindWidgetOptional
         ├─ PasswordInput (EditableTextBox)      // BindWidgetOptional
         ├─ LoginButton (Button)                 // BindWidgetOptional
         ├─ GuestLoginButton (Button)            // BindWidgetOptional
         ├─ ErrorText (TextBlock)                // BindWidgetOptional
         └─ StatusText (TextBlock)               // BindWidgetOptional
```

### 2.3 关键参数

| 组件 | 参数 | 建议值 |
|---|---|---|
| `BG_Login` | Anchors | Min(0,0), Max(1,1) |
| `BG_Login` | Brush | `T_DBA_LoginForestSanctuary` |
| `LoginPanel` | Anchors | Min(0.66,0.14), Max(0.95,0.90) |
| `LoginPanel` | Offset | 0（由Anchor控制） |
| `EmailInput` | HintText | `用户名 / 邮箱` |
| `PasswordInput` | IsPassword | `true` |
| `LoginButton` | Text | `进入游戏` |
| `GuestLoginButton` | Text | `游客登录` |
| `ErrorText` | Visibility | 默认 `Collapsed` |
| `StatusText` | Visibility | 默认 `Collapsed` |

---

## 3. 角色选择界面（WBP_DBA_CharacterSelect）

### 3.1 线框布局（1920x1080参考）

```text
+----------------------------------------------------------------------------------+
|  TopBar: 标题/返回                                                               |
|                                                                                  |
|  +----------------------------+        +--------------------------------------+  |
|  | LeftPanel                  |        | CenterStage (3D关卡镜头)             |  |
|  | CharacterListText          |        | CharacterPreviewHost (透明占位)      |  |
|  | RefreshButton              |        | 拖拽旋转热区                         |  |
|  +----------------------------+        +--------------------------------------+  |
|                                                                                  |
|                                      +-----------------------------+             |
|                                      | RightPanel                  |             |
|                                      | ConfirmButton(进入大厅)     |             |
|                                      | CreateButton(创建角色)      |             |
|                                      | StatusText                  |             |
|                                      +-----------------------------+             |
+----------------------------------------------------------------------------------+
```

### 3.2 组件层级（Widget Tree）

```text
WBP_DBA_CharacterSelect (UserWidget)
└─ RootCanvas
   ├─ LeftPanel (Border)
   │  └─ LeftVBox
   │     ├─ CharacterListText (TextBlock)        // BindWidgetOptional
   │     └─ RefreshButton (Button)               // BindWidgetOptional
   ├─ CharacterPreviewHost (Border/Overlay)      // BindWidgetOptional（透明，仅事件区域）
   └─ RightPanel (Border)
      └─ RightVBox
         ├─ ConfirmButton (Button)               // BindWidgetOptional
         ├─ CreateButton (Button)                // BindWidgetOptional
         └─ StatusText (TextBlock)               // BindWidgetOptional
```

### 3.3 关键参数

| 组件 | 参数 | 建议值 |
|---|---|---|
| `CharacterPreviewHost` | Anchors | Min(0.24,0.05), Max(0.76,0.95) |
| `CharacterPreviewHost` | RenderOpacity | `0.0`（显示交给关卡镜头） |
| `CharacterPreviewHost` | Visibility | `SelfHitTestInvisible` |
| `ConfirmButton` | Text | `进入大厅` |
| `CreateButton` | Text | `创建角色` |
| `RefreshButton` | Text | `刷新` |
| `PreviewDragRotationDegreesPerPixel` | C++默认 | `0.28` |

---

## 4. 角色创建界面（WBP_DBA_CharacterCreate）

### 4.1 线框布局（1920x1080参考）

```text
+----------------------------------------------------------------------------------+
|                                CenterStage (3D关卡镜头)                         |
|                                CharacterPreviewHost 透明热区                    |
|                                                                                  |
|  +-----------------------------+                                                 |
|  | CreatePanel                 |                                                 |
|  | CharacterNameInput          |                                                 |
|  | ZodiacButton + ZodiacText   |                                                 |
|  | ElementButton + ElementText |                                                 |
|  | FiveCampButton + FiveCampText                                                |
|  | ValidationText              |                                                 |
|  | CreateButton                |                                                 |
|  | BackButton                  |                                                 |
|  +-----------------------------+                                                 |
+----------------------------------------------------------------------------------+
```

### 4.2 组件层级（Widget Tree）

```text
WBP_DBA_CharacterCreate (UserWidget)
└─ RootCanvas
   ├─ CharacterPreviewHost (Border/Overlay)      // BindWidgetOptional（透明，仅事件区域）
   └─ CreatePanel (Border)
      └─ FormVBox
         ├─ CharacterNameInput (EditableTextBox) // BindWidgetOptional
         ├─ ZodiacButton (Button)                // BindWidgetOptional
         │  └─ ZodiacText (TextBlock)            // BindWidgetOptional
         ├─ ElementButton (Button)               // BindWidgetOptional
         │  └─ ElementText (TextBlock)           // BindWidgetOptional
         ├─ FiveCampButton (Button)              // BindWidgetOptional
         │  └─ FiveCampText (TextBlock)          // BindWidgetOptional
         ├─ ValidationText (TextBlock)           // BindWidgetOptional
         ├─ CreateButton (Button)                // BindWidgetOptional
         └─ BackButton (Button)                  // BindWidgetOptional
```

### 4.3 关键参数

| 组件 | 参数 | 建议值 |
|---|---|---|
| `CharacterPreviewHost` | Anchors | Min(0.24,0.05), Max(0.76,0.95) |
| `CharacterPreviewHost` | RenderOpacity | `0.0` |
| `CharacterNameInput` | HintText | `输入角色名` |
| `ZodiacButton` | 点击行为 | 轮换 `Rat -> ... -> Pig` |
| `ElementButton` | 点击行为 | 轮换 `Water/Fire/Wood/Gold/Earth` |
| `FiveCampButton` | 点击行为 | 轮换 `None/East/West/South/North/Center` |
| `CreateButton` | Text | `创建并进入` |
| `BackButton` | Text | `返回角色选择` |
| `ValidationText` | 默认文案 | `Enter a character name.` |

---

## 5. 关卡三维展示 Actor 规格（ADBACharacterPresentationActor）

> 以下为当前C++已实现的参考值，蓝图关卡摆放可直接对齐。

### 5.1 相机与舞台

| 项 | 值 |
|---|---|
| CameraFOV | `38.0` |
| CameraLocation | `(230, 0, 92)` |
| CameraRotation | `(-8, 180, 0)` |
| GroundScale | `(7.5, 7.5, 1.0)` |
| 模型缩放 | `3.75` |

### 5.2 灯光

| 灯光 | 强度 | 颜色倾向 |
|---|---|---|
| KeyLight | `82000` | 暖色 |
| FillLight | `24500` | 偏冷 |
| RimLight | `42000` | 冷白 |
| FaceLight | `15000` | 暖色补光 |
| SkyLight | `3.8` | 冷色环境 |

### 5.3 雾效

| 项 | 值 |
|---|---|
| FogDensity | `0.028` |
| FogHeightFalloff | `0.18` |
| StartDistance | `72` |
| FogColor | `(0.20, 0.42, 0.36, 1.0)` |

---

## 6. 流程验证检查清单

1. 启动后 `Splash -> Login`，显示 `WBP_DBA_Login`，用户名/密码输入框可见。  
2. 点击游客登录后进入 `WBP_DBA_CharacterSelect`。  
3. `CharacterPreviewHost` 区域能拖拽旋转角色（实际模型来自关卡镜头）。  
4. 点击创建角色进入 `WBP_DBA_CharacterCreate`。  
5. 创建成功后返回选择并可进入大厅。  
6. 日志中不应出现 `MM_Idle ... Invalid USkeleton supplied`。  

