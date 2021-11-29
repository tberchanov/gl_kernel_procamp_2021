#define pr_fmt(fmt) "%s: " fmt,  KBUILD_MODNAME

#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/kobject.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/list.h>
#include <linux/slab.h>

struct node {
	char *value;
	struct list_head list_member;
};

static LIST_HEAD(nodeHead);

static void add_node(char *value, struct list_head *head)
{
	pr_debug("---> %s", __func__);
	struct node *nodePtr = kmalloc(sizeof(struct node), GFP_KERNEL);

	if (nodePtr == NULL)
		pr_err("Node creation error");
	nodePtr->value = value;
	INIT_LIST_HEAD(&nodePtr->list_member);
	list_add(&(nodePtr->list_member), head);
	pr_debug("---< %s", __func__);
}

static ssize_t list_show(struct kobject *kobj, struct kobj_attribute *attr,
	char *buf)
{
	pr_debug("---> %s", __func__);
	struct list_head *iter;
	struct node *nodePtr;

	int bufSize = 0;

	list_for_each(iter, &nodeHead) {
		nodePtr = list_entry(iter, struct node, list_member);
		strcpy(buf + bufSize, nodePtr->value);
		bufSize += strlen(nodePtr->value);
		pr_debug("Node copy to buf: %s", nodePtr->value);
	}
	pr_debug("---< %s", __func__);
	return bufSize;
}

static ssize_t list_store(struct kobject *kobj, struct kobj_attribute *attr,
	const char *buf, size_t count)
{
	pr_debug("---> %s size_t: %ld", __func__, count);
	char *value = kcalloc(count, sizeof(char), GFP_KERNEL);

	memcpy(value, buf, count);
	pr_debug("value created: %ld", strlen(value));
	add_node(value, &nodeHead);
	pr_debug("---< %s", __func__);
	return count;
}

static struct kobj_attribute list_attribute =
	__ATTR(list, 0664, list_show, list_store);

static struct kobject *my_kobj;

static int hello_init(void)
{
	pr_debug("---> %s", __func__);
	int res = 0;

	my_kobj = kobject_create_and_add("MyObject", kernel_kobj);
	if (!my_kobj) {
		pr_err("Object was not created");
		return -ENOMEM;
	}
	res = sysfs_create_file(my_kobj, &list_attribute.attr);
	if (res) {
		pr_err("Attribute for Object was not created");
		kobject_put(my_kobj);
	}
	pr_debug("---< %s", __func__);
	return res;
}

static void hello_exit(void)
{
	pr_debug("---> %s", __func__);
	struct node *nodePtr;
	struct node *tmp;

	list_for_each_entry_safe(nodePtr, tmp, &nodeHead, list_member) {
		pr_debug("Clean node: %s", nodePtr->value);
		kfree(nodePtr->value);
		list_del(&(nodePtr->list_member));
		kfree(nodePtr);
	}
	pr_debug("List_empty: %d", list_empty(&nodeHead));
	kobject_put(my_kobj);

	pr_debug("---< %s", __func__);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Anatolii Berchanov <tberchanov@gmail.com>");
MODULE_DESCRIPTION("Basic data structures module");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
