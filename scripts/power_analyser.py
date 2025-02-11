import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import numpy as np
import random
import threading
import time

# 窗口和图表配置
WINDOW_WIDTH = 800
WINDOW_HEIGHT = 400
DATA_POINTS = 100  # 最多显示 100 个数据点
UPDATE_INTERVAL = 100  # 更新间隔，单位：毫秒

class RealTimeLineChart:
    def __init__(self, root):
        self.root = root
        self.root.title("Real-time Line Chart")
        self.root.geometry(f"{WINDOW_WIDTH}x{WINDOW_HEIGHT}")

        # 初始化数据
        self.data = np.zeros(DATA_POINTS)

        # 创建 Matplotlib 图表
        self.fig = Figure(figsize=(8, 4), dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_ylabel("electric/uA")
        self.ax.set_xlabel("time/100ms")
        self.ax.set_ylim(0, 100)  # Y 轴范围
        self.ax.set_xlim(0, DATA_POINTS)  # X 轴范围
        self.line, = self.ax.plot(self.data, color="r")

        # 将图表嵌入到 Tkinter 窗口中
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        # 启动数据更新线程
        self.update_thread = threading.Thread(target=self.update_data)
        self.update_thread.daemon = True
        self.update_thread.start()

    def update_data(self):
        """更新数据并刷新图表"""
        while True:
            # 生成随机数据
            new_value = random.uniform(0, 100)
            self.data = np.roll(self.data, -1)  # 将数据向左滚动
            self.data[-1] = new_value  # 添加新数据

            # 更新折线图
            self.line.set_ydata(self.data)
            self.ax.relim()
            self.ax.autoscale_view()
            self.canvas.draw()

            # 等待下一次更新
            time.sleep(UPDATE_INTERVAL / 1000)

if __name__ == "__main__":
    # 创建 Tkinter 窗口
    root = tk.Tk()
    app = RealTimeLineChart(root)

    # 运行 Tkinter 主循环
    root.mainloop()