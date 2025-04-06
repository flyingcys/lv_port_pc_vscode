#include <lvgl/lvgl.h>
#include <time.h>

static lv_obj_t * hour_tens;
static lv_obj_t * hour_units;
static lv_obj_t * min_tens;
static lv_obj_t * min_units;
static lv_obj_t * sec_tens;
static lv_obj_t * sec_units;

LV_IMG_DECLARE(num_0);
LV_IMG_DECLARE(num_1);
LV_IMG_DECLARE(num_2);
LV_IMG_DECLARE(num_3);
LV_IMG_DECLARE(num_4);
LV_IMG_DECLARE(num_5);
LV_IMG_DECLARE(num_6);
LV_IMG_DECLARE(num_7);
LV_IMG_DECLARE(num_8);
LV_IMG_DECLARE(num_9);

static const void *num_imgs[] = {
    &num_0, &num_1, &num_2, &num_3, &num_4, 
    &num_5, &num_6, &num_7, &num_8, &num_9
};

static void update_digit(lv_obj_t * digit_obj, int new_val)
{
    static int old_vals[6] = {0}; // 保存旧值
    int index = -1;
    
    // 找出是哪个数字对象
    if(digit_obj == hour_tens) index = 0;
    else if(digit_obj == hour_units) index = 1;
    else if(digit_obj == min_tens) index = 2;
    else if(digit_obj == min_units) index = 3;
    else if(digit_obj == sec_tens) index = 4;
    else if(digit_obj == sec_units) index = 5;
    
    if(index == -1 || old_vals[index] == new_val) return;
    
    // 创建新图像对象用于动画
    lv_obj_t * new_img = lv_img_create(lv_scr_act());
    lv_img_set_src(new_img, num_imgs[new_val]);
    lv_obj_align(new_img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(new_img, lv_obj_get_x(digit_obj), lv_obj_get_y(digit_obj) + 50); // 初始位置在下方
    
    // 设置旧图像的动画 - 向上移动并淡出
    lv_anim_t a_old;
    lv_anim_init(&a_old);
    lv_anim_set_var(&a_old, digit_obj);
    lv_anim_set_values(&a_old, lv_obj_get_y(digit_obj), lv_obj_get_y(digit_obj) - 50);
    lv_anim_set_exec_cb(&a_old, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_time(&a_old, 300);
    lv_anim_set_ready_cb(&a_old, lv_obj_delete_anim_completed_cb);
    lv_anim_start(&a_old);
    
    // 设置新图像的动画 - 从下方移动到中心
    lv_anim_t a_new;
    lv_anim_init(&a_new);
    lv_anim_set_var(&a_new, new_img);
    lv_anim_set_values(&a_new, lv_obj_get_y(new_img), lv_obj_get_y(new_img) - 50);
    lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_time(&a_new, 300);
    lv_anim_start(&a_new);
    
    // 更新引用
    if(digit_obj == hour_tens) hour_tens = new_img;
    else if(digit_obj == hour_units) hour_units = new_img;
    else if(digit_obj == min_tens) min_tens = new_img;
    else if(digit_obj == min_units) min_units = new_img;
    else if(digit_obj == sec_tens) sec_tens = new_img;
    else if(digit_obj == sec_units) sec_units = new_img;
    
    old_vals[index] = new_val;
}

static void clock_update(lv_timer_t * timer)
{
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);

    int hour = lt->tm_hour;
    int min = lt->tm_min;
    int sec = lt->tm_sec;

    // 更新小时
    // update_digit(hour_tens, hour / 10);
    // update_digit(hour_units, hour % 10);
    
    // // 更新分钟
    // update_digit(min_tens, min / 10);
    // update_digit(min_units, min % 10);
    
    // // 更新秒钟
    // update_digit(sec_tens, sec / 10);
    // update_digit(sec_units, sec % 10);
    lv_img_set_src(hour_tens, num_imgs[lt->tm_hour / 10]);
    lv_img_set_src(hour_units, num_imgs[lt->tm_hour % 10]);
    lv_img_set_src(min_tens, num_imgs[lt->tm_min / 10]);
    lv_img_set_src(min_units, num_imgs[lt->tm_min % 10]);
    lv_img_set_src(sec_tens, num_imgs[lt->tm_sec / 10]);
    lv_img_set_src(sec_units, num_imgs[lt->tm_sec % 10]);
}

void create_clock(void *parent)
{
    hour_tens = lv_img_create(parent);
    hour_units = lv_img_create(parent);
    lv_obj_t * colon1 = lv_img_create(lv_scr_act());
    min_tens = lv_img_create(parent);
    min_units = lv_img_create(parent);
    lv_obj_t * colon2 = lv_img_create(lv_scr_act());
    sec_tens = lv_img_create(parent);
    sec_units = lv_img_create(parent);

    lv_img_set_src(hour_tens, num_imgs[0]);
    lv_img_set_src(hour_units, num_imgs[0]);
    lv_img_set_src(min_tens, num_imgs[0]);
    lv_img_set_src(min_units, num_imgs[0]);
    lv_img_set_src(sec_tens, num_imgs[0]);
    lv_img_set_src(sec_units, num_imgs[0]);
    

    lv_obj_align(hour_tens, LV_ALIGN_CENTER, -120, 0);
    lv_obj_align(hour_units, LV_ALIGN_CENTER, -80, 0);
    lv_obj_align(colon1, LV_ALIGN_CENTER, -40, 0);
    lv_obj_align(min_tens, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(min_units, LV_ALIGN_CENTER, 40, 0);
    lv_obj_align(colon2, LV_ALIGN_CENTER, 80, 0);
    lv_obj_align(sec_tens, LV_ALIGN_CENTER, 120, 0);
    lv_obj_align(sec_units, LV_ALIGN_CENTER, 160, 0);

    lv_timer_create(clock_update, 1000, NULL);
}