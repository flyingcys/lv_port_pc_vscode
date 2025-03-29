#include "lvgl.h"
#include "icon_replace.h"
#include <stdint.h>
#include <stdlib.h>

#define  icon_start_x   30  //起始x坐标
#define  icon_start_y   30//起始y坐标
#define  icon_count 13//图标个数
#define  icon_max_row  3//图标排数
#define  icon_max_col  5//图标列数
#define  icon_x_distance 90//图标横向间距
#define  icon_y_distance 100//图标纵向间距
#define  icon_size   60// 图标大小

LV_IMG_DECLARE(icon_0_0)
LV_IMG_DECLARE(icon_0_1)
LV_IMG_DECLARE(icon_0_2)
LV_IMG_DECLARE(icon_0_3)
LV_IMG_DECLARE(icon_0_4)
LV_IMG_DECLARE(icon_0_5)
LV_IMG_DECLARE(icon_0_6)
LV_IMG_DECLARE(icon_0_7)
LV_IMG_DECLARE(icon_0_8)
LV_IMG_DECLARE(icon_0_9)
LV_IMG_DECLARE(icon_0_10)
LV_IMG_DECLARE(icon_0_11)
LV_IMG_DECLARE(icon_0_12)

static const lv_img_dsc_t * icon_img[13] = { &icon_0_0 ,&icon_0_1 ,&icon_0_2,&icon_0_3,&icon_0_4,&icon_0_5,&icon_0_6,&icon_0_7,&icon_0_8,&icon_0_9,&icon_0_10,&icon_0_11,&icon_0_12,};


typedef struct
{
	lv_obj_t * icon;
	int index;

}icon_type;

icon_type icons[icon_count];
static int offsetx,offsety,tuoching,touch_time_count,old_index,new_index,icon_shake;
static lv_obj_t * screen1;
static int index_by_xy(lv_point_t * click_point);
static int x_by_index(int index);
static int y_by_index(int index);
static void win_set_h_cb(void * var, int v);
static void add_bar_btn_cb(lv_anim_t * a);
static void resume_win_cb(lv_event_t * e);
static void released_cb(lv_event_t * e);
static void touching_cb(lv_event_t * e);
static void set_x_cb(void * var, int v);
static void set_y_cb(void * var, int v);
static void icon_shake_cb(void * var, int v);

