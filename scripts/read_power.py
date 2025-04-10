import hid
import time

def hid_loop_receive(vid, pid):
    try:
        # 1. 检查设备并获取路径
        device_list = hid.enumerate(vid, pid)
        if not device_list:
            raise ValueError(f"未找到设备 VID={vid:04X}, PID={pid:04X}")
        device_path = device_list[0]['path']

        # 2. 连接设备
        device = hid.Device(path=device_path)
        print(f"设备连接成功 | 制造商: {device.manufacturer} | 产品名: {device.product}")

        # 3. 配置为非阻塞模式（关键修改）
        device.nonblocking = True  # 启用非阻塞模式
        report_length = 8
        
        # 4. 主循环（增加退出标志）
        print("开始接收数据 (按Ctrl+C终止)...")
        running = True
        while running:
            try:
                data = device.read(report_length)
                if data:
                    hex_data = ' '.join(f"{byte:02X}" for byte in data)
                    print(f"[{time.strftime('%H:%M:%S')}] 接收: {hex_data}")
                time.sleep(0.01)  # 适当延迟降低CPU占用
            except KeyboardInterrupt:
                print("\n检测到终止信号，正在退出...")
                running = False

    except hid.HIDException as e:
        print(f"HID协议错误: {str(e)}")
    except IOError as e:
        print(f"IO错误: {str(e)} → 可能原因: 设备未连接或权限不足")
    finally:
        if 'device' in locals() and device:
            device.close()
            print("设备连接已释放")

if __name__ == "__main__":
    HID_VID = 0x0483  
    HID_PID = 0x5750  
    hid_loop_receive(HID_VID, HID_PID)
