# 4over6 server

## 参考代码

-   https://github.com/believedotchenyu/4Over6
-   https://github.com/PanQL/4over6_server

## 如何运行

```bash
# 配置隧道和nat规则并部署服务器
./run.sh

# 配置永久隧道和nat规则
./tun_setup.sh

# 运行测试客户端
./test_client.py
```

## 文件组成

```
.
├── doc
│   └── report.md
├── Makefile
├── Readme.md
├── report.pdf
├── run.sh
├── src
│   ├── Buffer.h
│   ├── Epoll_Utils.c
│   ├── Keep_Alive.c
│   ├── Macro.h
│   ├── main.c
│   ├── Msg.h
│   ├── Server.c
│   ├── Tun.c
│   ├── User_Info.c
│   └── User_Info.h
├── test_client.py
└── tun_setup.sh

2 directories, 17 files
```