void icon_replace_demo()
{
lv_obj_t * iconx;


    screen1=lv_tileview_create(lv_scr_act());
	lv_obj_set_size(screen1,480,320);
    lv_obj_set_style_bg_color(screen1,lv_color_hex(0x000000), LV_PART_MAIN);
	lv_obj_clear_flag(screen1, LV_OBJ_FLAG_SCROLLABLE);



	for(int i=0;i<icon_count;i++)
    {
        icons[i].index=i;
        icons[i].icon=lv_tileview_create(screen1);
        lv_obj_clear_flag(icons[i].icon, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_opa(icons[i].icon,0, LV_PART_MAIN);
        lv_obj_set_pos(icons[i].icon,x_by_index(i), y_by_index(i));
		lv_obj_set_size(icons[i].icon,icon_size,icon_size+20);
		lv_obj_set_user_data(icons[i].icon,&icons[i]);
		iconx=lv_img_create(icons[i].icon);
		lv_img_set_src(iconx,icon_img[i]);
        lv_obj_add_event_cb(icons[i].icon,released_cb,LV_EVENT_RELEASED,0);
        lv_obj_add_event_cb(icons[i].icon,touching_cb,LV_EVENT_PRESSING,0);


    }



	}



static int x_by_index(int index)
{
  return (index%icon_max_col)*icon_x_distance+icon_start_x;
}


static int y_by_index(int index)
{
return (index/icon_max_col)*icon_y_distance+icon_start_y;
}

static int index_by_xy(lv_point_t * click_point)
{
    int index;
    index=(click_point->x-icon_start_x)/icon_x_distance+ (click_point->y-icon_start_y)/icon_y_distance*icon_max_col;


return index>=0?index:0;
}




static void touching_cb(lv_event_t * e)
{
    lv_obj_t * xxx=(lv_obj_t *)e->target;
    lv_obj_move_foreground(xxx);

    touch_time_count++;
    if(touch_time_count>70)
    {


            lv_point_t click_point;
            lv_indev_get_point(lv_indev_get_act(), &click_point);


            if(icon_shake==0)
            {
                icon_shake=1;

             for(int i=0;i<icon_count;i++)
                {

                    lv_anim_t a;
                    lv_anim_init(&a);
                    lv_anim_set_var(&a,lv_obj_get_child(icons[i].icon,0));
                    lv_anim_set_exec_cb(&a,icon_shake_cb);
                    lv_anim_set_time(&a,100);
                    lv_anim_set_delay(&a,rand()%100);
                    lv_anim_set_playback_time(&a,100);
                    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
                    lv_anim_set_values(&a,-50,50);
                    lv_anim_start(&a);

                }





            }


            if(tuoching==0)
            {
                offsetx=click_point.x-lv_obj_get_x(xxx);
                offsety=click_point.y-lv_obj_get_y(xxx);
                tuoching=1;
                return;
            }

            lv_obj_set_pos(xxx,click_point.x-offsetx, click_point.y-offsety);
    }

}




static void released_cb(lv_event_t * e)
{
    lv_obj_t * xxx=(lv_obj_t *)e->target;
    old_index=((icon_type *)(xxx->user_data))->index;

    tuoching=0;
    icon_shake=0;
    lv_anim_del(0,icon_shake_cb);
     for(int i=0;i<icon_count;i++)
        {
          lv_img_set_angle(lv_obj_get_child(icons[i].icon,0),0);
        }

   if(touch_time_count>70)
   {
    lv_point_t click_point;
	lv_indev_get_point(lv_indev_get_act(), &click_point);

    touch_time_count=0;
    new_index=index_by_xy(&click_point);
    if(new_index>icon_count-1){new_index=icon_count-1;}


    if(new_index<old_index)
    {



        for(int i=old_index;i>new_index;i--)
        {
          icons[i].icon=icons[i-1].icon;
          icons[i].icon->user_data=&icons[i];

                    lv_anim_t a;
					lv_anim_init(&a);
					lv_anim_set_var(&a,icons[i-1].icon);
					lv_anim_set_exec_cb(&a,set_x_cb);
                    lv_anim_set_time(&a,300);
					lv_anim_set_values(&a,x_by_index(i-1),x_by_index(i));
					lv_anim_start(&a);

                    lv_anim_t a1;
					lv_anim_init(&a1);
					lv_anim_set_var(&a1,icons[i-1].icon);
					lv_anim_set_exec_cb(&a1,set_y_cb);
                    lv_anim_set_time(&a1,300);
					lv_anim_set_values(&a1,y_by_index(i-1),y_by_index(i));
					lv_anim_start(&a1);


        }
        icons[new_index].icon=xxx;
        xxx->user_data=&icons[new_index];
        lv_obj_set_pos(xxx,x_by_index(new_index),y_by_index(new_index));return;
    }


        if(new_index>old_index)
    {



        for(int i=old_index;i<new_index;i++)
        {
          icons[i].icon=icons[i+1].icon;
          icons[i].icon->user_data=&icons[i];

                    lv_anim_t a;
					lv_anim_init(&a);
					lv_anim_set_var(&a,icons[i+1].icon);
					lv_anim_set_exec_cb(&a,set_x_cb);
                    lv_anim_set_time(&a,300);
					lv_anim_set_values(&a,x_by_index(i+1),x_by_index(i));
					lv_anim_start(&a);

                    lv_anim_t a1;
					lv_anim_init(&a1);
					lv_anim_set_var(&a1,icons[i+1].icon);
					lv_anim_set_exec_cb(&a1,set_y_cb);
                    lv_anim_set_time(&a1,300);
					lv_anim_set_values(&a1,y_by_index(i+1),y_by_index(i));
					lv_anim_start(&a1);


        }
        icons[new_index].icon=xxx;
        xxx->user_data=&icons[new_index];
        lv_obj_set_pos(xxx,x_by_index(new_index),y_by_index(new_index));return;
    }



    if(old_index==new_index)
    {
        lv_obj_set_pos(xxx,x_by_index(new_index),y_by_index(new_index));
        return;
    }

    }

    touch_time_count=0;

}



static void set_x_cb(void * var, int v)
{
    lv_obj_t * xxx=(lv_obj_t *)var;
    lv_obj_set_x(xxx,v);
}


static void set_y_cb(void * var, int v)
{
    lv_obj_t * xxx=(lv_obj_t *)var;
    lv_obj_set_y(xxx,v);
}




static void icon_shake_cb(void * var, int v)
{
   lv_obj_t * xxx=(lv_obj_t *)var;
    lv_img_set_angle(xxx,v);
}



