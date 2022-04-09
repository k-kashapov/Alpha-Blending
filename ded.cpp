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

const char I = 255u,
	 	   Z = 0x80u;

const __m128i   _0 =                    _mm_set_epi8 (0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0);
const __m128i _255 = _mm_cvtepu8_epi16 (_mm_set_epi8 (I,I,I,I, I,I,I,I, I,I,I,I, I,I,I,I));

int BlendToCanvas (Color *back, Color *front, Color *scr, int width, int height, int bgwidth, Vector2 pos)
{
	for (int y = 0; y < Scr_h; y++)
	{
		for (int x = 0; x < Scr_w; x += 4)
		{	
		    //-----------------------------------------------------------------------
		    //       15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
		    // fr = [r3 g3 b3 a3 | r2 g2 b2 a2 | r1 g1 b1 a1 | r0 g0 b0 a0]
		    //-----------------------------------------------------------------------
		
			int froffs = (y - (int) pos.y) * width   + x - (int) pos.x;
			int bgoffs =  y                * bgwidth + x;

			__m128i fr = _mm_set1_epi32 (0);

			if (froffs >= 0                   &&
				froffs <= width * height - 3  &&
				x      <= (int) pos.x + width &&
				x      >= (int) pos.x)
			{
				fr = _mm_set_epi32 (*(int *)(front + froffs + 3), *(int *)(front + froffs + 2),
									*(int *)(front + froffs + 1), *(int *)(front + froffs));
			}

			__m128i bk = _mm_set_epi32 (*(int *)(back  + bgoffs + 3), *(int *)(back  + bgoffs + 2),
										*(int *)(back  + bgoffs + 1), *(int *)(back  + bgoffs));
			
		    // fr = front[y][x]
		
		    //-----------------------------------------------------------------------
		    //       15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
		    // fr = [a3 r3 g3 b3 | a2 r2 g2 b2 | a1 r1 g1 b1 | a0 r0 g0 b0]
		    //        \  \  \  \    \  \  \  \   xx xx xx xx   xx xx xx xx
		    //         \  \  \  \    \  \  \  \.
		    //          \  \  \  \    '--+--+--+-------------+--+--+--.
		    //           '--+--+--+------------+--+--+--.     \  \  \  \.
		    //                                  \  \  \  \     \  \  \  \.
		    // FR = [-- -- -- -- | -- -- -- -- | a3 r3 g3 b3 | a2 r2 g2 b2]
		    //-----------------------------------------------------------------------
		
		    __m128i FR = (__m128i) _mm_movehl_ps ((__m128) _0, (__m128) fr);       // FR = (fr >> 8*8)
		    __m128i BK = (__m128i) _mm_movehl_ps ((__m128) _0, (__m128) bk);
		
		    //-----------------------------------------------------------------------
		    //       15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
		    // fr = [a3 r3 g3 b3 | a2 r2 g2 b2 | a1 r1 g1 b1 | a0 r0 g0 b0]
		    //       xx xx xx xx   xx xx xx xx                 /  /   |  |
		    //                                         _______/  /   /   |
		    //            ...   ...     ...           /     ____/   /    |
		    //           /     /       /             /     /       /     |
		    // fr = [-- a1 -- r1 | -- g1 -- b1 | -- a0 -- r0 | -- g0 -- b0]
		    //-----------------------------------------------------------------------
		
		    fr = _mm_cvtepi8_epi16 (fr);                          // fr[i] = (WORD) fr[i]
		    FR = _mm_cvtepi8_epi16 (FR);
		
		    bk = _mm_cvtepi8_epi16 (bk);
		    BK = _mm_cvtepi8_epi16 (BK);
		
		    //-----------------------------------------------------------------------
		    //       15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
		    // fr = [-- a1 -- r1 | -- g1 -- b1 | -- a0 -- r0 | -- g0 -- b0]
		    //          |___________________        |___________________
		    //          |     \      \      \       |     \      \      \.
		    // a  = [-- a1 -- a1 | -- a1 -- a1 | -- a0 -- a0 | -- a0 -- a0]
		    //-----------------------------------------------------------------------
		
		    static const __m128i moveA = _mm_set_epi8 (Z, 0xE, Z, 0xE, Z, 0xE, Z, 0xE,
		                                               Z, 0x6, Z, 0x6, Z, 0x6, Z, 0x6);
		                                               
		    __m128i a = _mm_shuffle_epi8 (fr, moveA);            // a [for r0/b0/b0...] = a0...
		    __m128i A = _mm_shuffle_epi8 (FR, moveA);
		
		    //-----------------------------------------------------------------------
		
		    fr = _mm_mullo_epi16 (fr, a);                        // fr *= a
		    FR = _mm_mullo_epi16 (FR, A);
		
		    bk = _mm_mullo_epi16 (bk, _mm_sub_epi16 (_255, a));  // bk *= (255-a)
		    BK = _mm_mullo_epi16 (BK, _mm_sub_epi16 (_255, A));
		
		    __m128i sum = _mm_add_epi16 (fr, bk);                // sum = fr*a + bk*(255-a)
		    __m128i SUM = _mm_add_epi16 (FR, BK);
		
		    //-----------------------------------------------------------------------
		    //        15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
		    // sum = [A1 a1 R1 r1 | G1 g1 B1 b1 | A0 a0 R0 r0 | G0 g0 B0 b0]
		    //         \     \       \     \       \_____\_______\_____\.
		    //          \_____\_______\_____\______________    \  \  \  \.
		    //                                    \  \  \  \    \  \  \  \.
		    // sum = [-- -- -- -- | -- -- -- -- | A1 R1 G1 B1 | A0 R0 G0 B0]
		    //-----------------------------------------------------------------------
		
		    static const __m128i moveSum = _mm_set_epi8 (Z,  Z,  Z,  Z, Z, Z, Z, Z,
		                                                 15, 13, 11, 9, 7, 5, 3, 1);
		    sum = _mm_shuffle_epi8 (sum, moveSum);       // sum[i] = (sum[i] >> 8) = (sum[i] / 256)
		    SUM = _mm_shuffle_epi8 (SUM, moveSum);
		
		    //-----------------------------------------------------------------------
		    //          15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
		    // sum   = [-- -- -- -- | -- -- -- -- | a1 r1 g1 b1 | a0 r0 g0 b0] ->-.
		    // sumHi = [-- -- -- -- | -- -- -- -- | a3 r3 g3 b3 | a2 r2 g2 b2]    |
		    //                                      /  /  /  /    /  /  /  /      V
		    //             .--+--+--+----+--+--+--++--+--+--+----+--+--+--'       |
		    //            /  /  /  /    /  /  /  /    ____________________________/
		    //           /  /  /  /    /  /  /  /    /  /  /  /    /  /  /  /
		    // color = [a3 r3 g3 b3 | a2 r2 g2 b2 | a1 r1 g1 b1 | a0 r0 g0 b0]
		    //-----------------------------------------------------------------------
		
		    __m128i color = (__m128i) _mm_movelh_ps ((__m128) sum, (__m128) SUM);  // color = (sumHi << 8*8) | sum
		
		    _mm_storeu_si128 ((__m128i*) (scr + y * Scr_w + x), color);
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
	Vector2 pos = { 100, 200 };

	Image flat = GenImageColor (Scr_w, Scr_h, BLUE);
	
	Color     *old_pixels = LoadImageColors (*bgimg);
	Color     *new_pixels = LoadImageColors (*fgimg);
	Color     *scr        = LoadImageColors (flat);
	
	Texture2D bgTex = LoadTextureFromImage (flat);

	int width  = fgimg->width;
	int height = fgimg->height;

	while (!WindowShouldClose())
	{
		ProcessKeyboard (&pos);

		for (int i = 0; i < 100; i++)
		{
			BlendToCanvas (old_pixels, new_pixels, scr, 
						   width, height, bgimg->width, pos);
		}
		
		UpdateTexture (bgTex, scr);
	
		BeginDrawing();	
		
		DrawTexture (bgTex, 0, 0, WHITE);

		printf ("FPS = %d | frameTime = %f\n", GetFPS(), GetFrameTime());

		EndDrawing();		
	}

	UnloadImage (flat);

	free (old_pixels);
	free (new_pixels);
	free (scr);
	
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

	UnloadImage (bgimg);
	UnloadImage (fgimg);

	CloseWindow();

	if (loop_ret)
	{
		printf ("An error occured during game loop! Terminating...\n");
	}

	return 0;
}
