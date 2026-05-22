/*
中文阅读说明：
- 所属应用：DBA_GameLauncher 游戏启动器。
- 文件职责：前端应用源码，负责界面状态、用户操作和与本地/远程服务的桥接。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

import { useEffect, useMemo, useState } from "react";
import { invoke } from "@tauri-apps/api/core";
import "./App.css";

type VersionInfo = {
  version: string;
  path: string;
};

type ManifestFile = {
  name: string;
  sha256: string;
  size: number;
};

type UpdateManifest = {
  version: string;
  files: ManifestFile[];
};

type LauncherStatus = {
  status: string;
  latestVersion: string;
  downloadUrl: string;
  mandatory: boolean;
  activeGameServers: number;
  generatedAt: string;
};

const defaultInstallPath = "D:\\DivineBeastsArenaPlatform\\DBA_GameClient";
const defaultExecutablePath =
  "D:\\DivineBeastsArenaPlatform\\DBA_GameClient\\Binaries\\Win64\\DivineBeastsArena.exe";
const defaultManifestUrl = "http://localhost:8080/launcher/manifest.json";
const defaultBackendArg = "-BackendBaseUrl=http://localhost:8080";

function getBackendBaseUrl(manifestUrl: string) {
  try {
    const url = new URL(manifestUrl);
    return `${url.protocol}//${url.host}`;
  } catch {
    return "http://localhost:8080";
  }
}

function App() {
  const [installPath, setInstallPath] = useState(defaultInstallPath);
  const [executablePath, setExecutablePath] = useState(defaultExecutablePath);
  const [manifestUrl, setManifestUrl] = useState(defaultManifestUrl);
  const [extraArgs, setExtraArgs] = useState(defaultBackendArg);
  const [localVersion, setLocalVersion] = useState<VersionInfo | null>(null);
  const [manifest, setManifest] = useState<UpdateManifest | null>(null);
  const [launcherStatus, setLauncherStatus] = useState<LauncherStatus | null>(null);
  const [status, setStatus] = useState("准备就绪");
  const [busy, setBusy] = useState(false);

  const backendBaseUrl = useMemo(() => getBackendBaseUrl(manifestUrl), [manifestUrl]);

  const updateState = useMemo(() => {
    if (!localVersion || !manifest) {
      return "等待检查";
    }

    return manifest.version === localVersion.version ? "已是最新" : "发现更新";
  }, [localVersion, manifest]);

  useEffect(() => {
    void refreshServiceStatus();
  }, [backendBaseUrl]);

  async function runAction(action: () => Promise<string>) {
    setBusy(true);
    try {
      setStatus(await action());
    } catch (error) {
      setStatus(error instanceof Error ? error.message : String(error));
    } finally {
      setBusy(false);
    }
  }

  async function refreshServiceStatus() {
    try {
      const response = await fetch(`${backendBaseUrl}/api/launcher/status`);
      if (!response.ok) {
        setLauncherStatus(null);
        setStatus(`后端连接失败：HTTP ${response.status}`);
        return;
      }

      const envelope = await response.json();
      setLauncherStatus(envelope.data ?? envelope);
    } catch (error) {
      setLauncherStatus(null);
      setStatus(error instanceof Error ? `后端连接失败：${error.message}` : "后端连接失败");
    }
  }

  async function checkLocalVersion() {
    await runAction(async () => {
      const version = await invoke<VersionInfo>("get_local_version", {
        gamePath: installPath,
      });
      setLocalVersion(version);
      return `本地版本：${version.version}`;
    });
  }

  async function fetchRemoteManifest() {
    await runAction(async () => {
      const nextManifest = await invoke<UpdateManifest>("fetch_manifest", {
        url: manifestUrl,
      });
      setManifest(nextManifest);
      return `远端版本：${nextManifest.version}，文件数：${nextManifest.files.length}`;
    });
  }

  async function checkUpdate() {
    await runAction(async () => {
      if (!localVersion || !manifest) {
        return "请先检查本地版本并拉取远端清单";
      }

      const hasUpdate = await invoke<boolean>("check_update", {
        currentVersion: localVersion.version,
        manifest,
      });
      return hasUpdate ? `发现新版本 ${manifest.version}` : "当前客户端已是最新版本";
    });
  }

  async function verifyFiles() {
    await runAction(async () => {
      if (!manifest) {
        return "请先拉取远端清单";
      }

      const missingFiles = await invoke<string[]>("repair_game", {
        gamePath: installPath,
        manifest,
      });

      if (missingFiles.length === 0) {
        return "文件校验通过";
      }

      return `需要修复 ${missingFiles.length} 个文件：${missingFiles.slice(0, 5).join(", ")}`;
    });
  }

  async function launchGame() {
    await runAction(async () => {
      const args = extraArgs
        .split(" ")
        .map((item) => item.trim())
        .filter(Boolean);

      await invoke("launch_game", {
        executablePath,
        args,
      });

      return "游戏客户端已启动";
    });
  }

  async function openLogs() {
    await runAction(async () => {
      await invoke("open_log_folder", {
        gamePath: installPath,
      });
      return "已打开日志目录";
    });
  }

  return (
    <main className="launcher-shell">
      <section className="hero">
        <div>
          <p className="eyebrow">Divine Beasts Arena</p>
          <h1>五灵争霸启动器</h1>
          <p className="subtitle">
            检查版本、校验文件、查看后端状态并启动本地游戏客户端。
          </p>
        </div>
        <div className="status-panel">
          <span className="status-label">当前状态</span>
          <strong>{updateState}</strong>
          <p>{status}</p>
        </div>
      </section>

      <section className="settings-grid">
        <label>
          <span>安装目录</span>
          <input value={installPath} onChange={(event) => setInstallPath(event.target.value)} />
        </label>
        <label>
          <span>客户端可执行文件</span>
          <input value={executablePath} onChange={(event) => setExecutablePath(event.target.value)} />
        </label>
        <label>
          <span>远端清单地址</span>
          <input value={manifestUrl} onChange={(event) => setManifestUrl(event.target.value)} />
        </label>
        <label>
          <span>启动参数</span>
          <input value={extraArgs} onChange={(event) => setExtraArgs(event.target.value)} />
        </label>
      </section>

      <section className="action-bar" aria-label="启动器操作">
        <button onClick={checkLocalVersion} disabled={busy}>
          检查本地版本
        </button>
        <button onClick={fetchRemoteManifest} disabled={busy}>
          拉取远端清单
        </button>
        <button onClick={checkUpdate} disabled={busy}>
          检查更新
        </button>
        <button onClick={verifyFiles} disabled={busy}>
          校验文件
        </button>
        <button onClick={openLogs} disabled={busy}>
          打开日志
        </button>
        <button className="primary" onClick={launchGame} disabled={busy}>
          启动游戏
        </button>
      </section>

      <section className="details-grid">
        <article>
          <h2>本地版本</h2>
          <p>{localVersion?.version ?? "未检查"}</p>
          <small>{localVersion?.path ?? installPath}</small>
        </article>
        <article>
          <h2>远端清单</h2>
          <p>{manifest?.version ?? "未拉取"}</p>
          <small>{manifest ? `${manifest.files.length} 个文件` : manifestUrl}</small>
        </article>
        <article>
          <h2>后端服务</h2>
          <p>{launcherStatus?.status ?? "未连接"}</p>
          <small>
            {launcherStatus
              ? `版本 ${launcherStatus.latestVersion}，活跃服务器 ${launcherStatus.activeGameServers}`
              : backendBaseUrl}
          </small>
        </article>
      </section>
    </main>
  );
}

export default App;
