---
title: AI语音屏幕开关
content_type: TEXT
origin_url: https://wiki.tuya-inc.com:7799/page/2037048491108204608
version: V55
---

# 1. 背景

用户对开关的诉求从「能远程控」升级为「就地看得懂、说得清」——屏显降低学习成本，语音解决双手占用场景

屏幕：屏上运行UI图，语音交互时、实时显示文字（输入、输出）

音频：复杂自然语言对话、大模型级语义理解

pid：lq8pvhbwwzlneoa5

# 2. 硬件架构

<img src="./image_9975774264529359-u04s.png" alt="image_9975774264529359.png" style="width: 800px">

## 2.1. 屏驱

### 2.1.1. 屏驱芯片

屏驱芯片选择型号为D121BBV，除驱动屏幕外，还需和T1模组通过SPI通信、和CI1302屏驱芯片通过UART通信。

### 2.1.2. 显示屏

| 显示屏 | 参数说明 |
| :--- | :--- |
| 型号 | HD40010C40带触摸 |
| 屏幕尺寸 | 4寸/3.95寸TFT液晶屏 |
| 分辨率 | 480*480 |
| 接口类型 | RGB |
| 驱动 | ST7701 |
| 可视视角 | ≥80°（右/左/上/下） |
| 触摸屏 | 电容，IC：GT911 |
| 显示亮度 | ≥400cd/m2 |

### 2.1.3. UI/交互

交互稿：https://design.tuya-inc.com:7799/file/131850344368735?file=131850344368735&page_id=160%3A2775&devMode=true

部分示意图如下：

<table>
<tbody>
  <tr>
    <th style="text-align: center"><p>功能单元</p></th>
    <th style="text-align: center"><p>示意图</p></th>
    <th><p>备注说明</p></th>
  </tr>
  <tr>
    <td style="text-align: center"><p>首页</p><p>（左右滑动）</p><p><a href="https://design.tuya-inc.com:7799/prototype/131850344368735?zs=1&amp;pageId=160%3A2775&amp;layerId=160%3A3454">https://design.tuya-inc.com:7799/prototype/131850344368735?zs=1&amp;pageId=160%3A2775&amp;layerId=160%3A3454</a></p></td>
    <td style="text-align: center"><p><img src="smart/pontos/177452228915831245c02.png" alt="image_6098396692212279.png" style="width: 480px"></p></td>
    <td><p>1、天气、温度</p><p>2、日期、时间</p><p>3、Wi-Fi信号强度</p><p>4、主题背景</p><p>5、开关通道-2路（可控制）</p></td>
  </tr>
  <tr>
    <td style="text-align: center"><p>场景</p><p>（左右滑动）</p><p><a href="https://design.tuya-inc.com:7799/prototype/131850344368735?zs=1&amp;pageId=160%3A2775&amp;layerId=160%3A3601">https://design.tuya-inc.com:7799/prototype/131850344368735?zs=1&amp;pageId=160%3A2775&amp;layerId=160%3A3601</a></p></td>
    <td style="text-align: center"><p><img src="smart/pontos/1774522322974a40da72f.png" alt="image_49047876239237154.png" style="width: 480px"></p></td>
    <td><p>1、天气、温度</p><p>2、日期、时间</p><p>3、Wi-Fi信号强度</p><p>4、场景通道-2路（可控制）</p></td>
  </tr>
  <tr>
    <td style="text-align: center"><p>设置</p><p>（顶部下拉）</p><p><a href="https://design.tuya-inc.com:7799/prototype/131850344368735?zs=1&amp;pageId=160%3A2775&amp;layerId=162%3A05153">https://design.tuya-inc.com:7799/prototype/131850344368735?zs=1&amp;pageId=160%3A2775&amp;layerId=162%3A05153</a></p></td>
    <td style="text-align: center"><p><img src="smart/pontos/1774522381ceccc1db448.png" alt="image_6386477954163177.png" style="width: 480px"></p></td>
    <td><p>1、天气、温度</p><p>2、日期、时间</p><p>3、Wi-Fi信号强度</p><p>4、亮度设置</p><p>5、接近感应</p><p>6、自动休眠</p><p>7、关于：配网</p></td>
  </tr>
  <tr>
    <td><div style="text-align: center"><p>实时显示文字</p></div><div style="text-align: center"><p>（弹窗显示）</p></div></td>
    <td><p></p></td>
    <td><p>实时显示AI语音的输入、输出文字</p></td>
  </tr>
</tbody>
</table>

## 2.2. AI语音

### 2.2.1. AI对话模式

需要支持以下4种AI对话模式：

