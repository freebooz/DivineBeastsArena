# Release readiness report

- kind: release-readiness-report
- releaseId: local-launcher-evidence-support-files-gate-20260630
- generatedAtUtc: 2026-06-30T13:39:47.2329731Z
- readyForRelease: False
- developmentContinuationReady: True
- manifest: E:\work\Game\DivineBeastsArena\Artifacts\ProductionEvidence\production-evidence-manifest.json
- evidenceRoot: E:\work\Game\DivineBeastsArena\Artifacts\ProductionEvidence
- gitCommit: a5a08469d8d6ada512972c4edfc9787a680be9a4
- gitIsDirty: True
- presentRequirements: 10 / 14
- blockingRequirements: 4
- externalOnlyReleaseBlockers: True
- releaseBlockerPostureReleaseIdMatchesManifest: True
- releaseBlockerPostureBlockerCountMatchesManifest: True
- externalBlockers: 4
- localAutomationBlockers: 0
- emptyExternalInputBlockers: 0

## Missing or incomplete production evidence

| Key | Status | Files | Description |
| --- | --- | ---: | --- |
| client.release_prerequisites | incomplete | 1 | Client release input prerequisite evidence proving package root, production CDN URLs, and signing inputs are ready before release automation runs. |
| client.package_launcher | incomplete | 16 | Client package, launcher manifest, SHA256 file list, and local install/update repair smoke evidence. |
| client.cdn_launcher_smoke | incomplete | 2 | Launcher CDN smoke evidence proving manifest fetch, package downloads, SHA256 verification, and local install version persistence. |
| client.code_signing | incomplete | 7 | Windows Authenticode code-signing evidence proving public client executables and libraries are trusted signed. |

## All production evidence requirements

| Key | Status | Files | Description |
| --- | --- | ---: | --- |
| security.nuget | present | 1 | NuGet vulnerable package report from security-ci or production-security-audit. |
| security.npm | present | 9 | Production npm audit JSON for Admin, Website, and Launcher. |
| security.trivy | present | 2 | API and Worker container Trivy SARIF or scan report. |
| load.k6 | present | 4 | k6 login, matchmaking, or dedicated server orchestration load-test output. |
| ops.backup_restore | present | 2 | PostgreSQL backup and restore rehearsal evidence. |
| ops.deploy_rollback | present | 2 | Deployment, smoke test, and rollback evidence. |
| unreal.online_validation | present | 5 | UE packaged or editor online validation evidence for backend allocation, Dedicated Server startup, and client joins. |
| unreal.ai_showcase_automation | present | 6 | AI_Showcase UI/VFX automation regression evidence for MCP-generated showcase assets. |
| client.release_prerequisites | incomplete | 1 | Client release input prerequisite evidence proving package root, production CDN URLs, and signing inputs are ready before release automation runs. |
| client.package_launcher | incomplete | 16 | Client package, launcher manifest, SHA256 file list, and local install/update repair smoke evidence. |
| client.cdn_launcher_smoke | incomplete | 2 | Launcher CDN smoke evidence proving manifest fetch, package downloads, SHA256 verification, and local install version persistence. |
| client.code_signing | incomplete | 7 | Windows Authenticode code-signing evidence proving public client executables and libraries are trusted signed. |
| client.launcher_install_update | present | 10 | Launcher install/update smoke evidence proving manifest fetch, repair download, SHA256 verification, and version persistence. |
| client.launcher_ui_visual | present | 5 | Launcher UI visual evidence proving the player-facing launcher screen renders and exposes install/update actions. |

## Evidence file index

