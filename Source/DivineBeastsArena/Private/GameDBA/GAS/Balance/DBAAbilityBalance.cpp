// Copyright Freebooz Games, Inc. All Rights Reserved.
// 神兽竞技场 - 十二生肖数值平衡配置实现

#include "GameDBA/GAS/Balance/DBAAbilityBalance.h"

UDBAHeroBalanceConfig::UDBAHeroBalanceConfig()
{
	// ==================== 子鼠·夜影灵牙｜影牙 ====================
	Rat_ShadowFang.ZodiacType = EDBAZodiacType::Rat;
	Rat_ShadowFang.CharacterName = TEXT("子鼠·夜影灵牙");
	Rat_ShadowFang.ShortName = TEXT("影牙");
	Rat_ShadowFang.CoreRole = TEXT("潜行刺客 / 侦察收割");
	Rat_ShadowFang.CoreStats = FDBAHeroCoreStats{2, 5, 2, 5, 2, 5, 3};
	Rat_ShadowFang.PositionInfo = FDBAHeroPositionInfo{TEXT("打野 / 游走"), TEXT("探穴侦察、绕后切入、残血收尾")};
	Rat_ShadowFang.Advantages = TEXT("爆发高、侦察灵活、收尾强");
	Rat_ShadowFang.Weaknesses = TEXT("身板脆、依赖时机");
	Rat_ShadowFang.BestPartners = TEXT("金翎、天犬、铁角");

	// ==================== 丑牛·撼山铁角｜铁角 ====================
	Ox_Ironhorn.ZodiacType = EDBAZodiacType::Ox;
	Ox_Ironhorn.CharacterName = TEXT("丑牛·撼山铁角");
	Ox_Ironhorn.ShortName = TEXT("铁角");
	Ox_Ironhorn.CoreRole = TEXT("重装坦克 / 开团先锋");
	Ox_Ironhorn.CoreStats = FDBAHeroCoreStats{5, 2, 5, 2, 4, 3, 5};
	Ox_Ironhorn.PositionInfo = FDBAHeroPositionInfo{TEXT("上路 / 辅助前排"), TEXT("正面开团、举盾护队、反身保护")};
	Ox_Ironhorn.Advantages = TEXT("开团强、承伤高、保护稳");
	Ox_Ironhorn.Weaknesses = TEXT("机动低、输出低");
	Ox_Ironhorn.BestPartners = TEXT("苍龙、玉灵、玉角");

	// ==================== 寅虎·啸山白虎｜白虎 ====================
	Tiger_WhiteTiger.ZodiacType = EDBAZodiacType::Tiger;
	Tiger_WhiteTiger.CharacterName = TEXT("寅虎·啸山白虎");
	Tiger_WhiteTiger.ShortName = TEXT("白虎");
	Tiger_WhiteTiger.CoreRole = TEXT("爆发战士 / 目标压制");
	Tiger_WhiteTiger.CoreStats = FDBAHeroCoreStats{3, 5, 3, 4, 1, 4, 4};
	Tiger_WhiteTiger.PositionInfo = FDBAHeroPositionInfo{TEXT("上路 / 打野"), TEXT("侧翼突进、单点压制、追击收割")};
	Tiger_WhiteTiger.Advantages = TEXT("单点爆发强、追击强");
	Tiger_WhiteTiger.Weaknesses = TEXT("怕被集火控制");
	Tiger_WhiteTiger.BestPartners = TEXT("金翎、雷蹄、玉角");

	// ==================== 卯兔·踏月玉灵｜玉灵 ====================
	Rabbit_MoonSpirit.ZodiacType = EDBAZodiacType::Rabbit;
	Rabbit_MoonSpirit.CharacterName = TEXT("卯兔·踏月玉灵");
	Rabbit_MoonSpirit.ShortName = TEXT("玉灵");
	Rabbit_MoonSpirit.CoreRole = TEXT("机动输出 / 月影拉扯");
	Rabbit_MoonSpirit.CoreStats = FDBAHeroCoreStats{2, 4, 2, 5, 2, 5, 3};
	Rabbit_MoonSpirit.PositionInfo = FDBAHeroPositionInfo{TEXT("中路 / 游走"), TEXT("位移拉扯、月影迷惑、持续消耗")};
	Rabbit_MoonSpirit.Advantages = TEXT("灵活、拉扯强、操作上限高");
	Rabbit_MoonSpirit.Weaknesses = TEXT("容错低、怕硬控");
	Rabbit_MoonSpirit.BestPartners = TEXT("天犬、玉角、铁角");

	// ==================== 辰龙·御雷苍龙｜苍龙 ====================
	Dragon_ThunderLord.ZodiacType = EDBAZodiacType::Dragon;
	Dragon_ThunderLord.CharacterName = TEXT("辰龙·御雷苍龙");
	Dragon_ThunderLord.ShortName = TEXT("苍龙");
	Dragon_ThunderLord.CoreRole = TEXT("法师核心 / 雷云控场");
	Dragon_ThunderLord.CoreStats = FDBAHeroCoreStats{3, 5, 4, 2, 3, 4, 5};
	Dragon_ThunderLord.PositionInfo = FDBAHeroPositionInfo{TEXT("中路"), TEXT("雷云控场、团战法核、雷门辅助")};
	Dragon_ThunderLord.Advantages = TEXT("团战输出强、控场强");
	Dragon_ThunderLord.Weaknesses = TEXT("依赖站位和预判");
	Dragon_ThunderLord.BestPartners = TEXT("铁角、幽鳞、玉角");

	// ==================== 巳蛇·幽毒灵蛇｜幽鳞 ====================
	Snake_VenomScale.ZodiacType = EDBAZodiacType::Snake;
	Snake_VenomScale.CharacterName = TEXT("巳蛇·幽毒灵蛇");
	Snake_VenomScale.ShortName = TEXT("幽鳞");
	Snake_VenomScale.CoreRole = TEXT("灵动控场 / 区域节奏");
	Snake_VenomScale.CoreStats = FDBAHeroCoreStats{3, 3, 5, 4, 2, 4, 5};
	Snake_VenomScale.PositionInfo = FDBAHeroPositionInfo{TEXT("中路 / 辅助控制"), TEXT("区域控场、蛇纹减速、优雅脱身")};
	Snake_VenomScale.Advantages = TEXT("区域控制强、节奏压制强");
	Snake_VenomScale.Weaknesses = TEXT("爆发一般");
	Snake_VenomScale.BestPartners = TEXT("苍龙、铁角、金翎");

	// ==================== 午马·赤焰雷蹄｜雷蹄 ====================
	Horse_ThunderHoof.ZodiacType = EDBAZodiacType::Horse;
	Horse_ThunderHoof.CharacterName = TEXT("午马·赤焰雷蹄");
	Horse_ThunderHoof.ShortName = TEXT("雷蹄");
	Horse_ThunderHoof.CoreRole = TEXT("高机动先锋 / 跑图支援");
	Horse_ThunderHoof.CoreStats = FDBAHeroCoreStats{3, 4, 3, 5, 4, 3, 4};
	Horse_ThunderHoof.PositionInfo = FDBAHeroPositionInfo{TEXT("打野 / 上路"), TEXT("快速支援、路径铺设、远程开团")};
	Horse_ThunderHoof.Advantages = TEXT("支援快、开团好、节奏强");
	Horse_ThunderHoof.Weaknesses = TEXT("持续站场一般");
	Horse_ThunderHoof.BestPartners = TEXT("白虎、玉角、天犬");

	// ==================== 未羊·玉角灵铃｜玉角 ====================
	Goat_JadeBell.ZodiacType = EDBAZodiacType::Goat;
	Goat_JadeBell.CharacterName = TEXT("未羊·玉角灵铃");
	Goat_JadeBell.ShortName = TEXT("玉角");
	Goat_JadeBell.CoreRole = TEXT("治疗辅助 / 团队保护");
	Goat_JadeBell.CoreStats = FDBAHeroCoreStats{3, 1, 2, 3, 5, 3, 5};
	Goat_JadeBell.PositionInfo = FDBAHeroPositionInfo{TEXT("辅助"), TEXT("治疗、护盾、净化、团队祝福")};
	Goat_JadeBell.Advantages = TEXT("团队续航强、保护强");
	Goat_JadeBell.Weaknesses = TEXT("输出低、依赖队友");
	Goat_JadeBell.BestPartners = TEXT("铁角、獠牙、白虎");

	// ==================== 申猴·百戏灵猴｜灵猴 ====================
	Monkey_Trickster.ZodiacType = EDBAZodiacType::Monkey;
	Monkey_Trickster.CharacterName = TEXT("申猴·百戏灵猴");
	Monkey_Trickster.ShortName = TEXT("灵猴");
	Monkey_Trickster.CoreRole = TEXT("高机动扰乱 / 假身换位");
	Monkey_Trickster.CoreStats = FDBAHeroCoreStats{2, 4, 3, 5, 1, 5, 4};
	Monkey_Trickster.PositionInfo = FDBAHeroPositionInfo{TEXT("打野 / 游走"), TEXT("假身换位、连跳扰乱、后排干扰")};
	Monkey_Trickster.Advantages = TEXT("操作秀、扰乱强、机动高");
	Monkey_Trickster.Weaknesses = TEXT("容错低、怕稳定控制");
	Monkey_Trickster.BestPartners = TEXT("金翎、天犬、玉角");

	// ==================== 酉鸡·破晓金翎｜金翎 ====================
	Rooster_DawnBringer.ZodiacType = EDBAZodiacType::Rooster;
	Rooster_DawnBringer.CharacterName = TEXT("酉鸡·破晓金翎");
	Rooster_DawnBringer.ShortName = TEXT("金翎");
	Rooster_DawnBringer.CoreRole = TEXT("侦测辅助 / 视野控制");
	Rooster_DawnBringer.CoreStats = FDBAHeroCoreStats{3, 2, 3, 3, 5, 3, 4};
	Rooster_DawnBringer.PositionInfo = FDBAHeroPositionInfo{TEXT("辅助"), TEXT("视野预警、显形照场、反埋伏")};
	Rooster_DawnBringer.Advantages = TEXT("反隐强、视野强、团队价值高");
	Rooster_DawnBringer.Weaknesses = TEXT("正面伤害不足");
	Rooster_DawnBringer.BestPartners = TEXT("影牙、白虎、灵猴");

	// ==================== 戌狗·守门天犬｜天犬 ====================
	Dog_SkyGuardian.ZodiacType = EDBAZodiacType::Dog;
	Dog_SkyGuardian.CharacterName = TEXT("戌狗·守门天犬");
	Dog_SkyGuardian.ShortName = TEXT("天犬");
	Dog_SkyGuardian.CoreRole = TEXT("守护辅助 / 反突进");
	Dog_SkyGuardian.CoreStats = FDBAHeroCoreStats{4, 2, 4, 3, 5, 3, 5};
	Dog_SkyGuardian.PositionInfo = FDBAHeroPositionInfo{TEXT("辅助 / 上路"), TEXT("护主救援、反突进、守门结界")};
	Dog_SkyGuardian.Advantages = TEXT("保护强、反突进强");
	Dog_SkyGuardian.Weaknesses = TEXT("开团不如铁角");
	Dog_SkyGuardian.BestPartners = TEXT("玉灵、苍龙、金翎");

	// ==================== 亥猪·岩甲獠牙｜獠牙 ====================
	Pig_StoneTusk.ZodiacType = EDBAZodiacType::Pig;
	Pig_StoneTusk.CharacterName = TEXT("亥猪·岩甲獠牙");
	Pig_StoneTusk.ShortName = TEXT("獠牙");
	Pig_StoneTusk.CoreRole = TEXT("站场坦克 / 稳定承伤");
	Pig_StoneTusk.CoreStats = FDBAHeroCoreStats{5, 3, 4, 2, 3, 2, 5};
	Pig_StoneTusk.PositionInfo = FDBAHeroPositionInfo{TEXT("上路 / 前排辅助"), TEXT("稳定站场、岩甲承伤、福印稳阵")};
	Pig_StoneTusk.Advantages = TEXT("站场强、耐打、团战稳定");
	Pig_StoneTusk.Weaknesses = TEXT("机动低、手短");
	Pig_StoneTusk.BestPartners = TEXT("玉角、苍龙、金翎");
}

