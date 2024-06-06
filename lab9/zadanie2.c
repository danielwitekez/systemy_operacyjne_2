#include<linux/module.h>
#include<linux/genhd.h>
#include<linux/vmalloc.h>
#include<linux/fs.h>
#include<linux/bio.h>
#include<linux/blkdev.h>

#define DEVICE_SIZE 4*1024*1024

static int sector_size = 512;
static int major = 0;
static struct sbd_struct
{
	struct gendisk *gd1, *gd2;
	void *memory;
} sbd_dev;

static inline int transfer_single_bio(struct bio *bio)
{
        struct bvec_iter iter;
        struct bio_vec vector;
        sector_t sector = bio->bi_iter.bi_sector;
	bool wirte = bio_data_dir(bio) == WRITE;
        
	bio_for_each_segment(vector,bio,iter) {
                unsigned int len = vector.bv_len;
                void *addr = kmap(vector.bv_page);
		if(wirte) 
                	memcpy(sbd_dev.memory+sector*sector_size,addr+vector.bv_offset,len);
		else 
			memcpy(addr+vector.bv_offset,sbd_dev.memory+sector*sector_size,len);
                kunmap(addr);
                sector += len >> 9;
        }
        return 0;
}

static blk_qc_t make_request(struct request_queue *q, struct bio *bio)
{
        int result=0;

        if(bio_end_sector(bio)>get_capacity(bio->bi_bdev->bd_disk)) 
		goto mrerr0;
       
	result = transfer_single_bio(bio);
	if(unlikely(result!=0))
		goto mrerr0;

        bio_endio(bio);
        return BLK_QC_T_NONE;
mrerr0:
	bio_io_error(bio);
	return BLK_QC_T_NONE;
}

static struct block_device_operations block_methods = {
	.owner = THIS_MODULE
};


static int __init sbd_constructor(void)
{
	sbd_dev.memory = vmalloc(DEVICE_SIZE);
	if(!sbd_dev.memory) {
		pr_alert("Memory allocation error!\n");
		goto ier1;
	}
	sbd_dev.gd1 = alloc_disk(1);
    sbd_dev.gd2 = alloc_disk(2);
	if(!sbd_dev.gd1) {
		pr_alert("General disk structure allocation error!\n");
		goto ier2;
	}
    if(!sbd_dev.gd2) {
		pr_alert("General disk structure allocation error!\n");
		goto ier2;
	}
	major = register_blkdev(major,"sbd");
	if(major<=0) {
		pr_alert("Major number allocation error!\n");	
		goto ier3;
	}
	pr_info("[sbd] Major number allocated: %d.\n",major);

    // gen_disk 1
	sbd_dev.gd1->major = major;
	sbd_dev.gd1->first_minor = 0;
	sbd_dev.gd1->minors = 2;
	sbd_dev.gd1->fops = &block_methods;
	sbd_dev.gd1->private_data = NULL;
	strcpy(sbd_dev.gd1->disk_name,"sbd_a");
	set_capacity(sbd_dev.gd1,(DEVICE_SIZE)>>9);
	sbd_dev.gd1->queue = blk_alloc_queue(GFP_KERNEL);
	if(!sbd_dev.gd1->queue) {
		pr_alert("Request queue allocation error!\n");
		goto ier4;
	}
	blk_queue_make_request(sbd_dev.gd1->queue,make_request);
	pr_info("[sbd] Gendisk initialized.\n");
	add_disk(sbd_dev.gd1);

    // gen_disk 2
    sbd_dev.gd2->major = major;
	sbd_dev.gd2->first_minor = 1;
	sbd_dev.gd2->minors = 2;
	sbd_dev.gd2->fops = &block_methods;
	sbd_dev.gd2->private_data = NULL;
	strcpy(sbd_dev.gd2->disk_name,"sbd_b");
	set_capacity(sbd_dev.gd2,(DEVICE_SIZE)>>9);
	sbd_dev.gd2->queue = blk_alloc_queue(GFP_KERNEL);
	if(!sbd_dev.gd2->queue) {
		pr_alert("Request queue allocation error!\n");
		goto ier4;
	}
	blk_queue_make_request(sbd_dev.gd2->queue,make_request);
	pr_info("[sbd] Gendisk initialized.\n");
	add_disk(sbd_dev.gd2);

	return 0;

ier4:
	unregister_blkdev(major,"sbd");
ier3:
	put_disk(sbd_dev.gd1);
    put_disk(sbd_dev.gd2);
ier2:
	vfree(sbd_dev.memory);
ier1: 
	return -ENOMEM;
}

static void __exit sbd_desctructor(void)
{
	del_gendisk(sbd_dev.gd1);
	blk_cleanup_queue(sbd_dev.gd1->queue);

    del_gendisk(sbd_dev.gd2);
	blk_cleanup_queue(sbd_dev.gd2->queue);

	unregister_blkdev(major,"sbd");
	put_disk(sbd_dev.gd1);
    put_disk(sbd_dev.gd2);
	vfree(sbd_dev.memory);
}

module_init(sbd_constructor);
module_exit(sbd_desctructor);

MODULE_LICENSE("GPL");