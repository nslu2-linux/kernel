--- linux-2.6.12.3/fs/dcache.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/fs/dcache.c	2007-02-17 14:10:28.000000000 -0500
@@ -1443,6 +1443,54 @@
 	return res;
 }
 
+// 2005.10.27, Johnson Cheng Add for QRAID1 module
+// full_d_path is used to find the absolute-path name (included ch-rooted
+// application's) for the given dentry resides in the vfsmount.
+//
+// returns the pointer to the absolute-path name
+char* full_d_path(struct dentry* d, struct vfsmount* mnt, char* buf, int len) {
+
+	struct task_struct *init = current;
+	struct vfsmount *init_root_mnt, *curr_root_mnt;
+	struct dentry *init_root, *curr_root;
+	char tmp[2048];
+	char *fp;
+
+	// find the init process first... (needed for chrooted applications)
+	//while(init->pid != 1) init = init->p_pptr; 
+	
+	// get a handle on absolute root
+	read_lock(&init->fs->lock);
+	init_root_mnt = mntget(init->fs->rootmnt);
+	init_root = dget(init->fs->root);
+	read_unlock(&init->fs->lock);
+
+	// get a handle on the current root
+	read_lock(&current->fs->lock);
+	curr_root_mnt = mntget(current->fs->rootmnt);
+	curr_root = dget(current->fs->root);
+	read_unlock(&init->fs->lock);
+
+	// find the root path for the current process with respect to absolute root
+	spin_lock(&dcache_lock);
+	fp = __d_path(curr_root, curr_root_mnt, init_root, init_root_mnt, buf, len);
+	spin_unlock(&dcache_lock);
+
+	// done with the init_root and curr_root
+	dput(init_root);        dput(curr_root);
+	mntput(init_root_mnt);  mntput(curr_root_mnt);	
+	strcpy(tmp, fp); // tmp holds relative root for the current process
+
+	// find the traiiling part
+	fp = d_path(d,mnt,buf,len);
+		
+	// concatenate, and return
+	sprintf(buf, "%s%s", strlen(tmp)==1?"":tmp, fp);
+	return buf;
+}
+// 2005.10.27, Johnson Cheng End
+
+
 /*
  * NOTE! The user-level library version returns a
  * character pointer. The kernel system call just
