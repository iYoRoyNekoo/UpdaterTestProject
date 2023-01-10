# UpdaterTestProject
应用程序更新通用方案  
更新器客户端采用MSVC MFC编写  
服务端采用PHP编写
#### 依赖项：
[libcurl](https://github.com/curl/curl)
[openssl](https://github.com/openssl/openssl)
[zlib](http://www.zlib.net/)
[jsoncpp](https://github.com/open-source-parsers/jsoncpp)



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