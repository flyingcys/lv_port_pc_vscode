#include "lvgl.h"
#include "icon_replace_2.h"
#include <stdint.h>
#include <stdlib.h>

#define  icon_start_x   90
#define  icon_start_y   50
#define  page0_icon_count 13
#define  page1_icon_count 14
#define  page2_icon_count 15
#define  icon_max_row  3
#define  icon_max_col  5
#define  icon_x_distance 140
#define  icon_y_distance 140
#define  icon_size   60
#define  page_count  3
#define  page_width  800
#define  page_hight  480


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

static const lv_img_dsc_t * icon_img[] = { &icon_0_0 ,&icon_0_1 ,&icon_0_2,&icon_0_3,&icon_0_4,&icon_0_5,&icon_0_6,&icon_0_7,&icon_0_8,&icon_0_9,&icon_0_10,&icon_0_11,&icon_0_12,&icon_0_11,&icon_0_12,};


typedef struct
{
	lv_obj_t * icon;
	int page;
	int index;

}icon_type;

static int page_icon_count[page_count]={page0_icon_count,page1_icon_count,page2_icon_count};
static icon_type icons[page_count][icon_max_row*icon_max_col];
static int offsetx,offsety,tuoching,touch_time_count,old_index,new_index,icon_shake,border_lefttest_count,border_righttest_count;
static lv_obj_t * page[page_count];
static lv_obj_t * screen;
static int xxx_tempage;
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
static void screen_released_cb(lv_event_t * e);




