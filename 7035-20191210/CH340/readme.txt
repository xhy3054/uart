��ǰ�����ͳ��������� CH340оƬ��USBת���ڣ�
1. ��ѹCH340_driver.rar����ch34x.ko ������7035 �У�insmod ch34x.ko ����������

2. ��send.c ������7035�У����룺
gcc send-o send

3. ��read.c ������PC�������豸�У����룺
gcc read.c -o read

4. ��������֡��ʽ��0xAA+0xAA+���+���ݳ���+50�ֽ�����+У��ͣ�֧�ִ����ش���
��USBתTTL����7035 USB�ӿ��У�����һ�˽���PC�������豸�С�

��PC���ն����У�
./read  /dev/ttyUSB0  1  receive.txt
���ն˽��յ������ݾͻᱣ����receive.txt�������receive.txt ������������

3. ��7035���У�
./send  /dev/ttyUSB0  0  hello.txt
7035��ͨ��/dev/ttyUSB0 ��hello.txt �ĵ����������նˣ������ hello.txt ������������

