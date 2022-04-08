#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <x86intrin.h>

static const int Scr_w = 800;
static const int Scr_h = 600;

enum Errs
{
	ERR_FILES_SPEC = -1,
	IMG_LOAD_ERR   = -2,
};

#define GET_COLOR(arr, x, y) *(arr + (int) (y) * width + (int) (x))

#define BLEND_COL(curr_p, new_p, res_p, col)\
		res_p.col = (curr_p.col * curr_p.a + curr_p.col * (255 - curr_p.a)) / 255;

inline int BlendToCanvas (Color *old_pix, Color *new_pix, Vector2 pos, int width, int height)
{
	for (int y = 0; y < height; y += 1)
	{
		for (int x = 0; x < width; x += 1)
		{
			DrawPixel ((int) x - pos.x, (int) y - pos.y, BLUE);
		
			Color curr_pixel = GET_COLOR (old_pix, x - pos.x, y - pos.y);
			Color new_pixel  = GET_COLOR (new_pix, x, y);
			Color res_pixel  = WHITE;

			BLEND_COL (curr_pixel, new_pixel, res_pixel, r);
			BLEND_COL (curr_pixel, new_pixel, res_pixel, g);
			BLEND_COL (curr_pixel, new_pixel, res_pixel, b);
			res_pixel.a = 255;

			// printf ("pixel (%d, %d) = R: %2d | G: %2d | B: %2d | A: %2d\n", 
			// 		(int) x, (int) y, res_pixel.r, res_pixel.g, res_pixel.b, res_pixel.a );

			DrawPixel (x - pos.x, y - pos.y, curr_pixel);
		}
	}
	
	return 0;
}

int ProcessKeyboard (Vector2 *init_pos, float *scale)
{
	if (IsKeyDown (KEY_Z)) *scale *= 0.9;
	if (IsKeyDown (KEY_X)) *scale /= 0.9;

	if (IsKeyDown (KEY_A)) init_pos->x += 10 * *scale;
	if (IsKeyDown (KEY_D)) init_pos->x -= 10 * *scale;

	if (IsKeyDown (KEY_W)) init_pos->y += 10 * *scale;
	if (IsKeyDown (KEY_S)) init_pos->y -= 10 * *scale;

	return GetKeyPressed();
}

int Drawing (Image *bgimg, Image *fgimg)
{	
	Vector2 pos   = {};
	float   scale = 1.0;
	
	Texture2D bgTex       = LoadTextureFromImage (*bgimg);
	Color     *old_pixels = LoadImageColors      (*bgimg);
	Color     *new_pixels = LoadImageColors      (*fgimg);

	while (!WindowShouldClose())
	{
		ProcessKeyboard (&pos, &scale);
	
		BeginDrawing();
		ClearBackground (BLACK);

		DrawTexture (bgTex, 0, 0, WHITE);

		BlendToCanvas (old_pixels, new_pixels, 
					   pos, fgimg->width, fgimg->height);

		printf ("FPS = %d\n", GetFPS());

		EndDrawing();		
	}
	
	return 0;
}

int LoadImages (int *argc, const char **argv, Image *bgimg, Image *fgimg)
{
	if (*argc != 3)
	{
		printf ("Invalid amount of files specified!\n"
				"Expected: 2, Got: %d\n"
				"Terminating...\n", *argc - 1);
				
		return ERR_FILES_SPEC;
	}

	*argc--;
	*bgimg = LoadImage (*++argv);

	*argc--;
	*fgimg = LoadImage (*++argv);
	
	return 0;
}

int main (int argc, const char **argv)
{
	Image bgimg = {},
		  fgimg = {};

	int load_err = LoadImages (&argc, argv, &bgimg, &fgimg);
	if (load_err) return load_err;

	InitWindow (Scr_w, Scr_h, "Alpha-Blending");
	SetTargetFPS (60);

	int loop_ret = Drawing(&bgimg, &fgimg);

	CloseWindow();

	if (loop_ret)
	{
		printf ("An error occured during game loop! Terminating...\n");
	}

	return 0;
}
