TCP调试工具：

用Qt5.9实现

服务器是多线程的，可同时接受多个客户端的连接，同时接收多个消息；只能选择其中一个客户端来回复消息。

客户端是单线程的，只能发起一个TCP连接。

本测试工具的服务器和客户端之间可直接通信。

TcpDebugTool_boxed.exe是打包好的可执行程序，在windows环境下直接打开即可使用。