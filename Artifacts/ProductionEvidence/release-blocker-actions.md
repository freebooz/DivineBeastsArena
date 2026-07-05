# Release blocker actions

- kind: release-blocker-actions
- releaseId: local-launcher-evidence-support-files-gate-20260630
- generatedAtUtc: 2026-06-30T10:41:22.5705292Z
- readyForRelease: False
- blockerCount: 4

| Key | Status | Owner | Script | Next command |
| --- | --- | --- | --- | --- |
| client.release_prerequisites | incomplete | release-engineering | scripts\diagnose-client-release-prerequisites.ps1 | `.\scripts\diagnose-client-release-prerequisites.ps1 -PackageRoot <release-package-root> -DownloadUrl <real-https-cdn-download-url> -ManifestUrl <real-https-cdn-manifest-url> -RequireManifestUrl -RequireSigningIdentity -CertificateThumbprint <trusted-authenticode-signing-identity> -SignToolPath <signtool-path> -RequireSignTool -FailOnBlockingIssues` |
| client.package_launcher | incomplete | client-release | scripts\run-client-release-evidence.ps1 | `.\scripts\run-client-release-evidence.ps1 -PackageRoot <public-shipping-package-root> -DownloadUrl <real-https-cdn-download-url> -ManifestUrl <real-https-cdn-manifest-url> -BuildConfiguration Shipping` |
| client.cdn_launcher_smoke | incomplete | release-ops | scripts\run-launcher-cdn-smoke.ps1 | `.\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl <real-https-cdn-manifest-url> -InstallRoot <local-smoke-install-root>` |
| client.code_signing | incomplete | release-security | scripts\sign-client-release-package.ps1 | `.\scripts\sign-client-release-package.ps1 -PackageRoot <public-package-root-containing-signable-binaries> -CertificateThumbprint <trusted-authenticode-signing-identity> -TimestampUrl <timestamp-url> -RequireSigned` |

## Unblock criteria

- `client.release_prerequisites`: Generate client-release-prerequisites evidence with readyForReleaseInputs=true using a real HTTPS CDN URL, non-example manifest URL, package root, signing identity, and signtool.
- `client.package_launcher`: Generate client-package-launcher evidence with releaseReady=true, public Shipping package, no debug symbols, and non-example HTTPS CDN URLs.
- `client.cdn_launcher_smoke`: Generate launcher CDN smoke evidence with cdnReady=true after downloading every manifest file and validating SHA256, size, and version persistence from real HTTPS CDN.
- `client.code_signing`: Generate code-signing evidence with signingReady=true after every public .exe, .dll, .msi, .msix, and .appx has a trusted Authenticode signature.

## Automation status

### client.release_prerequisites
- automationBlocked: True
- blockedBy: real HTTPS CDN download URL
- blockedBy: real HTTPS CDN manifest URL
- blockedBy: release package root
- blockedBy: trusted Authenticode signing identity
- blockedBy: signtool path

### client.package_launcher
- automationBlocked: True
- blockedBy: public Shipping package root
- blockedBy: real HTTPS CDN download URL
- blockedBy: real HTTPS CDN manifest URL

### client.cdn_launcher_smoke
- automationBlocked: True
- blockedBy: real HTTPS CDN manifest URL
- blockedBy: local smoke install root

### client.code_signing
- automationBlocked: True
- blockedBy: trusted Authenticode signing identity
- blockedBy: timestamp URL
- blockedBy: public package root containing signable binaries

## Input resolution hints

### client.release_prerequisites
- real HTTPS CDN download URL: parameters: -DownloadUrl; env: none
- real HTTPS CDN manifest URL: parameters: -ManifestUrl; env: none
- release package root: parameters: -PackageRoot; env: none
- trusted Authenticode signing identity: parameters: -CertificateThumbprint, -CertificateSubject, -PfxPath; env: none
- signtool path: parameters: -SignToolPath; env: WindowsSdkDir

### client.package_launcher
- public Shipping package root: parameters: -PackageRoot, -StagedPackageRoot; env: none
- real HTTPS CDN download URL: parameters: -DownloadUrl; env: none
- real HTTPS CDN manifest URL: parameters: -ManifestUrl; env: none