| AI对话模式 | 模式说明 | 硬件配置要求 |
| :--- | :--- | :--- |
| 长按对话模式 | 唤醒--按键长按<br/>开始--长按按键开始对话<br/>结束--按键松开结束对话<br/>播报--设备播放<br/>打断--按键短按<br/>休眠--30s无对话休眠（时间可配置） | 唤醒按键 |
| 按键对话模式 | 唤醒--按键短按<br/>开始--唤醒后/播报完成随时开始对话<br/>结束--开始对话后500ms无人声结束对话<br/>播报--设备播放，播报完成之后回到开始<br/>打断--按键短按<br/>休眠--30s无对话休眠（时间可配置） | 唤醒按键 |
| 唤醒对话模式 | 唤醒--按键/唤醒词<br/>开始--唤醒后/播报完成随时开始对话<br/>结束--开始对话后500ms无人声结束对话<br/>播报--设备播放<br/>打断--唤醒词<br/>休眠--30s无对话退出唤醒（时间可配置） | 音频回采电路 |
| 自由对话模式 | 唤醒--按键/唤醒词<br/>开始--随时开始对话<br/>结束--开始对话后500ms无人声结束对话<br/>播报--设备播放<br/>打断--唤醒词/任意人声对话打断<br/>休眠--30s无对话休眠（时间可配置） | 音频回采电路 |

