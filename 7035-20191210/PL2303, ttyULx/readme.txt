serial_read.c serial_send.c ���ڳ���ʹ�÷�Χ��
1. PL�����д��ڣ�
/dev/ttyUL3  /dev/ttyUL4  /dev/ttyUL5  /dev/ttyUL6  /dev/ttyUL7  /dev/ttyUL8  /dev/ttyUL9

2. PL2303оƬUSBת���� /dev/ttyUSB0 (ע��CH340оƬ��USBת���ڲ���ʹ�øó��򣡣�

ʹ�÷�����
1. ���նˣ�PC��7035�壩
gcc serial_read.c -o serial_read

���ն˱�����ɺ������н��ճ������£�
./serial_read /dev/ttyUL5 receive.txt�������receive.txt ������������

2. ���Ͷˣ�7035�壩
gcc serial_send.c -o serial_send
������ɺ��ڽ��ն˳���������֮��ִ����������ͣ�
./serial_send /dev/ttyUL4 hello.txt������� hello.txt ������������

�ȴ����ն˽�����ɺ�receive.txt ��Ϊ���Ͷ˴��������hello.txt ��


