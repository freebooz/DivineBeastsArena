using Game.Application.Characters;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Characters;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Tests;

public class CharacterRosterSecurityTests
{
    [Fact]
    public async Task CharacterOwnedByAccountA_CannotBeReadDeletedOrSelectedByAccountB()
    {
        await using var db = new GameDbContext(new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"character-idor-{Guid.NewGuid()}").Options);
        var accountA = Guid.NewGuid();
        var accountB = Guid.NewGuid();
        var characterId = Guid.NewGuid();
        db.PlayerCharacters.Add(new PlayerCharacter { Id = characterId, PlayerId = accountA, ServerId = Guid.NewGuid(), CharacterName = "安全角色", NormalizedName = "安全角色", Zodiac = "Rat", PrimaryElement = "Water", FiveCamp = "East", FixedSkillGroupId = "Rat_Water" });
        await db.SaveChangesAsync();
        var store = new EfCharacterRosterStore(db, CreateOptions(), TimeProvider.System);

        Assert.Null(await store.GetAsync(accountB, characterId));
        Assert.False(await store.SoftDeleteAsync(accountB, characterId));
        Assert.Null(await store.SelectAsync(accountB, characterId));
        Assert.False((await db.PlayerCharacters.SingleAsync(x => x.Id == characterId)).IsDeleted);
    }

    [Fact]
    public void AppearanceOutsideZodiacRules_IsRejected()
    {
        var options = CreateOptions();
        var validator = new CharacterDefinitionValidator(options);
        var result = validator.Validate(new CreateCharacterCommand(Guid.NewGuid(), Guid.NewGuid(), "龙角色", "Dragon", "Fire", "Center", new CharacterAppearancePayload(new Dictionary<string, string> { ["Hair"] = "Rat_Hair_01" }, null, null, null), "key-001"), out _);
        Assert.False(result.Success);
        Assert.Equal("APPEARANCE_OPTION_INVALID", result.ErrorCode);
    }

    [Fact]
    public async Task RepeatedIdempotencyKey_ReturnsTheSamePersistedCharacter()
    {
        await using var db = new GameDbContext(new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"character-idempotency-{Guid.NewGuid()}").Options);
        var options = CreateOptions();
        var store = new EfCharacterRosterStore(db, options, TimeProvider.System);
        var command = new CreateCharacterCommand(Guid.NewGuid(), Guid.NewGuid(), "鼠角色", "Rat", "Water", "East", new CharacterAppearancePayload(new Dictionary<string, string> { ["Hair"] = "Rat_Hair_01" }, null, null, null), "create-001");

        var first = await store.CreateAsync(command, "鼠角色");
        var repeated = await store.CreateAsync(command, "鼠角色");

        Assert.NotNull(first);
        Assert.NotNull(repeated);
        Assert.Equal(first!.CharacterId, repeated!.CharacterId);
        Assert.Single(await db.PlayerCharacters.ToListAsync());
        Assert.Single(await db.CharacterAppearances.ToListAsync());
        Assert.Single(await db.CharacterProgresses.ToListAsync());
    }

    private static CharacterCreationOptions CreateOptions() => new()
    {
        NamePattern = "^[\\p{L}\\p{N}_]{2,16}$", InitialLevel = 1, MaxSlotsPerServer = 4, RulesVersion = "test",
        AllowedZodiacTypes = ["Rat", "Dragon"], AllowedElementTypes = ["Water", "Fire"], AllowedFiveCampTypes = ["East", "Center"],
        AppearanceRules = new Dictionary<string, CharacterAppearanceRuleOptions>(StringComparer.OrdinalIgnoreCase)
        {
            ["Rat"] = new() { AllowedOptionIds = ["Rat_Hair_01"] }, ["Dragon"] = new() { AllowedOptionIds = ["Dragon_Hair_01"] }
        }
    };
}
