# release-input-values

- kind: release-input-values
- releaseId: local-production-evidence-20260630n
- isTemplate: true
- inputCount: 9

| Input | Value | Placeholder | Primary parameter | Parameters | Env | Blocked by | Compatible inputs |
| --- | --- | --- | --- | --- | --- | --- | --- |
| real HTTPS CDN download URL |  | `<real-https-cdn-download-url>` | `-DownloadUrl` | -DownloadUrl |  | client.release_prerequisites, client.package_launcher |  |
| real HTTPS CDN manifest URL |  | `<real-https-cdn-manifest-url>` | `-ManifestUrl` | -ManifestUrl |  | client.release_prerequisites, client.package_launcher, client.cdn_launcher_smoke |  |
| release package root |  | `<release-package-root>` | `-PackageRoot` | -PackageRoot |  | client.release_prerequisites | public Shipping package root, public package root containing signable binaries |
| trusted Authenticode signing identity |  | `<trusted-authenticode-signing-identity>` | `-CertificateThumbprint` | -CertificateThumbprint, -CertificateSubject, -PfxPath |  | client.release_prerequisites, client.code_signing |  |
| signtool path |  | `<signtool-path>` | `-SignToolPath` | -SignToolPath | WindowsSdkDir | client.release_prerequisites |  |
| public Shipping package root |  | `<public-shipping-package-root>` | `-PackageRoot` | -PackageRoot, -StagedPackageRoot |  | client.package_launcher | release package root, public package root containing signable binaries |
| local smoke install root |  | `<local-smoke-install-root>` | `-InstallRoot` | -InstallRoot |  | client.cdn_launcher_smoke |  |
| timestamp URL |  | `<timestamp-url>` | `-TimestampUrl` | -TimestampUrl |  | client.code_signing |  |
| public package root containing signable binaries |  | `<public-package-root-containing-signable-binaries>` | `-PackageRoot` | -PackageRoot |  | client.code_signing | release package root, public Shipping package root |