void icon_replace_demo_2()
{
    int i,j;
    lv_obj_t * iconx;


		screen=lv_tileview_create(lv_scr_act());
		lv_obj_set_size(screen,page_width,page_hight);
		lv_obj_set_style_bg_color(screen,lv_color_hex(0x000000), LV_PART_MAIN);


    for(j=0;j<page_count;j++)
   {

        page[j]=lv_tileview_add_tile(screen, j,0, LV_DIR_ALL);
        lv_obj_set_size(page[j],page_width,page_hight);
        lv_obj_clear_flag(page[j], LV_OBJ_FLAG_SCROLLABLE);



            for(int i=0;i<icon_max_row*icon_max_col;i++)
            {

                icons[j][i].icon=0;
                icons[j][i].page=j;
                icons[j][i].index=i;

           }


    }





    for(j=0;j<page_count;j++)

    {

            for(i=0;i<page_icon_count[j];i++)
            {
                icons[j][i].icon=lv_img_create(page[j]);

                lv_img_set_src(icons[j][i].icon,icon_img[i]);

                lv_obj_set_pos(icons[j][i].icon,x_by_index(i), y_by_index(i));
                lv_obj_add_flag(icons[j][i].icon, LV_OBJ_FLAG_CLICKABLE);
                lv_obj_add_event_cb(icons[j][i].icon,released_cb,LV_EVENT_RELEASED,0);
                lv_obj_add_event_cb(icons[j][i].icon,touching_cb,LV_EVENT_PRESSING,0);
                lv_obj_set_user_data(icons[j][i].icon,&icons[j][i]);
          }


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
    int i,j;


    lv_obj_t * xxx=(lv_obj_t *)e->target;
    lv_obj_move_foreground(xxx);
    lv_point_t click_point;





                if(tuoching==0)
            {

                lv_indev_get_point(lv_indev_get_act(), &click_point);
                offsetx=click_point.x-lv_obj_get_x(xxx);
                offsety=click_point.y-lv_obj_get_y(xxx);
                tuoching=1;
                xxx_tempage=((icon_type *)xxx->user_data)->page;

                return;
            }

            if(icon_shake)
            {
                lv_indev_get_point(lv_indev_get_act(), &click_point);
                lv_obj_set_pos(xxx,click_point.x-offsetx, click_point.y-offsety);

                if(click_point.x<icon_size/2){border_lefttest_count++;}
                else if(click_point.x>page_width-icon_size/2){border_righttest_count++;}
                else{border_lefttest_count=0;border_righttest_count=0;}

                if(border_lefttest_count>70)
                {
                    xxx_tempage=xxx_tempage>0?xxx_tempage-1:0;
                    lv_obj_set_parent(xxx,page[xxx_tempage]);
                    lv_obj_set_tile(screen,page[xxx_tempage],LV_ANIM_ON);
                    border_lefttest_count=0;
                }

                if(border_righttest_count>70)
                {
                    xxx_tempage=xxx_tempage<page_count-1?xxx_tempage+1:page_count-1;
                    lv_obj_set_parent(xxx,page[xxx_tempage]);
                    lv_obj_set_tile(screen,page[xxx_tempage],LV_ANIM_ON);
                    border_righttest_count=0;
                }

                return;
            }



    touch_time_count++;

    if(touch_time_count>70)
    {
       lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

            if(icon_shake==0)
            {
                icon_shake=1;


                for(j=0;j<page_count;j++)

                {

                    for(i=0;i<page_icon_count[j];i++)
                    {
                        lv_anim_t a;
                        lv_anim_init(&a);
                        lv_anim_set_var(&a,icons[j][i].icon);
                        lv_anim_set_exec_cb(&a,icon_shake_cb);
                        lv_anim_set_time(&a,100);
                        lv_anim_set_delay(&a,rand()%100);
                        lv_anim_set_playback_time(&a,100);
                        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
                        lv_anim_set_values(&a,-50,50);
                        lv_anim_start(&a);
                    }

                }


            }



    }

}




static void released_cb(lv_event_t * e)
{

     int i,j;
    lv_obj_t * xxx=(lv_obj_t *)e->target;
    old_index=((icon_type *)(xxx->user_data))->index;

    touch_time_count=0;

    lv_obj_add_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    tuoching=0;
    lv_anim_del(0,icon_shake_cb);

     for(j=0;j<page_count;j++)
        {

            for(i=0;i<page_icon_count[j];i++)
                    {
                        lv_img_set_angle(icons[j][i].icon,0);
                    }
        }


       if(icon_shake&&xxx_tempage==((icon_type *)xxx->user_data)->page)//同一页换图标
       {
        lv_point_t click_point;
        lv_indev_get_point(lv_indev_get_act(), &click_point);
        icon_shake=0;

        ((icon_type *)xxx->user_data)->icon=0;

        new_index=index_by_xy(&click_point);

        if(new_index>page_icon_count[xxx_tempage]-1){new_index=page_icon_count[xxx_tempage]-1;}

         j=xxx_tempage;
        if(new_index<old_index)
        {



            for(i=old_index;i>new_index;i--)
            {
              icons[j][i].icon=icons[j][i-1].icon;
              icons[j][i].icon->user_data=&icons[j][i];

                        lv_anim_t a;
                        lv_anim_init(&a);
                        lv_anim_set_var(&a,icons[j][i-1].icon);
                        lv_anim_set_exec_cb(&a,set_x_cb);
                        lv_anim_set_time(&a,300);
                        lv_anim_set_values(&a,x_by_index(i-1),x_by_index(i));
                        lv_anim_start(&a);

                        lv_anim_t a1;
                        lv_anim_init(&a1);
                        lv_anim_set_var(&a1,icons[j][i-1].icon);
                        lv_anim_set_exec_cb(&a1,set_y_cb);
                        lv_anim_set_time(&a1,300);
                        lv_anim_set_values(&a1,y_by_index(i-1),y_by_index(i));
                        lv_anim_start(&a1);


            }
            icons[j][new_index].icon=xxx;
            xxx->user_data=&icons[j][new_index];
            lv_obj_set_pos(xxx,x_by_index(new_index),y_by_index(new_index));return;
        }


            if(new_index>=old_index)
        {

            for(i=old_index;i<new_index;i++)
            {
              icons[j][i].icon=icons[j][i+1].icon;
              icons[j][i].icon->user_data=&icons[j][i];

                        lv_anim_t a;
                        lv_anim_init(&a);
                        lv_anim_set_var(&a,icons[j][i].icon);
                        lv_anim_set_exec_cb(&a,set_x_cb);
                        lv_anim_set_time(&a,300);
                        lv_anim_set_values(&a,x_by_index(i+1),x_by_index(i));
                        lv_anim_start(&a);

                        lv_anim_t a1;
                        lv_anim_init(&a1);
                        lv_anim_set_var(&a1,icons[j][i].icon);
                        lv_anim_set_exec_cb(&a1,set_y_cb);
                        lv_anim_set_time(&a1,300);
                        lv_anim_set_values(&a1,y_by_index(i+1),y_by_index(i));
                        lv_anim_start(&a1);


            }
            icons[j][new_index].icon=xxx;
            xxx->user_data=&icons[j][new_index];
            lv_obj_set_pos(xxx,x_by_index(new_index),y_by_index(new_index));return;
        }

      }



      if(icon_shake&&xxx_tempage!=((icon_type *)xxx->user_data)->page)//不同页换图标
       {
        lv_point_t click_point;
        lv_indev_get_point(lv_indev_get_act(), &click_point);
        icon_shake=0;

        new_index=index_by_xy(&click_point);

            if(icons[xxx_tempage][icon_max_row*icon_max_col-1].icon==0)//新的页面最后一个图标为零
             {
                j=xxx_tempage;
                for(i=page_icon_count[xxx_tempage];i>new_index;i--)
                {
                  icons[j][i].icon=icons[j][i-1].icon;
                  icons[j][i].icon->user_data=&icons[j][i];

                            lv_anim_t a;
                            lv_anim_init(&a);
                            lv_anim_set_var(&a,icons[j][i-1].icon);
                            lv_anim_set_exec_cb(&a,set_x_cb);
                            lv_anim_set_time(&a,300);
                            lv_anim_set_values(&a,x_by_index(i-1),x_by_index(i));
                            lv_anim_start(&a);

                            lv_anim_t a1;
                            lv_anim_init(&a1);
                            lv_anim_set_var(&a1,icons[j][i-1].icon);
                            lv_anim_set_exec_cb(&a1,set_y_cb);
                            lv_anim_set_time(&a1,300);
                            lv_anim_set_values(&a1,y_by_index(i-1),y_by_index(i));
                            lv_anim_start(&a1);


                }


                j=((icon_type *)xxx->user_data)->page;

                  for(i=old_index;i<page_icon_count[j]-1;i++)
                    {
                      icons[j][i].icon=icons[j][i+1].icon;
                      icons[j][i].icon->user_data=&icons[j][i];
                      lv_obj_set_pos(icons[j][i].icon,x_by_index(i),y_by_index(i));



                    }

                    icons[j][page_icon_count[j]-1].icon=0;

                    page_icon_count[xxx_tempage]++;
                    page_icon_count[((icon_type *)xxx->user_data)->page]--;
                    icons[xxx_tempage][new_index].icon=xxx;
                    xxx->user_data=&icons[xxx_tempage][new_index];
                    lv_obj_set_pos(xxx,x_by_index(new_index),y_by_index(new_index));return;
                 }




            if(icons[xxx_tempage][icon_max_row*icon_max_col-1].icon!=0)//新的页面最后一个图标不为零
             {
                 int old_page=((icon_type *)(xxx->user_data))->page;
                 int old_index=((icon_type *)(xxx->user_data))->index;

                 icons[old_page][old_index].icon=icons[xxx_tempage][new_index].icon;

                 lv_obj_set_parent(icons[old_page][old_index].icon,page[old_page]);
                 lv_obj_set_pos(icons[old_page][old_index].icon,x_by_index(old_index),y_by_index(old_index));
                 icons[xxx_tempage][new_index].icon->user_data=&icons[xxx_tempage][new_index];

                 icons[old_page][old_index].icon->user_data=&icons[old_page][old_index];

                icons[xxx_tempage][new_index].icon=xxx;
                xxx->user_data=&icons[xxx_tempage][new_index];
                lv_obj_set_pos(xxx,x_by_index(new_index),y_by_index(new_index));return;
             }





      }



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


