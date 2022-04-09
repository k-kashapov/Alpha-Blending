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

#define GET_OFFSET(width, x, y) ((int) y * width + (int) x)

#define BLEND_COL(curr_p, new_p, res_p, col)\
		res_p.col = ((int) new_p.col * new_p.a + (int) curr_p.col * (255 - new_p.a)) / 255;

Color GetPixel (Color *arr, int arr_w, int arr_h, int x, int y)
{
	Color pix = { 0 };

	if (x < arr_w && y < arr_h && x > 0 && y > 0)
	{
		int arr_offset = GET_OFFSET (arr_w, x, y);

		if (arr_offset < (arr_w * arr_h))
		{
			pix = *(arr + arr_offset);
		}
	}
	
	return pix;
}

inline int BlendToCanvas (Color *old_pix, Color *new_pix, Color *scr, Vector2 pos, Image *fr, Image *bg)
{
	for (int y = 0; y < Scr_h; y += 1)
	{
		for (int x = 0; x < Scr_w; x += 1)
		{
			Color curr_pixel = GetPixel (old_pix, bg->width, bg->height, x,         y);
			Color new_pixel  = GetPixel (new_pix, fr->width, fr->height, x - pos.x, y - pos.y);
			Color res_pixel  = Color { 0 };

			BLEND_COL (curr_pixel, new_pixel, res_pixel, r);
			BLEND_COL (curr_pixel, new_pixel, res_pixel, g);
			BLEND_COL (curr_pixel, new_pixel, res_pixel, b);
			BLEND_COL (curr_pixel, new_pixel, res_pixel, a);

			// printf ("pixel (%d, %d) = R: %2d | G: %2d | B: %2d | A: %2d\n", 
			// 		(int) x, (int) y, res_pixel.r, res_pixel.g, res_pixel.b, res_pixel.a );

			*(scr + Scr_w * y + x) = res_pixel;
		}
	}
	
	return 0;
}

int ProcessKeyboard (Vector2 *init_pos)
{
	if (IsKeyDown (KEY_A)) init_pos->x -= 10;
	if (IsKeyDown (KEY_D)) init_pos->x += 10;

	if (IsKeyDown (KEY_W)) init_pos->y -= 10;
	if (IsKeyDown (KEY_S)) init_pos->y += 10;

	return 0;
}

int Drawing (Image *bgimg, Image *fgimg)
{	
	Vector2 pos = {};

	Image     flat        = GenImageColor        (Scr_w, Scr_h, GREEN);
	Color     *scr        = LoadImageColors      (flat);
	Texture2D bgTex       = LoadTextureFromImage (flat);
	
	Color     *old_pixels = LoadImageColors      (*bgimg);
	Color     *new_pixels = LoadImageColors      (*fgimg);

	while (!WindowShouldClose())
	{
		ProcessKeyboard (&pos);
	
		BeginDrawing();
		ClearBackground (BLUE);

		for (int i = 0; i < 100; i++)
		{
			BlendToCanvas (old_pixels, new_pixels, scr,
						   pos, fgimg, bgimg);
		}

		UpdateTexture (bgTex, scr);

		DrawTexture (bgTex, 0, 0, WHITE);

		printf ("FPS = %d | FrameTime = %f\n", GetFPS(), GetFrameTime());

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
