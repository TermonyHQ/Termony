# Termony

[GitHub](https://github.com/jiegec/Termony) [Gitee](https://gitee.com/jiegec/Termony)

Termux for HarmonyOS Computer。开发进行中。

目前可在华为 MateBook Pro 上运行一些基本命令：

![](./screenshot.jpg)

还可以在 HarmonyOS 计算机上编译和运行 C/C++ 程序：

![](./screenshot_gcc.jpg)

## 概述

内置软件包：

||||||
| --- | --- | --- | --- | --- |
| aria2 | bash | binutils | busybox | c-ares |
| coremark | curl | elf-loader | expat | fastfetch |
| fish | gcc | gdb | gettext | git |
| glib | gmp | hdc | htop | kbd |
|lib{archive|event|ffi|idn2|unistring}|
| lz4 | make | mpc | mpfr | ncnn |
|ncurses|openssh|openssl|pcre2|proot|
|python|qemu|qemu-vroot|readline|sl|
|strace|stream|talloc|tar|tmux|
|tree|vim|vkpeak|xxhash|xz|
|yyjson|zstd|

- [elf-loader](https://github.com/MikhailProg/elf): (你可以运行没有可执行权限的可执行文件！例如 `cp /data/app/bin/bash ~/ && loader ~/bash`)
- qemu{,vroot}: (你可以运行没有可执行权限的可执行文件！例如 `cp /data/app/bin/bash ~/ && qemu-aarch64 ~/bash`)
- qemu-system-{aarch64,x86_64}: 你可以借助 qemu-system 来运行虚拟机（支持 x86_64 和 aarch64）

小技巧：你可以在内置终端应用中使用这些工具 `/data/service/hnp`：

![](./screenshot_hishell.jpg)

由于 PREFIX 被设置为 `/data/app/base.org/base_1.0`（感谢 @duskmoon314），某些路径可能会出错。你可以像这样覆盖它们：

```shell
LD_LIBRARY_PATH=/data/service/hnp/base.org/base_1.0/lib TERMINFO=/data/service/hnp/base.org/base_1.0/share/terminfo fish
```

你可以将它们持久化到 `~/.bashrc`，并在运行其他由 Termony 提供的命令之前运行 bash：

```shell
if [ -d "/data/service/hnp/base.org/base_1.0" ]; then
  export LD_LIBRARY_PATH=/data/service/hnp/base.org/base_1.0/lib
  export TERMINFO=/data/service/hnp/base.org/base_1.0/share/terminfo
  export VIM=/data/service/hnp/base.org/base_1.0/share/vim
  export TMUX_TMPDIR=/data/storage/el2/base/cache
fi
```

HiShell 和其他 debug 模式的 app 也可以使用 elf 加载器，尽管这可能因系统更新而改变。
如果你升级了 Termony，公共 HNP 安装路径 `/data/service/hnp` 不会更新。你需要重新安装 Termony 才能获得最新版本。

终端特性：

- 基本的转义序列支持
- 通过右键菜单粘贴
- 在命令行中通过 pbcopy/pbpaste 复制/粘贴（基于 OSC52 转义序列）

### 在新的根文件系统中运行

`qemu-vroot-aarch64` 是一个用户模式 qemu，修改后可以模拟 proot 行为。它允许用户运行 Linux 二进制文件（甚至是另一种 CPU 架构的），并像 chroot 或 proot 一样切换到新的根文件系统。

#### Alpine Linux

例如，你可以通过以下步骤运行 Alpine 根文件系统：

- 从 [qemu_rootfs](https://gitee.com/nanqu_ait/termony-hnp/releases/tag/qemu_rootfs) 下载已经确认可以使用的最小根文件系统（aarch64 或 x64）
- 解压下载的 rootfs tar.gz 文件，为了更好的兼容性，建议使用 `/data/storage/el2/base/files/alpine_rootfs`
```shell
mkdir -p /data/storage/el2/base/files/alpine_rootfs
tar xvf alpine-minirootfs-3.22.0-aarch64.tar.gz -C /data/storage/el2/base/files/alpine_rootfs
```
- 运行 `qemu-vroot-aarch64` 加载 busybox shell 并设置根文件系统和环境变量（对于 x86_64 上的根文件系统，请使用 `qemu-vroot-x86_64`）
```shell
cd /data/storage/el2/base/files/alpine_rootfs
qemu-vroot-aarch64 -E PATH=/bin:/usr/bin:/sbin -E HOME=/root -L ./ ./bin/busybox sh -c 'cd && sh'
```
- 运行 `ls /`，根目录已经改变！
```shell
ls /
bin    dev    etc    home   lib    media  mnt    opt    proc   root   run    sbin   srv    sys    tmp    usr    var
```
- 运行 `apk update`，Alpine 包管理器运行良好，你可以通过 `apk` 安装包

#### Ubuntu

你也可以按下列步骤使用 Ubuntu 根文件系统：

- 下载 ubuntu base 根文件系统，地址：[Ubuntu Base 24.04](https://cdimage.ubuntu.com/ubuntu-base/releases/24.04/release/) (选择文件：ubuntu-base-24.04.3-base-arm64.tar.gz)
- 解压下载后的 tar.gz 文件, 考虑到兼容性, 推荐解压到`/data/storage/el2/base/files/ubuntu_rootfs`
```shell
mkdir -p /data/storage/el2/base/files/ubuntu_rootfs
tar xvf ubuntu-base-24.04.3-base-arm64.tar.gz -C /data/storage/el2/base/files/ubuntu_rootfs
```
- 将 `APT::Sandbox::User "root";` 添加到 Ubuntu 根文件系统中的 `/etc/apt/apt.conf.d/01-vendor-ubuntu` 文件中
```shell
cd /data/storage/el2/base/files/ubuntu_rootfs
echo 'APT::Sandbox::User "root";' > etc/apt/apt.conf.d/01-vendor-ubuntu
```
- 使用 `qemu-vroot-aarch64` 运行根文件系统中的 bash
```shell
cd /data/storage/el2/base/files/ubuntu_rootfs
qemu-vroot-aarch64 -E PATH=/bin:/usr/bin:/sbin -E HOME=/root -L ./ ./bin/bash -c 'cd && bash'
```
- 运行 `apt update`, 可以看到 `apt` 已经成功运行。你可以通过 `apt` 安装各种软件包

## 使用方法（如果你是 Mac 用户）：

1. 将你的 MateBook Pro 连接到 Mac，并在 Mac 上执行以下步骤
2. 递归克隆此仓库，并进入该仓库目录
3. 在 DevEco-Studio 中设置代码签名，忽略任何警告
4. 从 Homebrew 或 Nix 安装 `wget`、`coreutils`、`make`、`gsed`、`gettext`、`automake`、`cmake`、`pkg-config` 和 `ncurses`
5. （M 系列用户）`export PATH="/opt/homebrew/opt/coreutils/libexec/gnubin:/opt/homebrew/opt/gnu-sed/libexec/gnubin:/opt/homebrew/opt/make/libexec/gnubin:$PATH"`
6. 运行 `./create-hnp.sh` 创建 hnp 包
7. 运行 `./build-macos.sh`
8. 运行 `./push.sh ./entry/build/default/outputs/default/entry-default-signed.hap`
9. 在你的 HarmonyOS 计算机上尝试 Termony

## 使用方法（如果你是 Linux 用户）：

1. 将你的 MateBook Pro 连接到 Linux 机器，并执行以下步骤
2. 递归克隆此仓库，并进入该仓库目录
3. 在 DevEco-Studio 中设置代码签名，忽略任何警告
4. 设置 DevEco 命令行工具，并确保 `$TOOL_HOME` 环境变量是 SDK 的正确目录
5. 运行 `./build-linux.sh -b` 创建 hnp 包
6. 运行 `./build-linux.sh -s` 对 hap 文件签名
7. 运行 `./build-linux.sh -p` 推送并安装 Termony 到你的设备
8. 在你的 HarmonyOS 计算机上尝试 Termony

## 使用方法（如果你是 Windows 用户）：

1. 将你的 MateBook Pro 连接到 Windows 机器，并执行以下步骤
2. 安装 WSL（推荐 Ubuntu），并按照以下步骤构建 hap
3. 递归克隆此仓库，并进入该仓库目录
4. 在 DevEco-Studio 中设置代码签名，忽略任何警告
5. 在 WSL 中设置 DevEco 命令行工具，并确保 `$TOOL_HOME` 环境变量是 SDK 的正确目录
6. 在 WSL 中运行 `./build-linux.sh -b` 创建 hnp 包
7. 在 WSL 中运行 `./build-linux.sh -s` 对 hap 文件签名
8. 在 Windows 终端中使用 `hdc` 在 Windows 上发送和安装 hap 文件，如 `build-linux.sh` 中的 `hdc_push`
9. 在你的 HarmonyOS 计算机上尝试 Termony

## 它是如何工作的

通过检查 CodeArts IDE，我们发现它使用 HNP 包来运行本地程序。你需要将 `.hnp` 文件打包到 `.hap` 中，并像下面这样添加到 `module.json5`：

```json5
{
  "module": {
    "hnpPackages": [
      {
        "package": "busybox.hnp",
        "type": "private"
      }
    ]
  }
}
```

然后，你需要将 `.hnp` 文件添加到 `.hap` 并手动签名 `.hap`。你可以参考 `sign.py` 看看它是如何完成的。`.hnp` 包会自动解压到 `/data/app` 下，并在 `/data/app/bin` 下创建符号链接。
