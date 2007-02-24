--- linux-2.6.12.3/fs/namei.c	2005-07-15 17:18:57.000000000 -0400
+++ linux-2.6.12.3-qnap/fs/namei.c	2007-02-17 14:10:28.000000000 -0500
@@ -1834,6 +1834,47 @@
 	return error;
 }
 
+asmlinkage long sys_qnap_rmdir(const char __user * pathname)
+{
+        int error = 0;
+        char * name;
+        struct dentry *dentry;
+        struct nameidata nd;
+
+        name = getname(pathname);
+        if(IS_ERR(name))
+                return PTR_ERR(name);
+
+        error = path_lookup(name, LOOKUP_PARENT, &nd);
+        if (error)
+                goto exit;
+
+        switch(nd.last_type) {
+                case LAST_DOTDOT:
+                        error = -ENOTEMPTY;
+                        goto exit1;
+                case LAST_DOT:
+                        error = -EINVAL;
+                        goto exit1;
+                case LAST_ROOT:
+                        error = -EBUSY;
+                        goto exit1;
+        }
+        down(&nd.dentry->d_inode->i_sem);
+        dentry = lookup_hash(&nd.last, nd.dentry);
+        error = PTR_ERR(dentry);
+        if (!IS_ERR(dentry)) {
+                error = vfs_rmdir(nd.dentry->d_inode, dentry);
+                dput(dentry);
+        }
+        up(&nd.dentry->d_inode->i_sem);
+exit1:
+        path_release(&nd);
+exit:
+        putname(name);
+        return error;
+}
+
 int vfs_unlink(struct inode *dir, struct dentry *dentry)
 {
 	int error = may_delete(dir, dentry, 0);
