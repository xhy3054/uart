当前驱动和程序至适用 CH340芯片的USB转串口：
1. 解压CH340_driver.rar，将ch34x.ko 拷贝至7035 中，insmod ch34x.ko 加载驱动；

2. 将send.c 拷贝至7035中，编译：
gcc send-o send

3. 将read.c 拷贝至PC或其他设备中，编译：
gcc read.c -o read

4. 串口数据帧格式：0xAA+0xAA+序号+数据长度+50字节数据+校验和，支持错误重传。
将USB转TTL插入7035 USB接口中，将另一端接入PC或其他设备中。

在PC接收端运行：
./read  /dev/ttyUSB0  1  receive.txt
接收端接收到的内容就会保存至receive.txt，这里的receive.txt 可任意命名。

3. 在7035运行：
./send  /dev/ttyUSB0  0  hello.txt
7035会通过/dev/ttyUSB0 将hello.txt 文档发送至接收端，这里的 hello.txt 可任意命名。

