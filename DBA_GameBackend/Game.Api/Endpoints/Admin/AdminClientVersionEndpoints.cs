using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.Admin;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.Admin;

public static partial class AdminEndpoints
{
    private static async Task<IResult> ListClientVersions(int page, int pageSize, GameDbContext db)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query = db.ClientVersions.AsNoTracking()
            .OrderByDescending(x => x.CreatedAt);
        var total = await query.CountAsync();
        var items = await query
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new AdminClientVersionItem(
                x.Id,
                x.Version,
                x.Channel,
                x.Platform,
                x.DownloadUrl,
                x.Checksum,
                x.SizeBytes,
                x.IsMandatory,
                x.IsActive,
                x.MinOsVersion,
                x.ReleaseNotes,
                x.CreatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<AdminClientVersionListResponse>.Ok(
            new AdminClientVersionListResponse(items, total, page, pageSize)));
    }

    private static async Task<IResult> UpsertClientVersion(
        UpsertClientVersionRequest request,
        GameDbContext db,
        HttpContext ctx,
        CancellationToken cancellationToken)
    {
        if (string.IsNullOrWhiteSpace(request.Version) ||
            string.IsNullOrWhiteSpace(request.Channel) ||
            string.IsNullOrWhiteSpace(request.Platform) ||
            string.IsNullOrWhiteSpace(request.DownloadUrl) ||
            string.IsNullOrWhiteSpace(request.Checksum) ||
            string.IsNullOrWhiteSpace(request.Reason))
        {
            return Results.BadRequest(ApiResponse.Fail("Version, channel, platform, downloadUrl, checksum and reason are required."));
        }

        if (!Uri.TryCreate(request.DownloadUrl, UriKind.Absolute, out var downloadUri) ||
            downloadUri.Scheme is not ("https" or "http"))
        {
            return Results.BadRequest(ApiResponse.Fail("downloadUrl must be an absolute HTTP/HTTPS URL."));
        }

        if (request.SizeBytes <= 0)
        {
            return Results.BadRequest(ApiResponse.Fail("sizeBytes must be greater than 0."));
        }

        var adminId = GetAdminId(ctx);
        var channel = request.Channel.Trim();
        var platform = request.Platform.Trim();
        var version = request.Version.Trim();
        await using var tx = await db.Database.BeginTransactionAsync(cancellationToken);

        if (request.IsActive)
        {
            await db.ClientVersions
                .Where(x => x.Channel == channel && x.Platform == platform && x.IsActive)
                .ExecuteUpdateAsync(x => x.SetProperty(v => v.IsActive, false), cancellationToken);
        }

        var existing = await db.ClientVersions
            .FirstOrDefaultAsync(x => x.Channel == channel && x.Platform == platform && x.Version == version, cancellationToken);

        if (existing is null)
        {
            existing = new ClientVersion
            {
                Id = Guid.NewGuid(),
                CreatedAt = DateTimeOffset.UtcNow
            };
            db.ClientVersions.Add(existing);
        }

        existing.Version = version;
        existing.Channel = channel;
        existing.Platform = platform;
        existing.DownloadUrl = request.DownloadUrl.Trim();
        existing.Checksum = request.Checksum.Trim();
        existing.SizeBytes = request.SizeBytes;
        existing.IsMandatory = request.IsMandatory;
        existing.IsActive = request.IsActive;
        existing.MinOsVersion = string.IsNullOrWhiteSpace(request.MinOsVersion) ? null : request.MinOsVersion.Trim();
        existing.ReleaseNotes = string.IsNullOrWhiteSpace(request.ReleaseNotes) ? null : request.ReleaseNotes.Trim();

        await AddAuditLogAsync(
            db,
            adminId,
            "ADMIN_CLIENT_VERSION_UPSERT",
            "ClientVersion",
            existing.Id.ToString(),
            request.Reason.Trim(),
            ctx);

        await db.SaveChangesAsync(cancellationToken);
        await tx.CommitAsync(cancellationToken);

        return Results.Ok(ApiResponse<AdminClientVersionItem>.Ok(new AdminClientVersionItem(
            existing.Id,
            existing.Version,
            existing.Channel,
            existing.Platform,
            existing.DownloadUrl,
            existing.Checksum,
            existing.SizeBytes,
            existing.IsMandatory,
            existing.IsActive,
            existing.MinOsVersion,
            existing.ReleaseNotes,
            existing.CreatedAt)));
    }
}
