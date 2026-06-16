#ifndef FRUIT_NINJA_EASING_H
#define FRUIT_NINJA_EASING_H

/* 所有缓动入参 t、返回值均归一化到 [0,1](ease_out_back 返回值会短暂越界,属预期回弹) */
float fruit_ninja_ease_out_quad(float t);  /* JS quadratic.co:减速 */
float fruit_ninja_ease_in_quad(float t);   /* JS quadratic.ci:加速 */
float fruit_ninja_ease_out_expo(float t);  /* JS exponential.co */
float fruit_ninja_ease_out_back(float t);  /* JS back.co:末端回弹 */

#endif
