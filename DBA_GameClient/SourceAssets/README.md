# SourceAssets

本目录保存已生成或已整理、等待通过 Unreal Editor 导入的中间资源。

- SourceArt 是可编辑源文件；SourceAssets 是导入前中间产物；Content 是运行时资产。
- 文件名应匹配最终资产语义，并使用 DBA 前缀。
- 导入脚本放在 Tools/Editor 或 Scripts，不能把生成逻辑混入资源目录。
- 导入完成后仍有复用价值的文件可以保留，否则应在独立清理变更中移除。

