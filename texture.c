#include "texture.h"
#include "lib/string-util.h"
#include "lib/error.h"
#include "lib/array.h"

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define ASSETS_DIR "assets"

static struct texture *textures;
size_t nr_textures, alloc_textures;

static void load_texture_img(const char *filename, struct texture *texture)
{
	int channels;
	/*
	 * Using stbi_loadf() may seem like a better idea instead of manually
	 * dividing by 255 down below. However, this function uses gamma
	 * correction, which will make the image darker if the file is not
	 * appropriated for it. For a more complete explanation see:
	 * https://stackoverflow.com/a/59774898/11019779
	 */
	unsigned char *img = stbi_load(filename, &texture->W, &texture->H,
				       &channels, 0);
	if (!img)
		die("failed to load img '%s'", filename);
	size_t nr_pixels = texture->W * texture->H;
	texture->data = xmalloc(nr_pixels * sizeof(*texture->data));
	for (size_t i = 0, j = 0; i < nr_pixels; i++, j += channels)
		texture->data[i] = vec3_new(img[j]/255.0, img[j+1]/255.0, img[j+2]/255.0);
}

struct texture *load_texture(const char *name, struct texture_opts *opts)
{
	char *filename = xmkstr("%s/%s", ASSETS_DIR, name);
	ALLOC_GROW(textures, nr_textures + 1, alloc_textures);
	struct texture *texture = &textures[nr_textures++];
	memset(texture, 0, sizeof(*texture));
	load_texture_img(filename, texture);
	if (opts)
		memcpy(&texture->opts, opts, sizeof(texture->opts));
	free(filename);
	return texture;
}

void free_textures(void)
{
	for (size_t i = 0; i < nr_textures; i++)
		free(textures[i].data);
	FREE_AND_NULL(textures);
	nr_textures = alloc_textures = 0;
}