【注1】参考技术文档：[Wukong AI 语音模式概念](https://wiki.tuya-inc.com:7799/page/1897891485349580898)

<img src="./17574117531218548ce2c-w9gq.png" alt="" style="width: 400px">

【注2】唤醒对话模式，唤醒词：hey tuya、你好涂鸦、小智同学

【注3】本产品为强电设备，不进入休眠

﻿

### 2.2.2. 音频参数

音频参数设置如下：

| 音频参数设置 | 参数说明 | 备注 |
| :--- | :--- | :--- |
| 音量设置 | Speaker的播放音量<br/>APP端可无级调节<br/>设备端可“分段跳跃式”步进调节 | 0-100%，默认50% |
| 拾音最大时间设置 | Mic最大的拾音时长，超过该时间后停止拾音 | 默认30s |
| 唤醒超时时间设置 | 设备唤醒后，在超时时间内如果没有说话，后面麦克不再拾音，需要重新唤醒 | 默认30s |

﻿

### 2.2.3. 按键/指示灯

按键/指示灯外设如下：

<table>
<tbody>
  <tr>
    <th><p>按键/指示灯</p></th>
    <th><p>可选项</p></th>
    <th><p>功能说明</p></th>
    <th><p>备注</p></th>
  </tr>
  <tr>
    <td><p>唤醒按键</p></td>
    <td><p>必选</p></td>
    <td><ul><li><p>长按对话模式</p></li></ul><p>     长按：唤醒；短按：打断</p><ul><li><p>按键对话模式</p></li></ul><p>     短按：唤醒；短按：打断</p></td>
    <td><p>适用于长按对话模式和单次按键对话模式</p><p>【注】唤醒超时时间为30s</p></td>
  </tr>
  <tr>
    <td><p>状态指示灯</p></td>
    <td><p>可选</p></td>
    <td><ul><li><p>拾音ing：常亮</p></li><li><p>播放ing：500ms亮/灭</p></li></ul></td>
    <td><p>﻿</p></td>
  </tr>
  <tr>
    <td><p>音量+/-按键</p><p>（两个按键）</p></td>
    <td><p>可选</p></td>
    <td><p>音量+按键，短按：步进调节、音量+10%</p><p>音量-按键，短按：步进调节、音量-10%</p></td>
    <td><p>均采用“分段跳跃式”步进调节</p><p>【案例】53% -&gt; 60% -&gt; 70% -&gt; 80%；</p><p>           53% -&gt; 50% -&gt; 40% -&gt; 30%</p></td>
  </tr>
  <tr>
    <td><p>Mute按键</p></td>
    <td><p>可选</p></td>
    <td><p>短按：禁麦/开麦状态切换</p></td>
    <td><p>﻿</p></td>
  </tr>
  <tr>
    <td><p>Mute指示灯</p></td>
    <td><p>可选</p></td>
    <td><p>静音ing：常亮</p><p>解除静音：常灭</p></td>
    <td><p>﻿</p></td>
  </tr>
</tbody>
</table>

﻿

### 2.2.4. 语音芯片

语音芯片选择型号为CI1302，和D121BBV屏驱芯片通过UART通信。

<img src="./image_8795501556934749-sdyq.png" alt="image_8795501556934749.png" style="width: 800px">

﻿﻿

### 2.2.5. 外设：音频

音频外设如下：

| 音频 | 外设说明 | 备注 |
| :--- | :--- | :--- |
| Mic | 1路 | ﻿ |
| Speaker | 1路 | ﻿ |
| 音频回采 | 1路 | 内置板载电路 |

﻿

#### 2.2.5.1. Mic 和 Speaker 选型及相关设计方案

参考T1AI-BOX 语音方案：https://developer.tuya.com/cn/docs/iot/T1AI-BOX-DATA-SHEET?id=Kehlfln9zvtww﻿

<img src="./1757387377f484547fd50-gzlc.png" alt="" style="width: 500px">

#### 2.2.5.2. 声学设计参考

参考文档：

1、语音模组声学结构设计：https://developer.tuya.com/cn/docs/iot/speech-module-reference-for-acoustic-structure-design-of-product-design?id=K9hhl5myc7393

2、声学结构设计规范：https://developer.tuya.com/cn/docs/iot/Acoustic-structure-design?id=Kb97hhu11c9k2

## 2.3. 控制

支持2路继电器输出。

# 3. 软件功能

<table>
<tbody>
  <tr>
    <th><p>功能模块</p></th>
    <th><p>功能点</p></th>
    <th><p>设置/交互入口</p><p>（APP 面板）</p></th>
    <th><p>设置/交互入口（设备屏幕）</p></th>
    <th><p>备注</p></th>
  </tr>
  <tr>
    <td colSpan="1" rowSpan="5"><p>开关</p><p>﻿</p></td>
    <td><p>开/关</p></td>
    <td><p>✅</p></td>
    <td><p>✅</p></td>
    <td><p>﻿详见：<a href="https://wiki.tuya-inc.com:7799/page/73680419">https://wiki.tuya-inc.com:7799/page/73680419</a></p><p>开关名称默认为 Switch1，Switch2，可在 APP 上修改，修改后设备屏幕立即同步</p></td>
  </tr>
  <tr>
    <td><p>定时</p></td>
    <td><p>✅</p></td>
    <td><p>﻿</p></td>
    <td><p>﻿详见：<a href="https://wiki.tuya-inc.com:7799/page/73680421">https://wiki.tuya-inc.com:7799/page/73680421</a></p></td>
  </tr>
  <tr>
    <td><p>倒计时</p></td>
    <td><p>✅</p></td>
    <td><p>﻿</p></td>
    <td><p>﻿详见：<a href="https://wiki.tuya-inc.com:7799/page/73680775">https://wiki.tuya-inc.com:7799/page/73680775</a></p></td>
  </tr>
  <tr>
    <td><p>点动（延时关）</p></td>
    <td><p>✅</p></td>
    <td><p>﻿</p></td>
    <td><p>﻿详见：<a href="https://wiki.tuya-inc.com:7799/page/73680825">https://wiki.tuya-inc.com:7799/page/73680825</a></p></td>
  </tr>
  <tr>
    <td><p>上电状态</p></td>
    <td><p>✅</p></td>
    <td><p>﻿</p></td>
    <td><p>详见：<a href="https://wiki.tuya-inc.com:7799/page/73680858">https://wiki.tuya-inc.com:7799/page/73680858</a></p></td>
  </tr>
  <tr>
    <td><p>场景</p></td>
    <td><p>场景控制</p></td>
    <td><p>✅</p></td>
    <td><p>✅</p></td>
    <td><p>在 APP面板 设置的一键执行场景，可以绑定同步到设备屏幕上显示控制</p><p>绑定时场景列表需过滤，只显示一键执行场景</p><p>同步名称、图标、顺序</p><p>已经绑定的场景，在 APP面板 修改后，需要同步修改的内容</p><p>场景下发成功与否需要有弹框提醒</p><p>*最多支持绑定8个</p></td>
  </tr>
  <tr>
    <td colSpan="1" rowSpan="2"><p>屏幕</p></td>
    <td><p>设置（屏幕亮度、自动熄屏、接近亮屏、关于）</p></td>
    <td><p>﻿</p></td>
    <td><p>✅</p></td>
    <td><p>﻿屏幕亮度：1～100%，初始值为100%</p><p>自动熄屏：关闭｜15秒｜30秒｜1分钟｜5分钟（默认）｜15分钟</p><p>接近亮屏：关闭（默认）（点击屏幕任意处，实现亮屏）；开启：靠近亮屏</p><p>关于：显示 Wi-Fi 连接状态/名称、重置设备</p></td>
  </tr>
  <tr>
    <td><p>首页显示（日期、时间、天气、温度、Wi-Fi 信号强度）</p></td>
    <td><p>﻿</p></td>
    <td><p>✅</p></td>
    <td><p>云端接口</p><p>﻿</p></td>
  </tr>
  <tr>
    <td colSpan="1" rowSpan="5"><p>其他</p></td>
    <td><p>配网</p></td>
    <td><p>✅</p></td>
    <td><p>✅</p></td>
    <td><p>﻿</p></td>
  </tr>
  <tr>
    <td><p>多语言</p></td>
    <td><p>✅</p></td>
    <td><p>✅</p></td>
    <td><p>﻿</p></td>
  </tr>
  <tr>
    <td><p>24/12小时切换</p></td>
    <td><p>✅</p></td>
    <td><p>✅</p></td>
    <td><p>﻿</p></td>
  </tr>
  <tr>
    <td><p>自定义名称同步</p></td>
    <td><p>✅</p></td>
    <td><p></p></td>
    <td><p></p></td>
  </tr>
  <tr>
    <td><p>OTA</p></td>
    <td><p>✅</p></td>
    <td><p>﻿</p></td>
    <td><p>﻿</p></td>
  </tr>
</tbody>
</table>

﻿
