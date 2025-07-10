# Spark Godot 翻译工具

这是一个基于Qt 5.12.4的CSV翻译工具，专为Godot项目设计，支持使用百度翻译API进行多语言翻译。

## 功能特性

- 支持CSV文件的拖拽和选择
- 集成百度翻译API
- 支持28种目标语言翻译
- 实时翻译进度显示
- 翻译缓存机制
- 多线程翻译处理
- 可选择是否翻译已有内容
- 详细的日志记录

## 系统要求

- Qt 5.12.4 或更高版本
- C++17 编译器
- CMake 3.16+ 或 qmake
- 百度翻译API账号

## 安装和构建

### 使用qmake构建

```bash
qmake
make
```

### 使用CMake构建

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Qt环境配置

如果CMake找不到Qt5，请设置Qt5的安装路径：

```bash
# Windows示例
set CMAKE_PREFIX_PATH=C:\Qt\5.12.4\msvc2017_64
cmake ..

# 或者直接指定Qt5_DIR
set Qt5_DIR=C:\Qt\5.12.4\msvc2017_64\lib\cmake\Qt5
cmake ..
```

## 使用说明

### 1. 配置百度翻译API

1. 在百度翻译开放平台注册账号：https://fanyi-api.baidu.com/
2. 创建应用获取App ID和Secret Key
3. 在软件中填入App ID和Secret Key
4. 点击"保存设置"按钮

### 2. 加载CSV文件

- 点击"选择文件"按钮选择CSV文件
- 或者直接拖拽CSV文件到文件路径输入框
- 或者手动输入CSV文件路径

### 3. 配置翻译选项

1. 从"源语言"下拉框选择包含原文的列
2. 在目标语言区域选择需要翻译的语言
   - 使用"全选"按钮选择所有语言
   - 使用"取消全选"按钮取消所有选择
   - 手动勾选/取消特定语言
3. 选择是否翻译已有内容（勾选"翻译已翻译内容"复选框）

### 4. 开始翻译

1. 点击"开始翻译"按钮
2. 观察进度条和日志输出
3. 翻译过程中可以点击"停止翻译"按钮中止
4. 翻译完成后会自动保存为"原文件名_translated.csv"

### 5. 查看结果

- 翻译结果会实时显示在日志区域
- 完成后会生成新的CSV文件包含所有翻译结果
- 可以使用"清空日志"按钮清理日志显示

## 支持的语言

工具支持以下28种目标语言：

- 英语 (en)
- 中文 (zh)
- 繁体中文 (cht)
- 粤语 (yue)
- 文言文 (wyw)
- 日语 (jp)
- 韩语 (kor)
- 西班牙语 (spa)
- 法语 (fra)
- 泰语 (th)
- 阿拉伯语 (ara)
- 俄语 (ru)
- 葡萄牙语 (pt)
- 德语 (de)
- 意大利语 (it)
- 希腊语 (el)
- 荷兰语 (nl)
- 波兰语 (pl)
- 保加利亚语 (bul)
- 爱沙尼亚语 (est)
- 丹麦语 (dan)
- 芬兰语 (fin)
- 捷克语 (cs)
- 罗马尼亚语 (rom)
- 斯洛文尼亚语 (slo)
- 瑞典语 (swe)
- 匈牙利语 (hu)
- 越南语 (vie)

## 文件结构

```
Spark-godot-translation/
├── main.cpp                 # 应用程序入口
├── mainwindow.h            # 主窗口头文件
├── mainwindow.cpp          # 主窗口实现
├── mainwindow.ui           # UI界面文件
├── CMakeLists.txt          # CMake构建文件
├── Spark-godot-translation.pro  # qmake项目文件
└── README.md               # 说明文档
```

## 注意事项

1. **API限制**：百度翻译API有调用频率限制，工具已内置延时机制
2. **文件格式**：仅支持UTF-8编码的CSV文件
3. **网络连接**：翻译过程需要稳定的网络连接
4. **文件备份**：建议在翻译前备份原始CSV文件
5. **大文件处理**：对于大型CSV文件，翻译可能需要较长时间

## 故障排除

### 编译问题

1. **Qt找不到**：确保Qt 5.12.4已正确安装并设置环境变量
2. **编译器错误**：确保使用支持C++17的编译器
3. **依赖缺失**：检查是否安装了所需的Qt模块（Core, Widgets, Network, Concurrent）

### 运行问题

1. **API错误**：检查百度翻译API配置是否正确
2. **文件加载失败**：确保CSV文件格式正确且为UTF-8编码
3. **翻译失败**：检查网络连接和API配额

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 贡献

欢迎提交Issue和Pull Request来改进这个工具。

## 联系方式

如有问题或建议，请通过GitHub Issues联系我们。