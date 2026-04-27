# 自然元素之力命名策略

## 一、底层键名与显示名称分离

### 底层键名（代码中使用）
- `Metal` - 金
- `Wood` - 木
- `Water` - 水
- `Fire` - 火
- `Earth` - 土

### 显示名称（UI 中显示）
- 金
- 木
- 水
- 火
- 土

## 二、Gold 源命名兼容

### 问题
- 英文中 "Gold" 通常指黄金、金币
- 五行中的 "金" 指金属元素，不仅限于黄金
- 为避免混淆，底层使用 `Metal` 而非 `Gold`

### 兼容策略
```cpp
// 枚举定义
enum class EDBAElementType : uint8
{
    Metal,  // 金元素，不使用 Gold
    // ...
};

// 显示名称映射
UMETA(DisplayName = "金")  // UI 显示为"金"

// GameplayTag
Element.Metal  // Tag 使用 Metal
```

### 数据表兼容
```
// DT_Elements.csv
RowName,DisplayName,Description
Metal,金,金属元素，克制木
Wood,木,木元素，克制土
...
```

### 本地化兼容
```
// ST_Elements.uasset
Key: Element.Metal.Name
zh-Hans: 金
en: Metal

Key: Element.Metal.Description
zh-Hans: 金属元素，锋利、坚硬
en: Metal element, sharp and solid
```

## 三、命名一致性检查清单

- [ ] 枚举使用 `Metal`
- [ ] GameplayTag 使用 `Element.Metal`
- [ ] 数据表 RowName 使用 `Metal`
- [ ] UI 显示使用 `DisplayName = "金"`
- [ ] 本地化键使用 `Element.Metal.*`
- [ ] 代码注释使用 "金元素" 或 "Metal"
- [ ] 文档统一使用 "自然元素之力" 而非 "五行"