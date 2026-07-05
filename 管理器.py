#!/usr/bin/env python3
import os
import shutil
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
import subprocess

class LinuxFileManager:
    def __init__(self, root_window):
        self.root = root_window
        self.root.title("Linux 文件管理器 (模拟Windows窗口)")
        self.root.geometry("900x600")
        self.current_path = "/"

        # 顶部路径栏
        top_frame = tk.Frame(self.root)
        top_frame.pack(fill=tk.X, padx=5, pady=5)
        tk.Button(top_frame, text="返回上级", command=self.go_parent).pack(side=tk.LEFT)
        self.path_var = tk.StringVar(value=self.current_path)
        path_entry = tk.Entry(top_frame, textvariable=self.path_var, width=80)
        path_entry.pack(side=tk.LEFT, padx=5)
        tk.Button(top_frame, text="跳转", command=self.refresh_dir).pack(side=tk.LEFT)

        # 主列表框（显示文件文件夹）
        self.tree = ttk.Treeview(self.root, columns=("type", "size"), show="tree headings")
        self.tree.heading("#0", text="名称")
        self.tree.heading("type", text="类型")
        self.tree.heading("size", text="大小")
        self.tree.column("type", width=100)
        self.tree.column("size", width=100)
        self.tree.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # 右键菜单
        self.file_menu = tk.Menu(self.root, tearoff=0)
        self.file_menu.add_command(label="编辑文件", command=self.open_edit)
        self.file_menu.add_command(label="删除", command=self.delete_item)

        self.empty_menu = tk.Menu(self.root, tearoff=0)
        self.empty_menu.add_command(label="新建文件夹", command=self.new_folder)
        self.empty_menu.add_command(label="新建py脚本", command=self.new_py_file)

        # 绑定事件
        self.tree.bind("<Double-1>", self.double_click_item)
        self.tree.bind("<Button-3>", self.right_click)

        self.refresh_dir()

    # 刷新当前目录列表
    def refresh_dir(self):
        path = self.path_var.get()
        if not os.path.isdir(path):
            messagebox.showerror("错误", "目录不存在！")
            return
        self.current_path = path
        self.tree.delete(*self.tree.get_children())
        try:
            items = os.listdir(self.current_path)
        except PermissionError:
            messagebox.showerror("权限不足", "无法读取该目录，权限受限！")
            return
        for name in sorted(items):
            full_path = os.path.join(self.current_path, name)
            if os.path.isdir(full_path):
                f_type = "文件夹"
                size = "-"
            else:
                f_type = "文件"
                size = str(os.path.getsize(full_path)) + " 字节"
            self.tree.insert("", tk.END, iid=name, text=name, values=(f_type, size))

    # 返回上级目录
    def go_parent(self):
        if self.current_path == "/":
            return
        parent = os.path.dirname(self.current_path.rstrip("/"))
        self.path_var.set(parent)
        self.refresh_dir()

    # 双击进入文件夹
    def double_click_item(self, event):
        sel = self.tree.focus()
        if not sel:
            return
        full_path = os.path.join(self.current_path, sel)
        if os.path.isdir(full_path):
            self.path_var.set(full_path)
            self.refresh_dir()

    # 右键菜单判断
    def right_click(self, event):
        sel = self.tree.focus()
        item = self.tree.identify_row(event.y)
        self.tree.selection_set(item)
        if item:
            self.file_menu.tk_popup(event.x_root, event.y_root)
        else:
            self.empty_menu.tk_popup(event.x_root, event.y_root)

    # 新建文件夹
    def new_folder(self):
        name = simpledialog.askstring("新建文件夹", "输入文件夹名称：")
        if not name:
            return
        new_path = os.path.join(self.current_path, name)
        try:
            os.mkdir(new_path)
            self.refresh_dir()
        except Exception as e:
            messagebox.showerror("失败", str(e))

    # 新建py文件
    def new_py_file(self):
        name = simpledialog.askstring("新建py文件", "输入文件名（无需.py）：")
        if not name:
            return
        full_name = name + ".py"
        file_path = os.path.join(self.current_path, full_name)
        try:
            with open(file_path, "w", encoding="utf-8") as f:
                f.write("#!/usr/bin/env python3\n\nprint(\"新脚本\")")
            self.refresh_dir()
        except Exception as e:
            messagebox.showerror("失败", str(e))

    # 用文本编辑器打开文件编辑
    def open_edit(self):
        sel = self.tree.focus()
        full_path = os.path.join(self.current_path, sel)
        if os.path.isdir(full_path):
            messagebox.showinfo("提示", "文件夹无法编辑！")
            return
        # Kali 默认文本编辑器 mousepad
        subprocess.Popen(["mousepad", full_path])

    # 删除文件/文件夹
    def delete_item(self):
        sel = self.tree.focus()
        full_path = os.path.join(self.current_path, sel)
        confirm = messagebox.askyesno("确认删除", f"确定删除 {sel}？删除后无法恢复！")
        if not confirm:
            return
        try:
            if os.path.isdir(full_path):
                shutil.rmtree(full_path)
            else:
                os.remove(full_path)
            self.refresh_dir()
        except Exception as e:
            messagebox.showerror("删除失败", str(e))

if __name__ == "__main__":
    win = tk.Tk()
    app = LinuxFileManager(win)
    win.mainloop()
