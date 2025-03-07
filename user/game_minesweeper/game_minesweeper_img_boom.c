#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_GMAE_MINESWEEPER_IMG_BOOM
#define LV_ATTRIBUTE_IMG_GMAE_MINESWEEPER_IMG_BOOM
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_GMAE_MINESWEEPER_IMG_BOOM uint8_t gmae_minesweeper_img_boom_map[] = {
#if LV_COLOR_DEPTH == 1 || LV_COLOR_DEPTH == 8
  /*Pixel format: Red: 3 bit, Green: 3 bit, Blue: 2 bit*/
  0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xb7, 0xb7, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 
  0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xff, 0x49, 0x49, 0xff, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 
  0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xff, 0x49, 0x49, 0xff, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 
  0xdb, 0xdb, 0xb7, 0x25, 0xdb, 0x6d, 0x24, 0x00, 0x00, 0x24, 0x49, 0xdb, 0x25, 0x92, 0xff, 0xdb, 
  0xdb, 0xdb, 0xdb, 0xdb, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0xb7, 0xdb, 0xdb, 0xdb, 
  0xdb, 0xdb, 0xdb, 0x6d, 0x00, 0x92, 0xdb, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x49, 0xdb, 0xdb, 0xdb, 
  0xdb, 0xff, 0xdb, 0x00, 0x00, 0xff, 0xff, 0xdb, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdb, 0xff, 0xff, 
  0xb7, 0x6d, 0x49, 0x00, 0x00, 0x92, 0xb7, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x6d, 0xb6, 
  0xb6, 0x25, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x25, 0x92, 
  0xdb, 0xff, 0xdb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb7, 0xff, 0xdb, 
  0xdb, 0xdb, 0xdb, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0xb7, 0xff, 0xdb, 
  0xdb, 0xdb, 0xdb, 0xdb, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0xdb, 0xdb, 0xdb, 0xdb, 
  0xdb, 0xff, 0xb7, 0x25, 0xb7, 0x49, 0x00, 0x00, 0x00, 0x00, 0x25, 0xdb, 0x25, 0x92, 0xff, 0xdb, 
  0xdb, 0xdb, 0xdb, 0x92, 0xdb, 0xdb, 0xdb, 0x49, 0x25, 0xdb, 0xb7, 0xdb, 0xb6, 0xdb, 0xdb, 0xdb, 
  0xdb, 0xdb, 0xdb, 0xff, 0xdb, 0xdb, 0xff, 0x49, 0x25, 0xff, 0xff, 0xdb, 0xff, 0xdb, 0xdb, 0xdb, 
  0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xb6, 0x92, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 
#endif
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0
  /*Pixel format: Red: 5 bit, Green: 6 bit, Blue: 5 bit*/
  0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x39, 0xce, 0x76, 0xb5, 0x75, 0xad, 0x39, 0xce, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 
  0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x59, 0xce, 0x18, 0xc6, 0x39, 0xce, 0x9e, 0xf7, 0x69, 0x4a, 0xa7, 0x39, 0x7e, 0xf7, 0x59, 0xce, 0x18, 0xc6, 0x79, 0xce, 0x38, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 
  0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0xf7, 0xbd, 0x59, 0xce, 0x39, 0xce, 0x3d, 0xef, 0x49, 0x4a, 0xa7, 0x39, 0x3c, 0xe7, 0x39, 0xce, 0x39, 0xce, 0xf7, 0xbd, 0x18, 0xc6, 0x38, 0xc6, 0x18, 0xc6, 
  0x18, 0xc6, 0x9a, 0xd6, 0x35, 0xad, 0x25, 0x29, 0x18, 0xc6, 0xec, 0x62, 0xa3, 0x18, 0x41, 0x08, 0x61, 0x08, 0x04, 0x21, 0x49, 0x4a, 0x59, 0xce, 0x66, 0x31, 0x92, 0x94, 0xdb, 0xde, 0x18, 0xc6, 
  0x18, 0xc6, 0x38, 0xc6, 0x18, 0xc6, 0x96, 0xb5, 0x29, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa7, 0x39, 0x34, 0xa5, 0x18, 0xc6, 0x39, 0xce, 0x18, 0xc6, 
  0x18, 0xc6, 0x59, 0xce, 0x18, 0xc6, 0xeb, 0x5a, 0x00, 0x00, 0xf0, 0x83, 0x59, 0xce, 0x0c, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x52, 0xb7, 0xbd, 0x9a, 0xd6, 0x18, 0xc6, 
  0x59, 0xce, 0xff, 0xff, 0x9a, 0xd6, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xb6, 0xb5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb6, 0xb5, 0xff, 0xff, 0x9a, 0xd6, 
  0x34, 0xa5, 0xec, 0x62, 0x8a, 0x52, 0x00, 0x00, 0x00, 0x00, 0xae, 0x73, 0x34, 0xa5, 0xaa, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x49, 0x4a, 0xeb, 0x5a, 0xd3, 0x9c, 
  0xf3, 0x9c, 0x86, 0x31, 0x65, 0x29, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x45, 0x29, 0x86, 0x31, 0x31, 0x8c, 
  0x59, 0xce, 0xbe, 0xf7, 0x59, 0xce, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x75, 0xad, 0xff, 0xff, 0x79, 0xce, 
  0x18, 0xc6, 0x9a, 0xd6, 0xd7, 0xbd, 0xc7, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0x29, 0x55, 0xad, 0xdb, 0xde, 0x18, 0xc6, 
  0x18, 0xc6, 0x18, 0xc6, 0x59, 0xce, 0x39, 0xce, 0x45, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x18, 0xb6, 0xb5, 0x7a, 0xd6, 0x18, 0xc6, 0x18, 0xc6, 
  0x18, 0xc6, 0x9a, 0xd6, 0x55, 0xad, 0x04, 0x21, 0x75, 0xad, 0x08, 0x42, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x25, 0x29, 0xd7, 0xbd, 0x45, 0x29, 0x72, 0x94, 0xbb, 0xde, 0x18, 0xc6, 
  0x18, 0xc6, 0x59, 0xce, 0xd7, 0xbd, 0x71, 0x8c, 0x59, 0xce, 0x96, 0xb5, 0xb6, 0xb5, 0xe7, 0x39, 0x86, 0x31, 0xb7, 0xbd, 0x55, 0xad, 0x7a, 0xd6, 0x92, 0x94, 0x96, 0xb5, 0x59, 0xce, 0x18, 0xc6, 
  0x18, 0xc6, 0xf8, 0xc5, 0x39, 0xce, 0xdb, 0xde, 0x18, 0xc6, 0x9a, 0xd6, 0xff, 0xff, 0x28, 0x42, 0x45, 0x29, 0xff, 0xff, 0x9a, 0xd6, 0x18, 0xc6, 0xbb, 0xde, 0x59, 0xce, 0xf8, 0xc5, 0x18, 0xc6, 
  0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x9a, 0xd6, 0xb3, 0x9c, 0x72, 0x94, 0x7a, 0xd6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 
#endif
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP != 0
  /*Pixel format: Red: 5 bit, Green: 6 bit, Blue: 5 bit BUT the 2 bytes are swapped*/
  0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xce, 0x39, 0xb5, 0x76, 0xad, 0x75, 0xce, 0x39, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 
  0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xce, 0x59, 0xc6, 0x18, 0xce, 0x39, 0xf7, 0x9e, 0x4a, 0x69, 0x39, 0xa7, 0xf7, 0x7e, 0xce, 0x59, 0xc6, 0x18, 0xce, 0x79, 0xc6, 0x38, 0xc6, 0x18, 0xc6, 0x18, 
  0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xbd, 0xf7, 0xce, 0x59, 0xce, 0x39, 0xef, 0x3d, 0x4a, 0x49, 0x39, 0xa7, 0xe7, 0x3c, 0xce, 0x39, 0xce, 0x39, 0xbd, 0xf7, 0xc6, 0x18, 0xc6, 0x38, 0xc6, 0x18, 
  0xc6, 0x18, 0xd6, 0x9a, 0xad, 0x35, 0x29, 0x25, 0xc6, 0x18, 0x62, 0xec, 0x18, 0xa3, 0x08, 0x41, 0x08, 0x61, 0x21, 0x04, 0x4a, 0x49, 0xce, 0x59, 0x31, 0x66, 0x94, 0x92, 0xde, 0xdb, 0xc6, 0x18, 
  0xc6, 0x18, 0xc6, 0x38, 0xc6, 0x18, 0xb5, 0x96, 0x4a, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0xa7, 0xa5, 0x34, 0xc6, 0x18, 0xce, 0x39, 0xc6, 0x18, 
  0xc6, 0x18, 0xce, 0x59, 0xc6, 0x18, 0x5a, 0xeb, 0x00, 0x00, 0x83, 0xf0, 0xce, 0x59, 0x63, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x6a, 0xbd, 0xb7, 0xd6, 0x9a, 0xc6, 0x18, 
  0xce, 0x59, 0xff, 0xff, 0xd6, 0x9a, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xb5, 0xb6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb5, 0xb6, 0xff, 0xff, 0xd6, 0x9a, 
  0xa5, 0x34, 0x62, 0xec, 0x52, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x73, 0xae, 0xa5, 0x34, 0x52, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x4a, 0x49, 0x5a, 0xeb, 0x9c, 0xd3, 
  0x9c, 0xf3, 0x31, 0x86, 0x29, 0x65, 0x08, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x29, 0x45, 0x31, 0x86, 0x8c, 0x31, 
  0xce, 0x59, 0xf7, 0xbe, 0xce, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xad, 0x75, 0xff, 0xff, 0xce, 0x79, 
  0xc6, 0x18, 0xd6, 0x9a, 0xbd, 0xd7, 0x39, 0xc7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x45, 0xad, 0x55, 0xde, 0xdb, 0xc6, 0x18, 
  0xc6, 0x18, 0xc6, 0x18, 0xce, 0x59, 0xce, 0x39, 0x29, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xc3, 0xb5, 0xb6, 0xd6, 0x7a, 0xc6, 0x18, 0xc6, 0x18, 
  0xc6, 0x18, 0xd6, 0x9a, 0xad, 0x55, 0x21, 0x04, 0xad, 0x75, 0x42, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x29, 0x25, 0xbd, 0xd7, 0x29, 0x45, 0x94, 0x72, 0xde, 0xbb, 0xc6, 0x18, 
  0xc6, 0x18, 0xce, 0x59, 0xbd, 0xd7, 0x8c, 0x71, 0xce, 0x59, 0xb5, 0x96, 0xb5, 0xb6, 0x39, 0xe7, 0x31, 0x86, 0xbd, 0xb7, 0xad, 0x55, 0xd6, 0x7a, 0x94, 0x92, 0xb5, 0x96, 0xce, 0x59, 0xc6, 0x18, 
  0xc6, 0x18, 0xc5, 0xf8, 0xce, 0x39, 0xde, 0xdb, 0xc6, 0x18, 0xd6, 0x9a, 0xff, 0xff, 0x42, 0x28, 0x29, 0x45, 0xff, 0xff, 0xd6, 0x9a, 0xc6, 0x18, 0xde, 0xbb, 0xce, 0x59, 0xc5, 0xf8, 0xc6, 0x18, 
  0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xd6, 0x9a, 0x9c, 0xb3, 0x94, 0x72, 0xd6, 0x7a, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 
#endif
#if LV_COLOR_DEPTH == 32
  /*Pixel format: Fix 0xFF: 8 bit, Red: 8 bit, Green: 8 bit, Blue: 8 bit*/
  0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc5, 0xc5, 0xc5, 0xff, 0xac, 0xac, 0xac, 0xff, 0xab, 0xab, 0xab, 0xff, 0xc5, 0xc5, 0xc5, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 
  0xc0, 0xc0, 0xc0, 0xff, 0xbf, 0xbf, 0xbf, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xc9, 0xc9, 0xc9, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xc5, 0xc5, 0xc5, 0xff, 0xf0, 0xf0, 0xf0, 0xff, 0x4b, 0x4b, 0x4b, 0xff, 0x35, 0x35, 0x35, 0xff, 0xec, 0xec, 0xec, 0xff, 0xc8, 0xc8, 0xc8, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xca, 0xca, 0xca, 0xff, 0xc3, 0xc3, 0xc3, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 
  0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xbb, 0xbb, 0xbb, 0xff, 0xc6, 0xc6, 0xc6, 0xff, 0xc4, 0xc4, 0xc4, 0xff, 0xe5, 0xe5, 0xe5, 0xff, 0x49, 0x49, 0x49, 0xff, 0x34, 0x34, 0x34, 0xff, 0xe3, 0xe3, 0xe3, 0xff, 0xc5, 0xc5, 0xc5, 0xff, 0xc6, 0xc5, 0xc6, 0xff, 0xbb, 0xbb, 0xbb, 0xff, 0xbf, 0xbf, 0xbf, 0xff, 0xc2, 0xc2, 0xc2, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 
  0xc0, 0xc0, 0xc0, 0xff, 0xcf, 0xcf, 0xcf, 0xff, 0xa5, 0xa5, 0xa5, 0xff, 0x25, 0x25, 0x25, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0x5c, 0x5c, 0x5c, 0xff, 0x15, 0x15, 0x15, 0xff, 0x09, 0x09, 0x09, 0xff, 0x0a, 0x0a, 0x0a, 0xff, 0x1e, 0x1e, 0x1e, 0xff, 0x46, 0x46, 0x46, 0xff, 0xc9, 0xc8, 0xc9, 0xff, 0x2c, 0x2c, 0x2c, 0xff, 0x8e, 0x8e, 0x8e, 0xff, 0xd7, 0xd7, 0xd7, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 
  0xc0, 0xc0, 0xc0, 0xff, 0xc3, 0xc3, 0xc3, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xb0, 0xb0, 0xb0, 0xff, 0x45, 0x45, 0x45, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x34, 0x34, 0x34, 0xff, 0xa3, 0xa3, 0xa3, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xc5, 0xc5, 0xc5, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 
  0xc0, 0xc0, 0xc0, 0xff, 0xc8, 0xc8, 0xc8, 0xff, 0xbe, 0xbe, 0xbe, 0xff, 0x5a, 0x5a, 0x5a, 0xff, 0x00, 0x00, 0x00, 0xff, 0x7d, 0x7d, 0x7d, 0xff, 0xc6, 0xc6, 0xc6, 0xff, 0x61, 0x61, 0x61, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x4d, 0x4d, 0x4d, 0xff, 0xb5, 0xb5, 0xb5, 0xff, 0xcf, 0xcf, 0xcf, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 
  0xc9, 0xc9, 0xc9, 0xff, 0xfc, 0xfc, 0xfc, 0xff, 0xce, 0xce, 0xce, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xb2, 0xb2, 0xb2, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x01, 0x01, 0x01, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xb2, 0xb2, 0xb2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd1, 0xd1, 0xd1, 0xff, 
  0xa3, 0xa3, 0xa3, 0xff, 0x5c, 0x5c, 0x5c, 0xff, 0x4f, 0x4f, 0x4f, 0xff, 0x01, 0x01, 0x01, 0xff, 0x00, 0x00, 0x00, 0xff, 0x73, 0x73, 0x73, 0xff, 0xa2, 0xa2, 0xa2, 0xff, 0x52, 0x52, 0x52, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x02, 0x02, 0x02, 0xff, 0x47, 0x47, 0x47, 0xff, 0x5b, 0x5b, 0x5b, 0xff, 0x96, 0x96, 0x96, 0xff, 
  0x9b, 0x9b, 0x9b, 0xff, 0x2e, 0x2e, 0x2e, 0xff, 0x2b, 0x2b, 0x2b, 0xff, 0x07, 0x07, 0x07, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x02, 0x02, 0x02, 0xff, 0x27, 0x27, 0x27, 0xff, 0x2e, 0x2e, 0x2e, 0xff, 0x85, 0x85, 0x85, 0xff, 
  0xc6, 0xc6, 0xc6, 0xff, 0xf2, 0xf2, 0xf2, 0xff, 0xc9, 0xc9, 0xc9, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0xab, 0xab, 0xab, 0xff, 0xfd, 0xfd, 0xfd, 0xff, 0xcb, 0xcb, 0xcb, 0xff, 
  0xbf, 0xbf, 0xbf, 0xff, 0xcf, 0xcf, 0xcf, 0xff, 0xb7, 0xb7, 0xb7, 0xff, 0x36, 0x36, 0x36, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x01, 0x01, 0x01, 0xff, 0x01, 0x01, 0x01, 0xff, 0x01, 0x01, 0x01, 0xff, 0x01, 0x01, 0x01, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x27, 0x27, 0x27, 0xff, 0xa9, 0xa9, 0xa9, 0xff, 0xd6, 0xd6, 0xd6, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 
  0xc0, 0xc0, 0xc0, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xc9, 0xc9, 0xc9, 0xff, 0xc4, 0xc4, 0xc4, 0xff, 0x29, 0x29, 0x29, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x18, 0x18, 0x18, 0xff, 0xb3, 0xb3, 0xb3, 0xff, 0xcc, 0xcc, 0xcc, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 
  0xc0, 0xc0, 0xc0, 0xff, 0xd1, 0xd1, 0xd1, 0xff, 0xa9, 0xa9, 0xa9, 0xff, 0x21, 0x21, 0x21, 0xff, 0xaa, 0xaa, 0xaa, 0xff, 0x3e, 0x3e, 0x3e, 0xff, 0x00, 0x00, 0x00, 0xff, 0x01, 0x01, 0x01, 0xff, 0x02, 0x02, 0x02, 0xff, 0x00, 0x00, 0x00, 0xff, 0x24, 0x24, 0x24, 0xff, 0xb6, 0xb6, 0xb6, 0xff, 0x27, 0x27, 0x27, 0xff, 0x8d, 0x8d, 0x8d, 0xff, 0xd5, 0xd5, 0xd5, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 
  0xc0, 0xc0, 0xc0, 0xff, 0xc6, 0xc6, 0xc6, 0xff, 0xb7, 0xb7, 0xb7, 0xff, 0x8b, 0x8b, 0x8b, 0xff, 0xc9, 0xc9, 0xc9, 0xff, 0xb1, 0xb1, 0xb1, 0xff, 0xb3, 0xb3, 0xb3, 0xff, 0x3b, 0x3b, 0x3b, 0xff, 0x2e, 0x2e, 0x2e, 0xff, 0xb4, 0xb4, 0xb4, 0xff, 0xa8, 0xa8, 0xa8, 0xff, 0xcc, 0xcc, 0xcc, 0xff, 0x91, 0x91, 0x91, 0xff, 0xb1, 0xb1, 0xb1, 0xff, 0xc6, 0xc6, 0xc6, 0xff, 0xbf, 0xbf, 0xbf, 0xff, 
  0xc0, 0xc0, 0xc0, 0xff, 0xbd, 0xbd, 0xbd, 0xff, 0xc4, 0xc4, 0xc4, 0xff, 0xd6, 0xd6, 0xd6, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xce, 0xce, 0xce, 0xff, 0xff, 0xff, 0xff, 0xff, 0x43, 0x42, 0x43, 0xff, 0x29, 0x29, 0x29, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0xd0, 0xd0, 0xff, 0xbf, 0xbf, 0xbf, 0xff, 0xd5, 0xd5, 0xd5, 0xff, 0xc7, 0xc7, 0xc7, 0xff, 0xbd, 0xbd, 0xbd, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 
  0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xbf, 0xbf, 0xbf, 0xff, 0xce, 0xce, 0xce, 0xff, 0x94, 0x94, 0x94, 0xff, 0x8c, 0x8c, 0x8c, 0xff, 0xcc, 0xcc, 0xcc, 0xff, 0xbf, 0xbf, 0xbf, 0xff, 0xbf, 0xbf, 0xbf, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 0xc0, 0xc0, 0xc0, 0xff, 
#endif
};

const lv_img_dsc_t game_minesweeper_img_boom = {
  .header.cf = LV_IMG_CF_TRUE_COLOR,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 16,
  .header.h = 16,
  .data_size = 256 * LV_COLOR_SIZE / 8,
  .data = gmae_minesweeper_img_boom_map,
};
