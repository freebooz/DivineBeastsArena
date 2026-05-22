/*
中文阅读说明：
- 所属应用：DBA_GameLauncher 游戏启动器。
- 文件职责：Tauri Rust 后端命令实现，负责本地文件、下载校验、启动游戏等高权限能力。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fs;
use std::path::PathBuf;
use sha2::{Sha256, Digest};

#[derive(Debug, Serialize, Deserialize)]
pub struct VersionInfo {
    pub version: String,
    pub path: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ManifestFile {
    pub name: String,
    pub sha256: String,
    pub size: u64,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct UpdateManifest {
    pub version: String,
    pub files: Vec<ManifestFile>,
}

#[tauri::command]
fn get_local_version(game_path: String) -> Result<VersionInfo, String> {
    let version_file = PathBuf::from(&game_path).join("version.txt");
    if version_file.exists() {
        let version = fs::read_to_string(&version_file)
            .map_err(|e| e.to_string())?
            .trim()
            .to_string();
        Ok(VersionInfo { version, path: game_path })
    } else {
        Ok(VersionInfo {
            version: "0.0.0".to_string(),
            path: game_path,
        })
    }
}

#[tauri::command]
fn fetch_manifest(url: String) -> Result<UpdateManifest, String> {
    let response = ureq::get(&url)
        .call()
        .map_err(|e| e.to_string())?
        .into_string()
        .map_err(|e| e.to_string())?;

    serde_json::from_str(&response).map_err(|e| e.to_string())
}

#[tauri::command]
fn check_update(current_version: String, manifest: UpdateManifest) -> bool {
    let current = semver::Version::parse(&current_version).unwrap_or_default();
    let latest = semver::Version::parse(&manifest.version).unwrap_or_default();
    latest > current
}

#[tauri::command]
fn download_file(url: String, destination: String) -> Result<(), String> {
    let response = ureq::get(&url)
        .call()
        .map_err(|e| e.to_string())?;

    let mut file = fs::File::create(&destination)
        .map_err(|e| e.to_string())?;

    response.into_reader()
        .copy_to(&mut file)
        .map_err(|e| e.to_string())?;

    Ok(())
}

#[tauri::command]
fn verify_file_sha256(file_path: String, expected_hash: String) -> Result<bool, String> {
    let contents = fs::read(&file_path).map_err(|e| e.to_string())?;
    let mut hasher = Sha256::new();
    hasher.update(&contents);
    let result = hasher.finalize();
    let hash = hex::encode(result);

    Ok(hash.to_lowercase() == expected_hash.to_lowercase())
}

#[tauri::command]
fn repair_game(game_path: String, manifest: UpdateManifest) -> Result<Vec<String>, String> {
    let mut repaired = Vec::new();
    let base_path = PathBuf::from(&game_path);

    for file in manifest.files {
        let file_path = base_path.join(&file.name);
        if !file_path.exists() || !verify_file_sha256(file_path.to_string_lossy().to_string(), file.sha256.clone())? {
            // In a real implementation, this would redownload the file
            repaired.push(file.name);
        }
    }

    Ok(repaired)
}

#[tauri::command]
fn launch_game(executable_path: String, args: Vec<String>) -> Result<(), String> {
    std::process::Command::new(&executable_path)
        .args(&args)
        .spawn()
        .map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
fn open_log_folder(game_path: String) -> Result<(), String> {
    let log_path = PathBuf::from(&game_path).join("logs");
    opener::open(&log_path).map_err(|e| e.to_string())
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![
            get_local_version,
            fetch_manifest,
            check_update,
            download_file,
            verify_file_sha256,
            repair_game,
            launch_game,
            open_log_folder
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}