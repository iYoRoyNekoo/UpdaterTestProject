# UpdaterTestProject
应用程序更新通用方案  
更新器客户端采用MSVC MFC编写  
服务端采用PHP编写
#### 依赖项：
[libcurl](https://github.com/curl/curl)
[openssl](https://github.com/openssl/openssl)
[zlib](http://www.zlib.net/)
[jsoncpp](https://github.com/open-source-parsers/jsoncpp)

## 设计思路
原先手头有一个项目，发布之后又对其进行了更新。现希望应用程序能够自动执行更新。  
为应对国内复杂的网络情况，我准备了3个下载线路。使用了DNS解析记录TXT的方式存储节点信息。更新器乃至主程序解析DNS记录后解析出可用的节点列表，再从中选取速度较快的一条拉取数据。  
同时，因为服务器带宽有限，因此想到增量更新。Updater将当前版本号上传至服务器，服务器下发配置文件，Updater根据配置文件下载相依文件包。  

## 使用方式
### 客户端
**Test Program**为样例程序，可参考`CTestProgramDlg::OnInitDialog()`中调用的线程执行检查更新和下载更新器本体。 
更新器的调用方式采用了最简单的命令行传参，共需要传入**3**个参数。若参数缺少或多于，Updater将无法运行。（参数检查在Updater的`OnInitDialog()`函数中实现，你可以修改以实现更多功能。）  
其参数标准如下表：  
| 参数名称 | 数值意义 |
| ---- | ---- |
| --CurrentVersion | 当前主程序版本代号 |
| --CurrentBuild | 当前主程序Build代号 |
| --FilePath | 主程序所在目录（方便更新器写入） |

*注：Build代号需要为纯数字，便于比较 ~~（懒得写了就这样吧咕咕咕）~~ *

你需要在**Updater**中`OnCheckNewVer(LPVOID lpParam)`函数下修改如下参数：
```C++
	//解析节点
	//		参考：https://learn.microsoft.com/zh-cn/troubleshoot/windows/win32/use-dnsquery-resolve-host-names
	status = DnsQuery(
		L"<此处填入你的节点信息主机TXT记录>",	//TXT记录的主机名
		DNS_TYPE_TEXT,					//查询类型TXT
		DNS_QUERY_STANDARD,				//标准查询方式
		NULL,
		&pDnsRecord,
		NULL
	);
	if (status)return 0;//网络异常，无法查询DNS记录
	strLQueryResult = *(pDnsRecord->Data.TXT.pStringArray);
```

### 服务端
上文TXT记录采用了**Json**格式，保存若干URL，请确保该URL下存在**Release.json**文件。
在我的节点记录如下：
```json
[
	"https://ah.api.iyoroy.cn:58146/TestProject/",
	"https://hk.api.iyoroy.cn/TestProject/",
	"https://api.iyoroy.cn/TestProject/"
]
```
**Release.json**内保存最新版本信息，包含两个值：**build**和**release**  
**build**存储最新版本的编译代号（纯数字，方便比较），**release**存储版本名称。样例如下： 
```json
{
    "build":2,
    "release":"v1.0.1-alpha Build002"
}
```
你还需要修改**package-info.json**，其中保存了应用程序不同的更新文件的信息。若某次更新只对其中的几个文件进行了更新，将文件上传至服务器后请修改该json文件中对应数据。样例如下：
其中，`filepath`值保存了服务端文件的相对路径，`local`保存了客户端相对于主程序的相对路径,`generatebuild`保存了当前模块版本，若Updater上传的版本号小于该版本号则说明该模块需要更新。
```json
[
    {
        "filepath":"packages/TestProgram.exe",
        "local":"TestProgram.exe",
        "generatebuild":1
    },
    {
        "filepath":"packages/TestData.json",
        "local":"TestData.json",
        "generatebuild":2
    }
]
```



#### 参考文档：
##### MSVC API:
| 参考原因 | 文档地址 |
| ---- | ---- |
| DNSQuery | https://learn.microsoft.com/zh-cn/troubleshoot/windows/win32/use-dnsquery-resolve-host-names |
| CreateProcess | https://learn.microsoft.com/zh-cn/windows/win32/procthread/creating-processes |
| CreateProcess访问冲突 | https://www.zhaokeli.com/article/8471.html |
| CreateThread | https://learn.microsoft.com/zh-cn/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread |
| URLDownloadToFile | https://learn.microsoft.com/en-us/previous-versions/windows/internet-explorer/ie-developer/platform-apis/ms775123(v=vs.85) |
| URLDownloadToFile | https://blog.csdn.net/dengkangou9055/article/details/102068998 |
| GetModuleFileName | https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamea |
| GetModuleFileName | https://bbs.csdn.net/topics/50276709 |
| Unicode模式传参 | https://github.com/EnderMo/LightFrameInstaller/blob/master/LightFrameInstaller.cpp |

##### libcurl：
| 参考原因 | 文档地址 |
| ---- | ---- |
| 项目地址 | https://github.com/curl/curl |
| 编译参考1 | https://blog.csdn.net/lixiang987654321/article/details/81154613 |
| 编译参考2 | https://blog.csdn.net/cllcgl1314/article/details/90767440 |
| 编译参考3 | https://blog.csdn.net/NSDCODER/article/details/120956826 |
| 编译参考4 | https://blog.csdn.net/huang12041/article/details/108358879 |
| CURLE_PEER_FAILED_VERIFICATION解决 | https://blog.csdn.net/rqf520/article/details/96453865 |
| 获取https协议文本内容1 | https://blog.csdn.net/weixin_39510813/article/details/88978941 |
| 获取https协议文本内容2 | https://blog.csdn.net/stpeace/article/details/77984611 |
| 文件下载损坏问题 | https://www.thinbug.com/q/25427915 |
| 获取文件下载进度 | https://blog.csdn.net/haysonzeng/article/details/108540158 |
| 获取文件下载速度和剩余时间 | https://blog.csdn.net/ixiaochouyu/article/details/47301005 |
| 常见错误代码 | https://www.php.cn/js-tutorial-379132.html |

##### OpenSSL：
| 参考原因 | 文档地址 |
| ---- | ---- |
| 项目地址 | https://github.com/openssl/openssl |
| 版本问题 | https://blog.csdn.net/iMatt/article/details/93321601 |
| 编译参考 | https://blog.csdn.net/MRbrucewayne/article/details/105555681 |

##### PHP：
| 参考原因 | 文档地址 |
| ---- | ---- |
| foreach用法 | https://www.w3cschool.cn/php/php-array-foreach.html |
| json用法 | https://www.runoob.com/php/php-json.html |
| 字符串数字转化 | https://www.php.cn/php-weizijiaocheng-414709.html |
| json长度计算 | https://segmentfault.com/q/1010000016323643 |
| array增加元素 | https://www.php.cn/php-ask-485184.html |