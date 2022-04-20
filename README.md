# BLFS

希望模仿ext2/ext4设计文件系统

我们一共有四周时间，在第一周可以做什么：

1. 搭一个基础的框架，先实现“文件、目录的抽象”，和read/write/open/close之类的基本接口。（不考虑性能/崩溃一致性/持久性）。

2. 搞定文件系统的性能测试怎么做（比如在已有的基于fuse的文件系统上如何做测试）

3. 研究一下ext2对崩溃一致性/持久性的实现？

4. 从一开始就维护好文档，以方便最后交报告

implement what?

1. basic FS without persistency

   文件和目录的抽象，接口：read, write, open, close, mkdir, rename, fsync

   shell指令: ls, cp, mv, rm, mkdir, rmdir, cat
   
   KV Store 项目
   
   工业级程序
   
2. basic FS with persistency 

  元数据崩溃一致性 
  
  数据+元数据 崩溃一致性
  
  资源泄漏
  
  恢复
  
  报告说明：支持什么级别的崩溃一致性，任何一个时刻断电后，文件系统可能处在什么可能的状态，是否所有可能的状态都是合法的？
  
  durability：fsync/fdatasync成功后，数据/元数据不应该丢失
  
  并发：线程拓展性（多线程读写）
  
  冲突情况下的性能
  
  自己实现性能测试
  
  拓展：权限检查 加密 其他novelty
