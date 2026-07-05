# release-command-plan

- kind: release-command-plan
- releaseId: local-production-evidence-20260630n
- isComplete: False
- commandCount: 4
- missingInputCount: 9
- unresolvedPlaceholderCount: 9
- valuesValid: False

## Invalid values

- blankValueCount: 9
- placeholderValueCount: 0
- exampleUrlCount: 0
- insecureUrlCount: 0
- inputCountMatches: True

## Missing inputs

- release package root: `<release-package-root>`
- real HTTPS CDN download URL: `<real-https-cdn-download-url>`
- real HTTPS CDN manifest URL: `<real-https-cdn-manifest-url>`
- trusted Authenticode signing identity: `<trusted-authenticode-signing-identity>`
- signtool path: `<signtool-path>`
- public Shipping package root: `<public-shipping-package-root>`
- local smoke install root: `<local-smoke-install-root>`
- public package root containing signable binaries: `<public-package-root-containing-signable-binaries>`
- timestamp URL: `<timestamp-url>`

## Commands

### diagnose-client-release-prerequisites

- script: scripts\diagnose-client-release-prerequisites.ps1
- unresolvedPlaceholderCount: 5

```powershell
.\scripts\diagnose-client-release-prerequisites.ps1 -PackageRoot <release-package-root> -DownloadUrl <real-https-cdn-download-url> -ManifestUrl <real-https-cdn-manifest-url> -RequireManifestUrl -RequireSigningIdentity -CertificateThumbprint <trusted-authenticode-signing-identity> -SignToolPath <signtool-path> -RequireSignTool -FailOnBlockingIssues
```

### run-client-release-evidence

- script: scripts\run-client-release-evidence.ps1
- unresolvedPlaceholderCount: 3

```powershell
.\scripts\run-client-release-evidence.ps1 -PackageRoot <public-shipping-package-root> -DownloadUrl <real-https-cdn-download-url> -ManifestUrl <real-https-cdn-manifest-url> -BuildConfiguration Shipping
```

### run-launcher-cdn-smoke

- script: scripts\run-launcher-cdn-smoke.ps1
- unresolvedPlaceholderCount: 2

```powershell
.\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl <real-https-cdn-manifest-url> -InstallRoot <local-smoke-install-root>
```

### sign-client-release-package

- script: scripts\sign-client-release-package.ps1
- unresolvedPlaceholderCount: 3

```powershell
.\scripts\sign-client-release-package.ps1 -PackageRoot <public-package-root-containing-signable-binaries> -CertificateThumbprint <trusted-authenticode-signing-identity> -TimestampUrl <timestamp-url> -RequireSigned
```