### client.cdn_launcher_smoke
- real HTTPS CDN manifest URL: parameters: -ManifestUrl; env: none
- local smoke install root: parameters: -InstallRoot; env: none

### client.code_signing
- trusted Authenticode signing identity: parameters: -CertificateThumbprint, -CertificateSubject, -PfxPath; env: none
- timestamp URL: parameters: -TimestampUrl; env: none
- public package root containing signable binaries: parameters: -PackageRoot; env: none

## Missing external inputs

### client.release_prerequisites
- latest evidence: client/client-release-prerequisites-client-evidence-local-20260628T080212Z-example-url.json
- real HTTPS CDN download URL
- real HTTPS CDN manifest URL
- release package root
- trusted Authenticode signing identity
- signtool path

### client.package_launcher
- latest evidence: client/client-package-launcher-client-evidence-local-20260628T080212Z-package.json
- public Shipping package root
- real HTTPS CDN download URL
- real HTTPS CDN manifest URL

### client.cdn_launcher_smoke
- latest evidence: client/launcher-cdn-smoke-client-evidence-local-20260628T080212Z-local-cdn-payload-smoke.json
- real HTTPS CDN manifest URL
- local smoke install root

### client.code_signing
- latest evidence: client/code-signing-client-evidence-local-20260628T080212Z-signing.json
- trusted Authenticode signing identity
- timestamp URL
- public package root containing signable binaries

## Observed reasons

### client.release_prerequisites
- client/client-release-prerequisites-client-evidence-local-20260628T080212Z-example-url.json: readyForReleaseInputs=False
- client/client-release-prerequisites-client-evidence-local-20260628T080212Z-example-url.json: download_url_example - DownloadUrl still points at cdn.example.com: https://cdn.example.com/releases/0.1.0.0/
- client/client-release-prerequisites-client-evidence-local-20260628T080212Z-example-url.json: manifest_url_example - ManifestUrl still points at cdn.example.com: https://cdn.example.com/releases/0.1.0.0/launcher-manifest.json

### client.package_launcher
_showing 5 of 14 observed reasons_

- client/client-package-launcher-client-evidence-local-20260628T080212Z-package.json: releaseReady=False
- client/client-package-launcher-client-evidence-local-20260628T080212Z-package.json: DownloadUrl is an example CDN URL, not a release CDN URL.
- client/client-package-launcher-client-release-bundle-final-verify-20260628T104500Z-package.json: releaseReady=False
- client/client-package-launcher-client-release-bundle-final-verify-20260628T104500Z-package.json: DownloadUrl is an example CDN URL, not a release CDN URL.
- client/client-package-launcher-client-release-bundle-local-20260628T103500Z-package.json: releaseReady=False

### client.cdn_launcher_smoke
_showing 5 of 12 observed reasons_

- client/launcher-cdn-smoke-client-evidence-local-20260628T080212Z-local-cdn-payload-smoke.json: cdnReady=False
- client/launcher-cdn-smoke-client-evidence-local-20260628T080212Z-local-cdn-payload-smoke.json: manifestUrlIsHttps=False
- client/launcher-cdn-smoke-client-evidence-local-20260628T080212Z-local-cdn-payload-smoke.json: downloadUrlIsHttps=False
- client/launcher-cdn-smoke-client-evidence-local-20260628T080212Z-local-cdn-payload-smoke.json: manifestUrlIsExample=False
- client/launcher-cdn-smoke-client-evidence-local-20260628T080212Z-local-cdn-payload-smoke.json: downloadUrlIsExample=False

### client.code_signing
_showing 5 of 56 observed reasons_

- client/code-signing-client-evidence-local-20260628T080212Z-signing.json: signingReady=False
- client/code-signing-client-evidence-local-20260628T080212Z-signing.json: signableFileCount=15
- client/code-signing-client-evidence-local-20260628T080212Z-signing.json: signedFileCount=8
- client/code-signing-client-evidence-local-20260628T080212Z-signing.json: trustedSignedFileCount=8
- client/code-signing-client-evidence-local-20260628T080212Z-signing.json: unsignedFileCount=7

