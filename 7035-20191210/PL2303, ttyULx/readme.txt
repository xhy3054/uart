serial_read.c serial_send.c 串口程序使用范围：
1. PL端所有串口：
/dev/ttyUL3  /dev/ttyUL4  /dev/ttyUL5  /dev/ttyUL6  /dev/ttyUL7  /dev/ttyUL8  /dev/ttyUL9

2. PL2303芯片USB转串口 /dev/ttyUSB0 (注：CH340芯片的USB转串口不可使用该程序！）

使用方法：
1. 接收端（PC或7035板）
gcc serial_read.c -o serial_read

接收端编译完成后，先运行接收程序，如下：
./serial_read /dev/ttyUL5 receive.txt，这里的receive.txt 可任意命名。

2. 发送端（7035板）
gcc serial_send.c -o serial_send
编译完成后，在接收端程序已运行之后，执行如下命令发送：
./serial_send /dev/ttyUL4 hello.txt，这里的 hello.txt 可任意命名。

等待接收端接收完成后，receive.txt 即为发送端传输过来的hello.txt 。


