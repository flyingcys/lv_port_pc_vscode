#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "../src/v9-fruit_ninja/fruit_ninja_assets.h"

static void test_paths_with_explicit_root(void)
{
    char image_path[512];
    char audio_path[512];

    bool ok = fruit_ninja_assets_init("/tmp/fruit-ninja-root");
    assert(ok);

    ok = fruit_ninja_assets_build_image_path(image_path, sizeof(image_path), "images/background.jpg");
    assert(ok);
    assert(strcmp(image_path,
                  "A:/tmp/fruit-ninja-root/main/assets/fruit_ninja/images/background.jpg") == 0);

    ok = fruit_ninja_assets_build_audio_path(audio_path, sizeof(audio_path), "sound/menu.ogg");
    assert(ok);
    assert(strcmp(audio_path,
                  "/tmp/fruit-ninja-root/main/assets/fruit_ninja/sound/menu.ogg") == 0);
}

static void test_project_root_inference_and_core_files(void)
{
    char image_path[512];
    bool ok = fruit_ninja_assets_init(NULL);
    assert(ok);

    ok = fruit_ninja_assets_build_image_path(image_path, sizeof(image_path), "images/logo.png");
    assert(ok);
    assert(strstr(image_path, "main/assets/fruit_ninja/images/logo.png") != NULL);
    assert(fruit_ninja_assets_validate_core_files());
}

static void test_rejects_small_output_buffer(void)
{
    char tiny[8];

    assert(fruit_ninja_assets_init("/tmp/fruit-ninja-root"));
    assert(!fruit_ninja_assets_build_image_path(tiny, sizeof(tiny), "images/background.jpg"));
}

int main(void)
{
    test_paths_with_explicit_root();
    test_project_root_inference_and_core_files();
    test_rejects_small_output_buffer();
    return 0;
}