FDBAHeroBalanceData UDBAHeroBalanceConfig::GetHeroBalanceData(EDBAZodiacType ZodiacType) const
{
	switch (ZodiacType)
	{
	case EDBAZodiacType::Rat: return Rat_ShadowFang;
	case EDBAZodiacType::Ox: return Ox_Ironhorn;
	case EDBAZodiacType::Tiger: return Tiger_WhiteTiger;
	case EDBAZodiacType::Rabbit: return Rabbit_MoonSpirit;
	case EDBAZodiacType::Dragon: return Dragon_ThunderLord;
	case EDBAZodiacType::Snake: return Snake_VenomScale;
	case EDBAZodiacType::Horse: return Horse_ThunderHoof;
	case EDBAZodiacType::Goat: return Goat_JadeBell;
	case EDBAZodiacType::Monkey: return Monkey_Trickster;
	case EDBAZodiacType::Rooster: return Rooster_DawnBringer;
	case EDBAZodiacType::Dog: return Dog_SkyGuardian;
	case EDBAZodiacType::Pig: return Pig_StoneTusk;
	default: return FDBAHeroBalanceData{};
	}
}

TArray<FDBAHeroBalanceData> UDBAHeroBalanceConfig::GetAllHeroBalanceData() const
{
	return {
		Rat_ShadowFang,
		Ox_Ironhorn,
		Tiger_WhiteTiger,
		Rabbit_MoonSpirit,
		Dragon_ThunderLord,
		Snake_VenomScale,
		Horse_ThunderHoof,
		Goat_JadeBell,
		Monkey_Trickster,
		Rooster_DawnBringer,
		Dog_SkyGuardian,
		Pig_StoneTusk
	};
}