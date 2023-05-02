/* 将硬盘分区part上的inode清空 */
void inode_delete(struct partition* part, uint32_t inode_no, void* io_buf) {
   ASSERT(inode_no < 4096);
   struct inode_position inode_pos;
   inode_locate(part, inode_no, &inode_pos);     // inode位置信息会存入inode_pos
   ASSERT(inode_pos.sec_lba <= (part->start_lba + part->sec_cnt));
   
   char* inode_buf = (char*)io_buf;
   if (inode_pos.two_sec) {   // inode跨扇区,读入2个扇区
      /* 将原硬盘上的内容先读出来 */
      ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
      /* 将inode_buf清0 */
      memset((inode_buf + inode_pos.off_size), 0, sizeof(struct inode));
      /* 用清0的内存数据覆盖磁盘 */
      ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
   } else {    // 未跨扇区,只读入1个扇区就好
      /* 将原硬盘上的内容先读出来 */
      ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
      /* 将inode_buf清0 */
      memset((inode_buf + inode_pos.off_size), 0, sizeof(struct inode));
      /* 用清0的内存数据覆盖磁盘 */
      ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
   }
}

/* 回收inode的数据块和inode本身 */
void inode_release(struct partition* part, uint32_t inode_no) {
   struct inode* inode_to_del = inode_open(part, inode_no);
   ASSERT(inode_to_del->i_no == inode_no);

/* 1 回收inode占用的所有块 */
   uint8_t block_idx = 0, block_cnt = 12;
   uint32_t block_bitmap_idx;
   uint32_t all_blocks[140] = {0};	  //12个直接块+128个间接块
 
   /* a 先将前12个直接块存入all_blocks */
   while (block_idx < 12) {
      all_blocks[block_idx] = inode_to_del->i_sectors[block_idx];
      block_idx++;
   }

   /* b 如果一级间接块表存在,将其128个间接块读到all_blocks[12~], 并释放一级间接块表所占的扇区 */
   if (inode_to_del->i_sectors[12] != 0) {
      ide_read(part->my_disk, inode_to_del->i_sectors[12], all_blocks + 12, 1);
      block_cnt = 140;

      /* 回收一级间接块表占用的扇区 */
      block_bitmap_idx = inode_to_del->i_sectors[12] - part->sb->data_start_lba;
      ASSERT(block_bitmap_idx > 0);
      bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
      bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
   }
   
   /* c inode所有的块地址已经收集到all_blocks中,下面逐个回收 */
   block_idx = 0;
   while (block_idx < block_cnt) {
      if (all_blocks[block_idx] != 0) {
	 block_bitmap_idx = 0;
	 block_bitmap_idx = all_blocks[block_idx] - part->sb->data_start_lba;
	 ASSERT(block_bitmap_idx > 0);
	 bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
	 bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
      }
      block_idx++; 
   }

/*2 回收该inode所占用的inode */
   bitmap_set(&part->inode_bitmap, inode_no, 0);  
   bitmap_sync(cur_part, inode_no, INODE_BITMAP);

   /******     以下inode_delete是调试用的    ******
   * 此函数会在inode_table中将此inode清0,
   * 但实际上是不需要的,inode分配是由inode位图控制的,
   * 硬盘上的数据不需要清0,可以直接覆盖*/
   void* io_buf = sys_malloc(1024);
   inode_delete(part, inode_no, io_buf);
   sys_free(io_buf);
   /***********************************************/
    
   inode_close(inode_to_del);
}

