# Qiyu Native SDK / 奇遇 Native SDK

- [English](#english)
- [中文](#中文)

---

## English

Qiyu Native SDK provides resources for native development and third-party engine integration to develop Qiyu VR apps that use Android.

[![](https://pic2.iqiyipic.com/lequ/20210713/download.png)](https://static-d.iqiyi.com/lequ/20220310/Qiyu_Native_SDK_v1.0.0.rar)

v1.0.0 | Published 2022-03-10

Click the download button to indicate that you have read and agree to the [License](https://dev-qiyu.iqiyi.com/doc/develop/unity/sdklicense.html)

### Native SDK v1.0.0

Published 2022-03-10

* **Key Features**
1. Support Foveation Rendering
2. Support MultiView
3. Provide QIYU Platform Solution (QIYU Account , DeepLink, PlayerPrefs)
4. Support [QIYU Performance Tool(opens new window)](https://dev-qiyu.iqiyi.com/doc/develop/tools/download.html)
* **Known Issues**
1. Not Support Vulkan in this version
2. When casting during running SDK App, and then back to the App, it might have black screen issue.
3. When casting to PC and exist SDK App, it might have black screen issue.

### Native SDK v0.1.0 Beta

Published 2021-12-27

* **Key Features**
1. Support QIYU3 and QIYU Dream devices.
2. Provide the data, button and vibration APIs of QIYU controller
3. Provide Eye Resolution Scale function, support change the scale of EyeBuffer (Default 0.7 is recommended) to improve FPS
4. Support Tracking Origin Mode
5. Provide Boundary APIs

---

## 中文

奇遇Native SDK支持开发者在Native环境下开发奇遇VR内容。

[![](https://pic1.iqiyipic.com/lequ/20210708/946ee738e67f43c1980176737fe2111c.png)](https://static-d.iqiyi.com/lequ/20220310/Qiyu_Native_SDK_v1.0.0.rar)

v1.0.0 | 发布时间 2022-03-10

点击下载，即代表您已阅读并同意[许可条款](https://dev-qiyu.iqiyi.com/doc/zh/develop/unity/sdklicense/)

### Native SDK v1.0.0

发布时间 2022-03-10

* **新增功能**
1. 支持注视点渲染
2. 支持Multi view
3. 提供奇遇平台方案（登录账户、深度链接、用户数据存档）
4. 支持[奇遇性能分析工具(opens new window)](https://dev-qiyu.iqiyi.com/doc/zh/develop/tools/download.html)
* **已知问题**
1. 当前版本不支持Vulkan
2. SDK App运行中投屏，再返回App概率性灰屏
3. 投屏到PC时，SDK App退出概率性灰屏

### Native SDK v0.1.0 Beta

发布时间 2021-12-27

* **新增功能**
1. 支持奇遇3、奇遇Dream一体机设备
2. 提供奇遇手柄数据、按键、震动等功能接口
3. 提供Eye Resolution Scale功能，支持修改Eyebuffer大小（默认推荐0.7），可有效提高帧率
4. 支持真实身高追踪模式
5. 提供设备安全围栏APIs
