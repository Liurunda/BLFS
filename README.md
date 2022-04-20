# BLFS

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
