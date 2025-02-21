Stanford CS 144 Networking Lab
==============================

项目整体框架分为util目录下的工具类，apps的应用类，以及test的测试类。简单的事件循环eventloop类，负责同时处理多个文件描述符，采用poll类型进行fd的注册。
linux下一切皆文件的思路，这里重构了文件描述符file_descriptor,socket也为文件描述符，故继承自file_descriptor，也就是说，在此次代码框架下，作者重写了socket
而不是直接调用库文件。且作者重新构建了地址族类，adress，利用c++范型编程，该地址类可以兼容各种初始化方式（ipv4,ipv6，域名）等等，具体可以区看adress.cc。工具类对理解整体框架可能会有帮助。
lab0分为工具实验，即在shell命令行利用工具进行http访问。代码实验，在webget中利用socket工具对指定域名（ip）进行连接，并调用writer方法进行通信（使用http协议）。另一个代码实验，是构建一个安全读写字节流
具体而言，在byte_strean.cc中构建基类（字节流），添加控制状态，创造基类的另一个视角（读和写类），要求读和写类不能额外添加成员，读写共同继承流类（具体测试框架代码没有细看，c++的多态运用的淋漓尽致），
使用不同的函数进行安全的读写。（可以自己添加控制变量，即何时容器为空，文件是否关闭等），代码在byte_stream实现。没看测试代码有一点小问题，如何保证临界资源的安全性？

These labs are open to the public under the (friendly) request that to
preserve their value as a teaching tool, solutions not be posted
publicly by anybody.

Website: https://cs144.stanford.edu

To set up the build system: `cmake -S . -B build`

To compile: `cmake --build build`

To run tests: `cmake --build build --target test`

To run speed benchmarks: `cmake --build build --target speed`

To run clang-tidy (which suggests improvements): `cmake --build build --target tidy`

To format code: `cmake --build build --target format`

