int32_t file_open(uint32_t inode_no, uint8_t flag) {
   int fd_idx = get_free_slot_in_global();
   if (fd_idx == -1) {
      printk("exceed max open files\n");
      return -1;
   }
   file_table[fd_idx].fd_inode = inode_open(cur_part, inode_no);
   file_table[fd_idx].fd_pos = 0;	     // 每次打开文件,要将fd_pos还原为0,即让文件内的指针指向开头
   file_table[fd_idx].fd_flag = flag;
   bool* write_deny = &file_table[fd_idx].fd_inode->write_deny; 
   if (flag == O_WRONLY || flag == O_RDWR) {	// 只要是关于写文件,判断是否有其它进程正写此文件
						// 若是读文件,不考虑write_deny
   /* 以下进入临界区前先关中断 */
      enum intr_status old_status = intr_disable();
      if (!(*write_deny)) {    // 若当前没有其它进程写该文件,将其占用.
	 *write_deny = true;   // 置为true,避免多个进程同时写此文件
	 intr_set_status(old_status);	  // 恢复中断
      } else {		// 直接失败返回
	 intr_set_status(old_status);
	 printk("file can`t be write now, try again later\n");
	 return -1;
      }
   }  // 若是读文件或创建文件,不用理会write_deny,保持默认
   return pcb_fd_install(fd_idx);
}
/* 关闭文件 */
int32_t file_close(struct file* file) {
   if (file == NULL) {
      return -1;
   }
   file->fd_inode->write_deny = false;
   inode_close(file->fd_inode);
   file->fd_inode = NULL;   // 使文件结构可用
   return 0;
}
/* 将文件描述符转化为文件表的下标 */
static uint32_t fd_local2global(uint32_t local_fd) {
   struct task_struct* cur = running_thread();
   int32_t global_fd = cur->fd_table[local_fd];  
   ASSERT(global_fd >= 0 && global_fd < MAX_FILE_OPEN);
   return (uint32_t)global_fd;
} 
/* 关闭文件描述符fd指向的文件,成功返回0,否则返回-1 */
int32_t sys_close(int32_t fd) {
   int32_t ret = -1;   // 返回值默认为-1,即失败
   if (fd > 2) {
      uint32_t _fd = fd_local2global(fd);
      ret = file_close(&file_table[_fd]);
      running_thread()->fd_table[fd] = -1; // 使该文件描述符位可用
   }
   return ret;
}
