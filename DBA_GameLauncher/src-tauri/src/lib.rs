/*
中文阅读说明：
- 所属应用：DBA_GameLauncher 游戏启动器。
- 文件职责：Tauri Rust 后端命令实现，负责本地文件、下载校验、启动游戏等高权限能力。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

use serde::{Deserialize, Serialize};
use std::collections::HashSet;
use std::fs;
use std::io::{self, Read};
use std::path::{Component, Path, PathBuf};
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
    #[serde(default, rename = "downloadUrl")]
    pub download_url: String,
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
    validate_network_url(&url, "ManifestUrl")?;

    let response = ureq::get(&url)
        .call()
        .map_err(|e| e.to_string())?
        .into_string()
        .map_err(|e| e.to_string())?;

    let manifest = serde_json::from_str(&response).map_err(|e| e.to_string())?;
    validate_manifest(&manifest)?;
    Ok(manifest)
}

#[tauri::command]
fn check_update(current_version: String, manifest: UpdateManifest) -> bool {
    let current = parse_launcher_version(&current_version).unwrap_or((0, 0, 0, 0));
    let latest = parse_launcher_version(&manifest.version).unwrap_or((0, 0, 0, 0));
    latest > current
}

fn download_file(url: String, destination: String) -> Result<(), String> {
    let response = ureq::get(&url)
        .call()
        .map_err(|e| e.to_string())?;

    let destination_path = PathBuf::from(&destination);
    if let Some(parent) = destination_path.parent() {
        fs::create_dir_all(parent).map_err(|e| e.to_string())?;
    }

    let mut file = fs::File::create(&destination_path)
        .map_err(|e| e.to_string())?;

    let mut reader = response.into_reader();
    io::copy(&mut reader, &mut file).map_err(|e| e.to_string())?;

    Ok(())
}

#[tauri::command]
fn verify_file_sha256(file_path: String, expected_hash: String) -> Result<bool, String> {
    let mut file = fs::File::open(&file_path).map_err(|e| e.to_string())?;
    let mut hasher = Sha256::new();
    let mut buffer = [0_u8; 64 * 1024];

    loop {
        let read = file.read(&mut buffer).map_err(|e| e.to_string())?;
        if read == 0 {
            break;
        }
        hasher.update(&buffer[..read]);
    }

    let hash = hex::encode(hasher.finalize());

    Ok(hash.to_lowercase() == expected_hash.to_lowercase())
}

#[tauri::command]
fn repair_game(game_path: String, manifest: UpdateManifest) -> Result<Vec<String>, String> {
    validate_manifest(&manifest)?;

    let mut repaired = Vec::new();
    let base_path = PathBuf::from(&game_path);
    fs::create_dir_all(&base_path).map_err(|e| e.to_string())?;

    for (index, file) in manifest.files.iter().enumerate() {
        let file_path = safe_join(&base_path, &file.name)?;
        if file_path.exists() && verify_file_sha256(file_path.to_string_lossy().to_string(), file.sha256.clone())? {
            continue;
        }

        let url = build_file_download_url(&manifest.download_url, &file.name, manifest.files.len(), index)?;
        let temp_path = file_path.with_extension("download");
        download_file(url, temp_path.to_string_lossy().to_string())?;

        if !verify_file_sha256(temp_path.to_string_lossy().to_string(), file.sha256.clone())? {
            let _ = fs::remove_file(&temp_path);
            return Err(format!("Downloaded file failed SHA256 verification: {}", file.name));
        }

        if let Some(parent) = file_path.parent() {
            fs::create_dir_all(parent).map_err(|e| e.to_string())?;
        }
        fs::rename(&temp_path, &file_path).map_err(|e| e.to_string())?;
        repaired.push(file.name.clone());
    }

    fs::write(base_path.join("version.txt"), manifest.version.trim()).map_err(|e| e.to_string())?;

    Ok(repaired)
}

fn safe_join(base_path: &Path, relative_name: &str) -> Result<PathBuf, String> {
    let relative = Path::new(relative_name);
    if relative.is_absolute() {
        return Err(format!("Manifest file path must be relative: {}", relative_name));
    }

    if relative
        .components()
        .any(|component| matches!(component, Component::ParentDir | Component::Prefix(_) | Component::RootDir))
    {
        return Err(format!("Manifest file path is not allowed: {}", relative_name));
    }

    Ok(base_path.join(relative))
}

fn build_file_download_url(base_url: &str, file_name: &str, total_files: usize, index: usize) -> Result<String, String> {
    if base_url.trim().is_empty() {
        return Err(format!("Manifest has no downloadUrl for {}", file_name));
    }

    let trimmed = base_url.trim();
    if trimmed.ends_with('/') {
        return Ok(format!("{}{}", trimmed, file_name));
    }

    if total_files == 1 && index == 0 {
        return Ok(trimmed.to_string());
    }

    Err(format!(
        "Manifest downloadUrl must point to a file or end with '/' for multi-file repair: {}",
        trimmed
    ))
}

fn validate_manifest(manifest: &UpdateManifest) -> Result<(), String> {
    if parse_launcher_version(&manifest.version).is_none() {
        return Err(format!("Manifest version must be a valid semantic version: {}", manifest.version));
    }

    if manifest.download_url.trim().is_empty() {
        return Err("Manifest downloadUrl is required.".to_string());
    }
    validate_network_url(&manifest.download_url, "Manifest downloadUrl")?;

    if manifest.files.is_empty() {
        return Err("Manifest must contain at least one file.".to_string());
    }

    let mut seen_names = HashSet::new();
    for file in &manifest.files {
        if file.name.trim().is_empty() {
            return Err("Manifest file name is required.".to_string());
        }

        safe_join(Path::new("."), &file.name)?;

        let normalized_name = file.name.replace('\\', "/").to_ascii_lowercase();
        if !seen_names.insert(normalized_name) {
            return Err(format!("Manifest contains duplicate file name: {}", file.name));
        }

        let is_sha256 = file.sha256.len() == 64 && file.sha256.chars().all(|value| value.is_ascii_hexdigit());
        if !is_sha256 {
            return Err(format!("Manifest file has invalid SHA256: {}", file.name));
        }

        if file.size == 0 {
            return Err(format!("Manifest file size must be greater than zero: {}", file.name));
        }
    }

    if manifest.files.len() > 1 && !manifest.download_url.trim().ends_with('/') {
        return Err("Manifest downloadUrl must end with '/' when multiple files are listed.".to_string());
    }

    Ok(())
}

fn validate_network_url(url: &str, field_name: &str) -> Result<(), String> {
    let trimmed = url.trim();
    if let Some(rest) = trimmed.strip_prefix("https://") {
        let host = extract_url_host(rest);
        if host.is_empty() {
            return Err(format!("{} must include a valid host: {}", field_name, trimmed));
        }
        return Ok(());
    }

    if let Some(rest) = trimmed.strip_prefix("http://") {
        let host = extract_url_host(rest);
        if host.is_empty() {
            return Err(format!("{} must include a valid host: {}", field_name, trimmed));
        }

        if matches!(host, "localhost" | "127.0.0.1" | "::1") {
            return Ok(());
        }
    }

    Err(format!(
        "{} must use HTTPS unless it points to localhost for development validation: {}",
        field_name, trimmed
    ))
}

fn extract_url_host(url_after_scheme: &str) -> &str {
    if let Some(ipv6_rest) = url_after_scheme.strip_prefix('[') {
        return ipv6_rest
            .split_once(']')
            .map(|(host, _)| host)
            .unwrap_or_default();
    }

    url_after_scheme
        .split(['/', ':', '?', '#'])
        .next()
        .unwrap_or_default()
}

fn parse_launcher_version(version: &str) -> Option<(u64, u64, u64, u64)> {
    let parts = version.trim().split('.').collect::<Vec<_>>();
    if !(parts.len() == 3 || parts.len() == 4) {
        return None;
    }

    let major = parts[0].parse::<u64>().ok()?;
    let minor = parts[1].parse::<u64>().ok()?;
    let patch = parts[2].parse::<u64>().ok()?;
    let revision = if parts.len() == 4 {
        parts[3].parse::<u64>().ok()?
    } else {
        0
    };

    Some((major, minor, patch, revision))
}

#[tauri::command]
fn launch_game(game_path: String, executable_path: String, args: Vec<String>) -> Result<(), String> {
    let base_path = PathBuf::from(&game_path)
        .canonicalize()
        .map_err(|e| format!("Invalid game path: {}", e))?;
    let executable = PathBuf::from(&executable_path)
        .canonicalize()
        .map_err(|e| format!("Invalid executable path: {}", e))?;

    if !executable.starts_with(&base_path) {
        return Err("Executable must be inside the configured game installation path.".to_string());
    }

    let file_name = executable
        .file_name()
        .and_then(|value| value.to_str())
        .unwrap_or_default()
        .to_ascii_lowercase();
    if !file_name.starts_with("divinebeastsarena") || !file_name.ends_with(".exe") {
        return Err("Executable must be a DivineBeastsArena Windows client binary.".to_string());
    }

    std::process::Command::new(&executable)
        .args(&args)
        .spawn()
        .map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
fn open_log_folder(game_path: String) -> Result<(), String> {
    let log_path = PathBuf::from(&game_path).join("logs");
    fs::create_dir_all(&log_path).map_err(|e| e.to_string())?;
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
            verify_file_sha256,
            repair_game,
            launch_game,
            open_log_folder
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::{Read, Write};
    use std::net::TcpListener;
    use std::path::Path;
    use std::thread;

    fn manifest_with_file(name: &str, sha256: &str, download_url: &str) -> UpdateManifest {
        UpdateManifest {
            version: "1.2.3".to_string(),
            download_url: download_url.to_string(),
            files: vec![ManifestFile {
                name: name.to_string(),
                sha256: sha256.to_string(),
                size: 16,
            }],
        }
    }

    #[test]
    fn validate_manifest_rejects_empty_files() {
        let manifest = UpdateManifest {
            version: "1.2.3".to_string(),
            download_url: "https://cdn.example.com/releases/".to_string(),
            files: Vec::new(),
        };

        let result = validate_manifest(&manifest);

        assert!(result.is_err());
        assert!(result.unwrap_err().contains("at least one file"));
    }

    #[test]
    fn validate_manifest_rejects_path_traversal() {
        let manifest = manifest_with_file("../Game.exe", &"a".repeat(64), "https://cdn.example.com/releases/");

        let result = validate_manifest(&manifest);

        assert!(result.is_err());
        assert!(result.unwrap_err().contains("not allowed"));
    }

    #[test]
    fn validate_manifest_rejects_invalid_sha256() {
        let manifest = manifest_with_file("Game.exe", "not-a-sha256", "https://cdn.example.com/releases/");

        let result = validate_manifest(&manifest);

        assert!(result.is_err());
        assert!(result.unwrap_err().contains("SHA256"));
    }

    #[test]
    fn validate_manifest_rejects_duplicate_file_names() {
        let manifest = UpdateManifest {
            version: "1.2.3".to_string(),
            download_url: "https://cdn.example.com/releases/".to_string(),
            files: vec![
                ManifestFile {
                    name: "Game.exe".to_string(),
                    sha256: "a".repeat(64),
                    size: 16,
                },
                ManifestFile {
                    name: "Game.exe".to_string(),
                    sha256: "b".repeat(64),
                    size: 32,
                },
            ],
        };

        let result = validate_manifest(&manifest);

        assert!(result.is_err());
        assert!(result.unwrap_err().contains("duplicate"));
    }

    #[test]
    fn validate_manifest_accepts_four_part_backend_versions() {
        let manifest = manifest_with_file(
            "DivineBeastsArena-Windows-1.2.5.0.zip",
            &"a".repeat(64),
            "https://cdn.example.com/releases/DivineBeastsArena-Windows-1.2.5.0.zip",
        );
        let manifest = UpdateManifest {
            version: "1.2.5.0".to_string(),
            ..manifest
        };

        let result = validate_manifest(&manifest);

        assert!(result.is_ok());
    }

    #[test]
    fn validate_manifest_rejects_external_http_download_url() {
        let manifest = manifest_with_file(
            "DivineBeastsArena-Windows-1.2.5.0.zip",
            &"a".repeat(64),
            "http://download.example.com/releases/DivineBeastsArena-Windows-1.2.5.0.zip",
        );

        let result = validate_manifest(&manifest);

        assert!(result.is_err());
        assert!(result.unwrap_err().contains("HTTPS"));
    }

    #[test]
    fn validate_network_url_accepts_ipv6_loopback_http_for_local_validation() {
        let result = validate_network_url("http://[::1]:8080/launcher/manifest.json", "ManifestUrl");

        assert!(result.is_ok());
    }

    #[test]
    fn validate_network_url_rejects_https_url_without_host() {
        let result = validate_network_url("https://", "ManifestUrl");

        assert!(result.is_err());
        assert!(result.unwrap_err().contains("valid host"));
    }

    #[test]
    fn safe_join_keeps_manifest_files_inside_install_path() {
        let joined = safe_join(Path::new("C:/Games/DBA"), "Content/Paks/Game.pak").unwrap();

        assert!(joined.ends_with("Content/Paks/Game.pak"));
    }

    #[test]
    fn build_file_download_url_uses_single_file_url_verbatim() {
        let url = build_file_download_url(
            "https://cdn.example.com/releases/DivineBeastsArena.zip",
            "DivineBeastsArena.zip",
            1,
            0,
        )
        .unwrap();

        assert_eq!(url, "https://cdn.example.com/releases/DivineBeastsArena.zip");
    }

    #[test]
    fn build_file_download_url_appends_name_for_single_file_directory_url() {
        let url = build_file_download_url(
            "https://cdn.example.com/releases/",
            "DivineBeastsArena.zip",
            1,
            0,
        )
        .unwrap();

        assert_eq!(url, "https://cdn.example.com/releases/DivineBeastsArena.zip");
    }

    #[test]
    fn build_file_download_url_requires_directory_for_multi_file_manifest() {
        let result = build_file_download_url(
            "https://cdn.example.com/releases/DivineBeastsArena.zip",
            "Content/Paks/Game.pak",
            2,
            1,
        );

        assert!(result.is_err());
    }

    #[test]
    fn check_update_accepts_four_part_backend_versions() {
        let manifest = manifest_with_file(
            "DivineBeastsArena-Windows-1.2.5.0.zip",
            &"a".repeat(64),
            "https://cdn.example.com/releases/DivineBeastsArena-Windows-1.2.5.0.zip",
        );
        let manifest = UpdateManifest {
            version: "1.2.5.0".to_string(),
            ..manifest
        };

        let has_update = check_update("1.2.4.0".to_string(), manifest);

        assert!(has_update);
    }

    #[test]
    fn repair_game_downloads_local_package_and_persists_version() {
        let package_bytes = b"codex local launcher package";
        let package_sha256 = hex::encode(Sha256::digest(package_bytes));
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let server_url = format!("http://{}", listener.local_addr().unwrap());
        let manifest_json = format!(
            r#"{{
                "version":"1.2.6.0",
                "downloadUrl":"{server_url}/files/",
                "files":[{{"name":"Content/Paks/DBA-Test.pak","sha256":"{package_sha256}","size":{size}}}]
            }}"#,
            size = package_bytes.len()
        );

        let server = thread::spawn(move || {
            for _ in 0..2 {
                let (mut stream, _) = listener.accept().unwrap();
                let mut buffer = [0_u8; 2048];
                let read = stream.read(&mut buffer).unwrap();
                let request = String::from_utf8_lossy(&buffer[..read]);
                let (content_type, body) = if request.starts_with("GET /manifest.json ") {
                    ("application/json", manifest_json.as_bytes().to_vec())
                } else if request.starts_with("GET /files/Content/Paks/DBA-Test.pak ") {
                    ("application/octet-stream", package_bytes.to_vec())
                } else {
                    ("text/plain", b"not found".to_vec())
                };
                let status = if content_type == "text/plain" { "404 Not Found" } else { "200 OK" };
                let header = format!(
                    "HTTP/1.1 {status}\r\nContent-Length: {}\r\nContent-Type: {content_type}\r\nConnection: close\r\n\r\n",
                    body.len()
                );
                stream.write_all(header.as_bytes()).unwrap();
                stream.write_all(&body).unwrap();
            }
        });

        let install_path = std::env::temp_dir().join(format!(
            "dba-launcher-smoke-{}",
            std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap()
                .as_nanos()
        ));

        let manifest = fetch_manifest(format!("{server_url}/manifest.json")).unwrap();
        let repaired = repair_game(install_path.to_string_lossy().to_string(), manifest).unwrap();

        assert_eq!(repaired, vec!["Content/Paks/DBA-Test.pak".to_string()]);
        assert!(install_path.join("Content/Paks/DBA-Test.pak").exists());
        assert_eq!(fs::read_to_string(install_path.join("version.txt")).unwrap(), "1.2.6.0");

        fs::remove_dir_all(&install_path).unwrap();
        server.join().unwrap();
    }
}