bool delete_dir_entry(struct partition* part, struct dir* pdir, uint32_t inode_no, void* io_buf) {
   struct inode* dir_inode = pdir->inode;
   uint32_t block_idx = 0, all_blocks[140] = {0};
   /* 收集目录全部块地址 */
   while (block_idx < 12) {
      all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
      block_idx++;
   }
   if (dir_inode->i_sectors[12]) {
      ide_read(part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1);
   }

   /* 目录项在存储时保证不会跨扇区 */
   uint32_t dir_entry_size = part->sb->dir_entry_size;
   uint32_t dir_entrys_per_sec = (SECTOR_SIZE / dir_entry_size);       // 每扇区最大的目录项数目
   struct dir_entry* dir_e = (struct dir_entry*)io_buf;   
   struct dir_entry* dir_entry_found = NULL;
   uint8_t dir_entry_idx, dir_entry_cnt;
   bool is_dir_first_block = false;     // 目录的第1个块 

   /* 遍历所有块,寻找目录项 */
   block_idx = 0;
   while (block_idx < 140) {
      is_dir_first_block = false;
      if (all_blocks[block_idx] == 0) {
	 block_idx++;
	 continue;
      }
      dir_entry_idx = dir_entry_cnt = 0;
      memset(io_buf, 0, SECTOR_SIZE);
      /* 读取扇区,获得目录项 */
      ide_read(part->my_disk, all_blocks[block_idx], io_buf, 1);

      /* 遍历所有的目录项,统计该扇区的目录项数量及是否有待删除的目录项 */
      while (dir_entry_idx < dir_entrys_per_sec) {
	 if ((dir_e + dir_entry_idx)->f_type != FT_UNKNOWN) {
	    if (!strcmp((dir_e + dir_entry_idx)->filename, ".")) { 
	       is_dir_first_block = true;
	    } else if (strcmp((dir_e + dir_entry_idx)->filename, ".") && 
	       strcmp((dir_e + dir_entry_idx)->filename, "..")) {
	       dir_entry_cnt++;     // 统计此扇区内的目录项个数,用来判断删除目录项后是否回收该扇区
	       if ((dir_e + dir_entry_idx)->i_no == inode_no) {	  // 如果找到此i结点,就将其记录在dir_entry_found
		  ASSERT(dir_entry_found == NULL);  // 确保目录中只有一个编号为inode_no的inode,找到一次后dir_entry_found就不再是NULL
		  dir_entry_found = dir_e + dir_entry_idx;
		  /* 找到后也继续遍历,统计总共的目录项数 */
	       }
	    }
	 }
	 dir_entry_idx++;
      } 

      /* 若此扇区未找到该目录项,继续在下个扇区中找 */
      if (dir_entry_found == NULL) {
	 block_idx++;
	 continue;
      }

   /* 在此扇区中找到目录项后,清除该目录项并判断是否回收扇区,随后退出循环直接返回 */
      ASSERT(dir_entry_cnt >= 1);
    /* 除目录第1个扇区外,若该扇区上只有该目录项自己,则将整个扇区回收 */
      if (dir_entry_cnt == 1 && !is_dir_first_block) {
	 /* a 在块位图中回收该块 */
	 uint32_t block_bitmap_idx = all_blocks[block_idx] - part->sb->data_start_lba;
	 bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
	 bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

	 /* b 将块地址从数组i_sectors或索引表中去掉 */
	 if (block_idx < 12) {
	    dir_inode->i_sectors[block_idx] = 0;
	 } else {    // 在一级间接索引表中擦除该间接块地址
	    /*先判断一级间接索引表中间接块的数量,如果仅有这1个间接块,连同间接索引表所在的块一同回收 */
	    uint32_t indirect_blocks = 0;
	    uint32_t indirect_block_idx = 12;
	    while (indirect_block_idx < 140) {
	       if (all_blocks[indirect_block_idx] != 0) {
		  indirect_blocks++;
	       }
	    }
	    ASSERT(indirect_blocks >= 1);  // 包括当前间接块

	    if (indirect_blocks > 1) {	  // 间接索引表中还包括其它间接块,仅在索引表中擦除当前这个间接块地址
	       all_blocks[block_idx] = 0; 
	       ide_write(part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1); 
	    } else {	// 间接索引表中就当前这1个间接块,直接把间接索引表所在的块回收,然后擦除间接索引表块地址
	       /* 回收间接索引表所在的块 */
	       block_bitmap_idx = dir_inode->i_sectors[12] - part->sb->data_start_lba;
	       bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
	       bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
	       
	       /* 将间接索引表地址清0 */
	       dir_inode->i_sectors[12] = 0;
	    }
	 }
      } else { // 仅将该目录项清空
	 memset(dir_entry_found, 0, dir_entry_size);
	 ide_write(part->my_disk, all_blocks[block_idx], io_buf, 1);
      }

   /* 更新i结点信息并同步到硬盘 */
      ASSERT(dir_inode->i_size >= dir_entry_size);
      dir_inode->i_size -= dir_entry_size;
      memset(io_buf, 0, SECTOR_SIZE * 2);
      inode_sync(part, dir_inode, io_buf);

      return true;
   }
   /* 所有块中未找到则返回false,若出现这种情况应该是serarch_file出错了 */
   return false;
}

/* 删除文件(非目录),成功返回0,失败返回-1 */
int32_t sys_unlink(const char* pathname) {
   ASSERT(strlen(pathname) < MAX_PATH_LEN);

   /* 先检查待删除的文件是否存在 */
   struct path_search_record searched_record;
   memset(&searched_record, 0, sizeof(struct path_search_record));
   int inode_no = search_file(pathname, &searched_record);
   ASSERT(inode_no != 0);
   if (inode_no == -1) {
      printk("file %s not found!\n", pathname);
      dir_close(searched_record.parent_dir);
      return -1;
   }
   if (searched_record.file_type == FT_DIRECTORY) {
      printk("can`t delete a direcotry with unlink(), use rmdir() to instead\n");
      dir_close(searched_record.parent_dir);
      return -1;
   }

   /* 检查是否在已打开文件列表(文件表)中 */
   uint32_t file_idx = 0;
   while (file_idx < MAX_FILE_OPEN) {
      if (file_table[file_idx].fd_inode != NULL && (uint32_t)inode_no == file_table[file_idx].fd_inode->i_no) {
	 break;
      }
      file_idx++;
   }
   if (file_idx < MAX_FILE_OPEN) {
      dir_close(searched_record.parent_dir);
      printk("file %s is in use, not allow to delete!\n", pathname);
      return -1;
   }
   ASSERT(file_idx == MAX_FILE_OPEN);
   
   /* 为delete_dir_entry申请缓冲区 */
   void* io_buf = sys_malloc(SECTOR_SIZE + SECTOR_SIZE);
   if (io_buf == NULL) {
      dir_close(searched_record.parent_dir);
      printk("sys_unlink: malloc for io_buf failed\n");
      return -1;
   }

   struct dir* parent_dir = searched_record.parent_dir;  
   delete_dir_entry(cur_part, parent_dir, inode_no, io_buf);
   inode_release(cur_part, inode_no);
   sys_free(io_buf);
   dir_close(searched_record.parent_dir);
   return 0;   // 成功删除文件 
}
