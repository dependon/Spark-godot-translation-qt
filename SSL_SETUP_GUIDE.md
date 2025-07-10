# OpenSSL配置指南

## 问题描述
如果您看到以下错误信息：
```
OpenSSL支持情况: false
```

这表示Qt无法找到或加载OpenSSL库文件，这会导致HTTPS请求失败。

## 解决方案

### 方案1：下载OpenSSL库文件（推荐）

1. 下载适用于Qt 5.12.4的OpenSSL 1.1.x库文件：
   - 访问：https://slproweb.com/products/Win32OpenSSL.html
   - 下载 "Win64 OpenSSL v1.1.1" 或类似版本
   - 选择轻量版本（Light）即可

2. 安装后，将以下文件复制到您的应用程序目录：
   ```
   libssl-1_1-x64.dll
   libcrypto-1_1-x64.dll
   ```
   
   或者复制到Qt的bin目录中：
   ```
   C:\Qt\5.12.4\msvc2017_64\bin\
   ```

### 方案2：使用预编译的DLL文件

1. 从Qt官方或可信源下载以下文件：
   - `ssleay32.dll`
   - `libeay32.dll`
   
2. 将这些文件放置在：
   - 应用程序的可执行文件目录
   - 或系统PATH环境变量包含的目录
   - 或Qt安装目录的bin文件夹

### 方案3：修改系统PATH环境变量

1. 如果您已经安装了OpenSSL，确保其bin目录在系统PATH中
2. 重启应用程序以使PATH变更生效

## 验证安装

重新运行程序，检查日志输出：
- 如果显示 `OpenSSL支持情况: true`，则配置成功
- 如果仍显示 `false`，请检查DLL文件是否正确放置

## 注意事项

1. **版本兼容性**：Qt 5.12.4需要OpenSSL 1.1.x版本，不支持OpenSSL 3.x
2. **架构匹配**：确保OpenSSL库的架构（32位/64位）与您的Qt版本匹配
3. **文件权限**：确保DLL文件具有适当的读取权限

## 常见错误

- **找不到DLL**：检查文件路径和文件名
- **版本不匹配**：确保使用OpenSSL 1.1.x版本
- **架构不匹配**：64位Qt需要64位OpenSSL库

## 技术支持

如果问题仍然存在，请提供以下信息：
- Qt版本
- OpenSSL版本
- 操作系统版本
- 详细的错误日志