- `client/cdn-upload-manifest-client-evidence-local-20260628T080212Z-cdn-payload.json` (11989 bytes, sha256 `90222f86aae258077baaeda3735c41208efa42cf54804fa81be06808d58a967a`)
- `client/client-package-launcher-client-evidence-local-20260628T080212Z-package.json` (1670 bytes, sha256 `b32bff017e22760c6f229749a3622f82e8b9f90ab2be9fda716393dd6f8d1556`)
- `client/client-package-launcher-client-package-launcher-copy-smoke-20260628T072000Z.json` (1318 bytes, sha256 `47e41f4f4583e50675a6c974640324fbc22e922b68334bf8b9ef6523c3f808a3`)
- `client/client-package-launcher-client-package-launcher-local-20260628T071000Z.json` (1309 bytes, sha256 `782399e24e4ed4d79490c78b7e4d2120521e18df42d88df2b2b2e0b0ec040f41`)
- `client/client-package-launcher-client-package-release-readiness-dev-20260628T073000Z.json` (1866 bytes, sha256 `0bc60819a0923853d1ef6efcac6d013974578af640bdc3edf59db8707c889055`)
- `client/client-package-launcher-client-package-shipping-public-example-cdn-20260628T084000Z.json` (1717 bytes, sha256 `7b922b659ff1f7c0e8580b06b093778ac01d1717704b57c831c6dbbe9c72d65e`)
- `client/client-package-launcher-client-package-shipping-release-ready-20260628T083000Z.json` (1631 bytes, sha256 `bd7b9fb13b62ac6d7218870dbfe2f593f40d085225bb4f74f55202f094736213`)
- `client/client-package-launcher-client-release-bundle-final-verify-20260628T104500Z-package.json` (1717 bytes, sha256 `1292fc75a2680a12c59edf24ca37e0f29201d37f1cb79b49cdfd5e10831752bb`)
- `client/client-package-launcher-client-release-bundle-local-20260628T103500Z-package.json` (1696 bytes, sha256 `7d3235facb3a45da0e24fa7cc666c11409a5d0a75cc21cdd73c96c9eeeeec3a5`)
- `client/client-release-evidence-client-evidence-local-20260628T080212Z.json` (2686 bytes, sha256 `5e41485a9162dce91ae7e4aa03b79a17c757bf3694a3942a4b2495c07c7a61a2`)
- `client/client-release-evidence-client-release-bundle-final-verify-20260628T104500Z.json` (1772 bytes, sha256 `3b380a7df5972b7f7109474227941816be12487ec1da3f0e4b150010684d0933`)
- `client/client-release-evidence-client-release-bundle-local-20260628T103500Z.json` (1744 bytes, sha256 `18e63b5355d43ff3584958a31102ad42a954f8767467c3f19dbeec2ca6987261`)
- `client/client-release-package-client-evidence-local-20260628T080212Z.json` (14948 bytes, sha256 `a8b83d69ec6a03f7af46f9c217b7f75ade539ee44d8a6f55e3c38c3b0440cb17`)
- `client/client-release-package-client-release-public-symbols-20260628T082500Z.json` (14972 bytes, sha256 `dd03c724f6b910c2b3218bb7614baf8b0ef489db0f38cd684653d4423aaa6369`)
- `client/client-release-prerequisites-client-evidence-local-20260628T080212Z-example-url.json` (2614 bytes, sha256 `532a256aafbb21748caf908fbe306fc3053aebeaa14473720bae24275d47696c`)
- `client/code-signing-client-code-signing-final-require-signed-expected-fail-20260628T094500Z.json` (16763 bytes, sha256 `01c5889111ed4ec6004b8d875dc032df2dfaf418ce254ba113c866ccb71b0448`)
- `client/code-signing-client-code-signing-final-verify-20260628T094500Z.json` (16741 bytes, sha256 `fc230fc00b12899242ad75ee0f62d85423919dde59d6871a28f70adfda7b3382`)
- `client/code-signing-client-code-signing-require-signed-expected-fail-20260628T093000Z.json` (16757 bytes, sha256 `9f01c219970a1073a847116e1c72175ee73ad97774cae63edefe6ada1a6aa7a8`)
- `client/code-signing-client-code-signing-unsigned-local-20260628T093000Z.json` (16743 bytes, sha256 `a8f369f270bbbc5637580ba1c898e6811e9ad7830cfdba867e655024bde6897b`)
- `client/code-signing-client-evidence-local-20260628T080212Z-signing.json` (16674 bytes, sha256 `7f2cf9ef15980bcbf58f01ad10582e232a620a067cf128d3f4c0197b3f1f6b27`)
- `client/code-signing-client-release-bundle-final-verify-20260628T104500Z-signing.json` (16751 bytes, sha256 `6a05e6bc02c34d4a7e2f8e83da5ab64bbc9b1f281e4aa4c6a1bf67282a02766e`)
- `client/code-signing-client-release-bundle-local-20260628T103500Z-signing.json` (16744 bytes, sha256 `e75e02630ee15b60bdc6f4924fec5881a966eb77936c03b9727d84b050615dd4`)
- `client/launcher-cdn-smoke-client-evidence-local-20260628T080212Z-local-cdn-payload-smoke.json` (11848 bytes, sha256 `ec62eff1104ceb71ea2172ff2dbb3e1a0d47488d91d88f796035fc5777660fe4`)
- `client/launcher-cdn-smoke-launcher-cdn-local-http-smoke-20260628T090000Z.json` (11832 bytes, sha256 `3059b28f46458b6e55961f785045463d1d2537f3e082467171844b9a65d1af8f`)
- `client/launcher-install-update-smoke-client-evidence-local-20260628T080212Z-launcher-install-update.json` (1356 bytes, sha256 `bed2d4d37409cb95f738bc067d4efc7897fe78ddca36f911aad6b4626917d5a0`)
- `client/launcher-install-update-smoke-client-evidence-local-20260628T080212Z-launcher-install-update.log` (302 bytes, sha256 `9e7adff755210c625f12430093182a6053334fb91f42199849044b97e3bc1bc7`)
- `client/launcher-install-update-smoke-client-evidence-local-20260628T080212Z-launcher-install-update.stderr.log` (310 bytes, sha256 `7272e55a147840bccedb954e186c59fc004b900b9b42a4b2721dd4de02687f14`)
- `client/launcher-install-update-smoke-client-release-bundle-final-verify-20260628T104500Z-launcher-install-update.json` (1395 bytes, sha256 `ff295568efcd73ea1c5ba41735c57c07bf1e8f7639e1053a62db414715246883`)
- `client/launcher-install-update-smoke-client-release-bundle-final-verify-20260628T104500Z-launcher-install-update.log` (302 bytes, sha256 `9e7adff755210c625f12430093182a6053334fb91f42199849044b97e3bc1bc7`)
- `client/launcher-install-update-smoke-client-release-bundle-final-verify-20260628T104500Z-launcher-install-update.stderr.log` (310 bytes, sha256 `98d4cb5b25a7b87eb5ab84b27d2e93eafc378f73b5b9214539ab24f1f5d088a0`)
- `client/launcher-install-update-smoke-client-release-bundle-local-20260628T103500Z-launcher-install-update.json` (1374 bytes, sha256 `16e3e081c0015b9d188c785e9cf7bf62d4a5ae2c60527a89fa41dee0ec1ee887`)
- `client/launcher-install-update-smoke-client-release-bundle-local-20260628T103500Z-launcher-install-update.log` (302 bytes, sha256 `9e7adff755210c625f12430093182a6053334fb91f42199849044b97e3bc1bc7`)
- `client/launcher-install-update-smoke-client-release-bundle-local-20260628T103500Z-launcher-install-update.stderr.log` (310 bytes, sha256 `7272e55a147840bccedb954e186c59fc004b900b9b42a4b2721dd4de02687f14`)
- `client/launcher-install-update-smoke-launcher-install-update-final-verify-20260628T102500Z.json` (1329 bytes, sha256 `78e5e276465476d10e6206df7379eaf27f373cd5de082edc2871564b24dfd8d7`)
- `client/launcher-install-update-smoke-launcher-install-update-final-verify-20260628T102500Z.log` (302 bytes, sha256 `9e7adff755210c625f12430093182a6053334fb91f42199849044b97e3bc1bc7`)
- `client/launcher-install-update-smoke-launcher-install-update-final-verify-20260628T102500Z.stderr.log` (310 bytes, sha256 `43775504eef516a14b228f612c2e38586d518e1b7cbd11c2369752d2bc712716`)
- `client/launcher-install-update-smoke-launcher-install-update-local-20260628T101500Z.json` (1307 bytes, sha256 `e5031a2e10e6a3d3da951e79b362e5c1309fe2135add8224e1105afbeac5a19f`)
- `client/launcher-install-update-smoke-launcher-install-update-local-20260628T101500Z.log` (302 bytes, sha256 `041a67083c9aa6445f2811c4d065f1e2e03713d14333f3a66aedbf51bf909802`)
- `client/launcher-install-update-smoke-launcher-install-update-local-20260628T101500Z.stderr.log` (310 bytes, sha256 `5ecee1cd4bf6992437dac01a3891f5d60c9a1d037b94c19492af53895ed3c26c`)
- `client/launcher-manifest-client-evidence-local-20260628T080212Z-package.json` (9294 bytes, sha256 `19d32088574e4c291d68f099fda69fe02c238ce77167839d4f4c887c3fdfcecf`)
- `client/launcher-manifest-client-package-launcher-copy-smoke-20260628T072000Z.json` (14700 bytes, sha256 `4a639ea573d42f7a10b7ce1c1fe598854fc46f3cdaea068661dff3963adc249d`)
- `client/launcher-manifest-client-package-launcher-local-20260628T071000Z.json` (14700 bytes, sha256 `4a639ea573d42f7a10b7ce1c1fe598854fc46f3cdaea068661dff3963adc249d`)
- `client/launcher-manifest-client-package-release-readiness-dev-20260628T073000Z.json` (14700 bytes, sha256 `4a639ea573d42f7a10b7ce1c1fe598854fc46f3cdaea068661dff3963adc249d`)
- `client/launcher-manifest-client-package-shipping-public-example-cdn-20260628T084000Z.json` (9294 bytes, sha256 `19d32088574e4c291d68f099fda69fe02c238ce77167839d4f4c887c3fdfcecf`)
- `client/launcher-manifest-client-package-shipping-release-ready-20260628T083000Z.json` (9294 bytes, sha256 `19d32088574e4c291d68f099fda69fe02c238ce77167839d4f4c887c3fdfcecf`)
- `client/launcher-manifest-client-release-bundle-final-verify-20260628T104500Z-package.json` (9550 bytes, sha256 `1e7207a80c9a6396b59ae4d34e85d566ece9590e2cdc40ab37fc71cff9f7b63b`)
- `client/launcher-manifest-client-release-bundle-local-20260628T103500Z-package.json` (9550 bytes, sha256 `1e7207a80c9a6396b59ae4d34e85d566ece9590e2cdc40ab37fc71cff9f7b63b`)
- `client/launcher-ui-visual-evidence-client-evidence-local-20260628T080212Z-launcher-ui-visual.browser.log` (0 bytes, sha256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`)
- `client/launcher-ui-visual-evidence-client-evidence-local-20260628T080212Z-launcher-ui-visual.browser.stderr.log` (0 bytes, sha256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`)
- `client/launcher-ui-visual-evidence-client-evidence-local-20260628T080212Z-launcher-ui-visual.build.log` (436 bytes, sha256 `b46302ddaa0dbd166d943452d28a215ff74fd651134dde0e2ef66407785c490a`)
- `client/launcher-ui-visual-evidence-client-evidence-local-20260628T080212Z-launcher-ui-visual.build.stderr.log` (0 bytes, sha256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`)
- `client/launcher-ui-visual-evidence-client-evidence-local-20260628T080212Z-launcher-ui-visual.dom.html` (2067 bytes, sha256 `c8d8fb33b2b740e37fdfd3802c20d6efc21c0c98cf190177c16ce93eeb2ca988`)
- `client/launcher-ui-visual-evidence-client-evidence-local-20260628T080212Z-launcher-ui-visual.json` (2789 bytes, sha256 `f17e66244b239dfa47ada50e67ac90d58b77e1cdddb2292e093b589dfd896ada`)
- `client/launcher-ui-visual-evidence-client-evidence-local-20260628T080212Z-launcher-ui-visual.png` (221746 bytes, sha256 `702ec90a81990d1815f7850664f2ef23a11e30a878bd073934460dd5db7a11fd`)
- `client/launcher-ui-visual-evidence-client-evidence-local-20260628T080212Z-launcher-ui-visual.preview.log` (172 bytes, sha256 `48668653aef7ccbbd0dd0217760d0803d7a5fa7cddf0c6759eb47daa6f023357`)
- `client/launcher-ui-visual-evidence-client-evidence-local-20260628T080212Z-launcher-ui-visual.preview.stderr.log` (0 bytes, sha256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`)
- `load/dedicated-server-orchestration-skipped-local-runner-docker-20260628T024513Z.txt` (87 bytes, sha256 `0d33b13c3dc8ddc028544398cd1ffc6e07f7d1e29ab73cf9623eb440ec624e56`)
- `load/dedicated-server-orchestration-skipped-local-runner-docker-20260628T024614Z.txt` (87 bytes, sha256 `0d33b13c3dc8ddc028544398cd1ffc6e07f7d1e29ab73cf9623eb440ec624e56`)
- `load/k6-login-20260628T024142Z.json` (3967 bytes, sha256 `d919461642bd4100aa6c4020ecc8d76291f8c1ccba3517b2402f82ed6dc254b8`)
- `load/k6-login-20260628T024142Z.log` (16680 bytes, sha256 `ce615c6acb658080d98fd68e0b2f293a0938b2e1b63fe5b9bc9c8bc7395f4d12`)
- `load/k6-login-20260628T024142Z.meta.txt` (229 bytes, sha256 `c153b2c80cea45bc3843e6af98726f09af26439a1bc9eafa0af42b014e527a98`)
- `load/k6-login-local-k6-20260628T022038Z.json` (3956 bytes, sha256 `6ba8cfdd3686c3bff4efb335d3878ca11a439223d8c71d11e6e4c58e488cae10`)
- `load/k6-login-local-k6-20260628T022038Z.log` (33910 bytes, sha256 `b3a2a99f35d7edc97f0c4215aeab113970aae6a3c5f2f4e8a4dc0aec7f4b0c46`)
- `load/k6-login-local-k6-20260628T022038Z.meta.txt` (37 bytes, sha256 `1a1bfb6b1be9e272019b98bdc21d9626fd9f70878ccfe2e4cfd3edfc7479a777`)
- `load/k6-login-local-k6-login-20260628T022527Z.json` (3843 bytes, sha256 `ee97246d62757bb6f2780b8975294baf21d5086c1cde672e3e77d70fbc3e27f9`)
- `load/k6-login-local-k6-login-20260628T022527Z.log` (8462 bytes, sha256 `77194b65afa57e554e2590f3c45af5bbd7e3931eac34ad2c5665179272d0a111`)
- `load/k6-login-local-k6-login-20260628T022527Z.meta.txt` (230 bytes, sha256 `fd74c3f8cc3043c1fc3b3a9c058de5bd5df511fc433f702535133a601d7114fb`)
- `load/k6-login-local-runner-docker-20260628T024513Z.json` (3856 bytes, sha256 `f0701307640147f0fccb2ada69e939c0f048e6b1dfe099b7ad412626d7dcbb88`)
- `load/k6-login-local-runner-docker-20260628T024513Z.log` (2759 bytes, sha256 `813946256da25c7677989c11a05b8ec1f7ecdac2ced636ba7de27a09a1114058`)
- `load/k6-login-local-runner-docker-20260628T024513Z.meta.txt` (254 bytes, sha256 `38651fd13c1deeba61b7467c7b841d3112dca68fdf7b576068246fb11423d6fe`)
- `load/k6-login-local-runner-docker-20260628T024614Z.json` (3817 bytes, sha256 `036edd1139f3d26bcd9cd0e73f3fa89f1462a51873c158f52610648a2b6a7a0d`)
- `load/k6-login-local-runner-docker-20260628T024614Z.log` (2740 bytes, sha256 `5b04126ed00a08714fd49f5a9930650b951cf441da8ca2cb6bb3f13a7dde6d7c`)
- `load/k6-login-local-runner-docker-20260628T024614Z.meta.txt` (254 bytes, sha256 `c02400f3af588eec6c5b3cf7df5b51a79e1c53d846327926816333b2965547e2`)
- `load/k6-matchmaking-local-k6-matchmaking-20260628T022603Z.json` (4505 bytes, sha256 `64dd3ac75a60afb9294310af741c378abc5faaadecf83acbe13e94180974cd15`)
- `load/k6-matchmaking-local-k6-matchmaking-20260628T022603Z.log` (8370 bytes, sha256 `d63aa2b368e036567834bbb0878825b291155d351d236ef1497d0c44ceefc129`)
- `load/k6-matchmaking-local-k6-matchmaking-20260628T022603Z.meta.txt` (178 bytes, sha256 `6c0d9d3d9de6fee439a2a98da6210157230e6c95fb865a33f691d2384ff2c96c`)
- `load/k6-matchmaking-local-k6-matchmaking-20260628T022812Z.json` (4397 bytes, sha256 `a7e529f9b0234765ce719d684dbc3cf62425a91d799dc174e9d720e8c9b2eb0b`)
- `load/k6-matchmaking-local-k6-matchmaking-20260628T022812Z.log` (8078 bytes, sha256 `d5c8d4223cf9b7abc1167f858583276f1a5f21fed61cdfdace9eccd0e7670c9c`)
- `load/k6-matchmaking-local-k6-matchmaking-20260628T022812Z.meta.txt` (178 bytes, sha256 `6d2e179d03472ae9b9f41ac65b79b06385eff4e80468595b880e435674e70aa3`)
- `load/k6-matchmaking-local-runner-docker-20260628T024513Z.json` (4404 bytes, sha256 `5eec0fab16783f2db4542f9f14ac5774ed0fd3c9b3645207504b4212ad9e0282`)
- `load/k6-matchmaking-local-runner-docker-20260628T024513Z.log` (2244 bytes, sha256 `634111e2bd390733f2c4272082fd917b9a681834087ec207bc6bf0395c0f9dcb`)
- `load/k6-matchmaking-local-runner-docker-20260628T024513Z.meta.txt` (254 bytes, sha256 `38651fd13c1deeba61b7467c7b841d3112dca68fdf7b576068246fb11423d6fe`)
- `load/k6-matchmaking-local-runner-docker-20260628T024614Z.json` (4379 bytes, sha256 `6894e4659b2822bce937e7187eb5b69d210c9ae2621139bd1b5b752b847e8a6b`)
- `load/k6-matchmaking-local-runner-docker-20260628T024614Z.log` (2243 bytes, sha256 `af4e21c9af043442563b072ae832ec0588fd945145f36395102a816492d167e4`)
- `load/k6-matchmaking-local-runner-docker-20260628T024614Z.meta.txt` (254 bytes, sha256 `c02400f3af588eec6c5b3cf7df5b51a79e1c53d846327926816333b2965547e2`)
- `ops/backup-restore-rehearsal-20260628T015304Z.json` (484 bytes, sha256 `c5d605cff645a0e4827cc479ce1951b2295f974c3b99aa02614b2d53b62d6d09`)
- `ops/backup-restore-rehearsal-20260628T015304Z.log` (562 bytes, sha256 `78238c82a6f35854a21b729ac09e68d542339972751e610379d9b0368e2f3cb7`)
- `ops/backup-restore-rehearsal-20260628T015502Z.json` (530 bytes, sha256 `72e20c121e25a4194b1869ecc23a6039c4857d875a007bc322e32be476ce7f9a`)
- `ops/backup-restore-rehearsal-20260628T015502Z.log` (562 bytes, sha256 `baaf30915112805ad6ffe3d12ad756d9fdf054e47109f2d1addf079163922d5b`)
- `ops/production-smoke-backend-local-smoke-20260628T021000Z.json` (1560 bytes, sha256 `ca34d22ce99e86b242854546466ded6d0e4b038b6cd9dcd93b931fa4e9415952`)
- `ops/production-smoke-backend-local-smoke-20260628T021000Z.log` (1724 bytes, sha256 `9d0210647e417a379e6247142912a62ee09415c46e775591a055a15eaeeee2a5`)
- `release-input-values-template-validation.json` (4201 bytes, sha256 `b3c1d4c7a688aa4c1e20685aab680030819713b8adbfbaba0f49ccc65b2c0995`)
- `security/npm-audit-admin-local-20260628T075336Z.json` (390 bytes, sha256 `79bcc53871a9f6ea2124785562cd58cce508900005e0370e610ea063920fd35c`)
- `security/npm-audit-admin-local-20260630T095827Z.json` (390 bytes, sha256 `79bcc53871a9f6ea2124785562cd58cce508900005e0370e610ea063920fd35c`)
- `security/npm-audit-admin-local-security-20260628T020000Z.json` (390 bytes, sha256 `79bcc53871a9f6ea2124785562cd58cce508900005e0370e610ea063920fd35c`)
- `security/npm-audit-launcher-local-20260628T075336Z.json` (386 bytes, sha256 `bcb84495521f2b945c1e3d577b3ad4c38c127ba5680c05fe58d5d980a058ac4e`)
- `security/npm-audit-launcher-local-20260630T095827Z.json` (386 bytes, sha256 `bcb84495521f2b945c1e3d577b3ad4c38c127ba5680c05fe58d5d980a058ac4e`)
- `security/npm-audit-launcher-local-security-20260628T020000Z.json` (386 bytes, sha256 `bcb84495521f2b945c1e3d577b3ad4c38c127ba5680c05fe58d5d980a058ac4e`)
- `security/npm-audit-website-local-20260628T075336Z.json` (389 bytes, sha256 `1303aed0dc4f924e2797849b9ca13d4a4ffd0c715879358ba523874ad03b2ac6`)
- `security/npm-audit-website-local-20260630T095827Z.json` (389 bytes, sha256 `1303aed0dc4f924e2797849b9ca13d4a4ffd0c715879358ba523874ad03b2ac6`)
- `security/npm-audit-website-local-security-20260628T020000Z.json` (389 bytes, sha256 `1303aed0dc4f924e2797849b9ca13d4a4ffd0c715879358ba523874ad03b2ac6`)
- `security/trivy-api-local-security-20260628T020000Z.sarif` (867 bytes, sha256 `54950812c544ce5fbef91ee3e8d0f7e5199637e6c866d77457f81cf81d59e182`)
- `security/trivy-worker-local-security-20260628T020000Z.sarif` (876 bytes, sha256 `67238d2fe838bf62296be36ff0fde9e356474e6f215908ed6e9ccd021f3fd3c1`)
- `security/vulnerability-report.txt` (983 bytes, sha256 `6d6de3a61fbfcdf127a789b2d7260529545bfe816d8ce64d80180a4d3ccf54da`)
- `unreal/ai-showcase-automation-ai-showcase-editorcmd-20260629.json` (1659 bytes, sha256 `7e16fdb15042bb9e6d9c274905140f06587a6b7456da56611123a63fff0b988a`)
- `unreal/ai-showcase-automation-ai-showcase-editorcmd-window-20260629.json` (1678 bytes, sha256 `9df64cf7290d1ea7f0cf42f4fa89d4b718428c069942b4c18d104ff81db496d9`)
- `unreal/ai-showcase-automation-ai-showcase-loggate-20260629.json` (1496 bytes, sha256 `5de459d030465fb51273f7cdebb86ee980e5a3642e28c0227f0acbc2ff4bdaac`)
- `unreal/ai-showcase-automation-ai-showcase-semantic-20260629.json` (1382 bytes, sha256 `01c191f8466f81a5998b200d526f8b5ebc72c53734524c67314fdd6fbe091b6c`)
- `unreal/ai-showcase-automation-ai-showcase-widget-tree-20260630.json` (1665 bytes, sha256 `ff5fccb8fc4e852e69d1c5658f5baed2f5f344f03e1f0d86addbed3dd5f99a85`)
- `unreal/ai-showcase-automation-ai-showcase-widget-tree-20260630b.json` (1666 bytes, sha256 `df0b90c38e81a03bcb4402467fd0b6437ec91465efb7f649b0bb4a1ace387a74`)
- `unreal/runner-diagnostic-packaged-ue-online-external-mode-20260628T062800Z.json` (4153 bytes, sha256 `b2dad7a96ecf252a2b538d8e3bb2bda888b3945f3e0fd93f3d14a1491dcc45b2`)
- `unreal/runner-diagnostic-packaged-ue-online-url-decode-20260628T064000Z.json` (4153 bytes, sha256 `85c44f92da37a2d685f5793239c62ea049f6d3ab800621c21417b6786f5c91cb`)
- `unreal/runner-diagnostic-packaged-ue-online-wrapper-full-20260628T061500Z.json` (4153 bytes, sha256 `715cbe0598baf71836fbcfaae572c1fc169f9f316bfb238bb21396454e337f46`)
- `unreal/ue-online-validation-packaged-ue-online-build-summary-20260628T040000Z.json` (2412 bytes, sha256 `63d51b5bde3886997648d928dcc8cf2eee7109d053690d120f7478db9c6737dd`)
- `unreal/ue-online-validation-packaged-ue-online-build-summary-20260628T041000Z.json` (2521 bytes, sha256 `a64fa24fa0d4d3ff09fc1e3fd0e0377db2f1b57893c52459657986b7ed567579`)
- `unreal/ue-online-validation-packaged-ue-online-external-mode-20260628T062800Z.json` (2521 bytes, sha256 `568db20b46dc773cb10839905ac8ac198690eaa2490d9e920c067ea59b78f43f`)
- `unreal/ue-online-validation-packaged-ue-online-url-decode-20260628T064000Z.json` (3213 bytes, sha256 `40f8dbb85fd9e33e0e34ec03d03ae696d5f63195642ba92a5ca45b493ab9b379`)
- `unreal/ue-online-validation-packaged-ue-online-wrapper-full-20260628T061500Z.json` (2517 bytes, sha256 `f567a38d2673a27621ee8ebabf4b79d392fd068c7becb0a77fd23b9241f0b860`)
