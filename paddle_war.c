
#include <gb/gb.h>
#include <gb/drawing.h>
#include <rand.h>

unsigned char sprite_data[] =
{
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,
  0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

#define Width				160
#define Height 				144
#define Centerx				Width / 2
#define Centery 			Height / 2
#define offset_y 			9
#define offset_y_player 	9
#define score_offset_x  	0
#define winloseoffset   	14
#define player_move_speed   4

// sound channel 2
#define SND_LENGTH(N)             (N % 64)
#define WAV_DUTY(N)               (N % 4) << 6
#define ENVELOPE_STEPS(N)         (N % 8)
#define ENVELOPE_UP_OR_DOWN(N)    (N % 2) << 3
#define DEFAULT_ENVELOPE_VALUE(N) (N % 16) << 4
#define HOFD(N)                   (N % 8) // high order freq data
#define INIT_SND(N)               (N % 2) << 7

// channel 1
#define SWEEP_SHIFT(N)            (N % 8)
#define SWEEP_TIME(N)             (N % 8) << 4
#define SWEEP_INC_OR_DEC(N)       (N % 2) << 3


void ball_bounce_sound() {
        NR14_REG = INIT_SND(0);
        NR21_REG = WAV_DUTY(3) | SND_LENGTH(4);
        NR22_REG = DEFAULT_ENVELOPE_VALUE(3) | ENVELOPE_UP_OR_DOWN(0) | ENVELOPE_STEPS(1);
        NR23_REG = 0x0F; // low order freq data
        NR24_REG = INIT_SND(1);
}

void player_lose_sound() {
	    NR14_REG = INIT_SND(0) | HOFD(85); // init sound off
        NR10_REG = SWEEP_TIME(3) | SWEEP_INC_OR_DEC(189) | SWEEP_SHIFT(241);
        NR11_REG = WAV_DUTY(98) | SND_LENGTH(227);
        NR12_REG = DEFAULT_ENVELOPE_VALUE(119) | ENVELOPE_UP_OR_DOWN(82) | ENVELOPE_STEPS(214);
        NR13_REG = 150; // waveform data one byte
        NR14_REG = INIT_SND(1) | HOFD(85); // init sound on
}

void player_win_sound() {
		NR14_REG = INIT_SND(0) | HOFD(92); // init sound off
        NR10_REG = SWEEP_TIME(4) | SWEEP_INC_OR_DEC(214) | SWEEP_SHIFT(149);
        NR11_REG = WAV_DUTY(78) | SND_LENGTH(147);
        NR12_REG = DEFAULT_ENVELOPE_VALUE(39) | ENVELOPE_UP_OR_DOWN(253) | ENVELOPE_STEPS(57);
        NR13_REG = 45; // waveform data one byte
        NR14_REG = INIT_SND(1) | HOFD(92); // init sound on
}

void nice_shot_sound() {
		NR14_REG = INIT_SND(0) | HOFD(246); // init sound off
        NR10_REG = SWEEP_TIME(0) | SWEEP_INC_OR_DEC(119) | SWEEP_SHIFT(79);
        NR11_REG = WAV_DUTY(162) | SND_LENGTH(35);
        NR12_REG = DEFAULT_ENVELOPE_VALUE(181) | ENVELOPE_UP_OR_DOWN(108) | ENVELOPE_STEPS(137);
        NR13_REG = 129; // waveform data one byte
        NR14_REG = INIT_SND(1) | HOFD(246); // init sound on
	
}

void clear_text() {
	gotogxy(winloseoffset, 0);
	gprintf("      ");
	gotogxy(winloseoffset, 17);
	gprintf("      ");
}


#define Enemy_move_speed 		1
#define Initial_ball_speed      1

void main() {
	UINT8 ballxy[2] = {Centerx, Centery};
	UINT8 player_x = Centerx;
	UINT8 enemy__x = Centerx;
	BYTE ball_move_speedx = Initial_ball_speed;
	BYTE ball_move_speedy = Initial_ball_speed;
	UINT8 halt = 0;
	UINT8 pause4 = 0;
	UINT8 player_score = 0;
	UINT8 enemy_score = 0;
	UWORD seed = 0;
	UINT8 paused = 0;
	UINT8 attack_type = 0;
	UINT8 hits = 0;
	UINT8 menu = 1;
	UINT8 pts = 0;

	// init sound
	NR52_REG = 0x80; // sound on
	NR50_REG = 0x7F; // master volume
	NR51_REG = 0xFF; // enables channels

	disable_interrupts();
  	DISPLAY_OFF;

	set_sprite_data(0,4, sprite_data);
	// players sprites
	set_sprite_tile(0,0);
	set_sprite_tile(2,0);
	// enemy sprites
	set_sprite_tile(1,1);
	set_sprite_tile(3,1);

	// ball
	set_sprite_tile(4, 2);
	// player
	move_sprite(0, player_x, Width-8);
	move_sprite(2, player_x + 8, Width-8);
	// enemy
	move_sprite(1, enemy__x, 16);
	move_sprite(3, enemy__x + 8, 16);
	// ball
	move_sprite(4, Centerx, Centery);

	// seed rand for the first time.
	seed = DIV_REG;
	seed |= (UWORD)DIV_REG << 8;
	initarand(seed);


	DISPLAY_ON;
	enable_interrupts();

	// draw lines for boundery
	line(0,offset_y, 159,offset_y);
	line(0, 134, 159, 134);

	gotogxy(2,4);
	gprintf("First to ");
	gotogxy(2,6);
	gprintf("UP +");
	gotogxy(2,7);
	gprintf("DOWN -");
	gotogxy(12,4);
	gprintf("%d", pts);
	gotogxy(15,4);
	gprintf("PTS:");

	// game loop counting menu and game over 
	SHOW_SPRITES;
	while (1) {
	// menu initialization
	while (1) {

		if ( (player_score == pts || enemy_score == pts) && menu == 0) {
			break;
		}

		if (joypad() & J_START && menu == 0) {
			waitpadup();
			paused = 1;
			}

		if(hits == 2) {
			if (ball_move_speedy < 10) {
				ball_move_speedy++;
			}

			hits = 0;
		}

		if (halt == 0) {
			ballxy[0] += ball_move_speedx;
			ballxy[1] += ball_move_speedy;
				if (joypad() & J_LEFT && menu == 0) {
					if (player_x > 8) {
						player_x -= player_move_speed;
					}
				
			}
			if (joypad() & J_RIGHT && menu == 0) {
				if (player_x + 8 < Width) {
					player_x += player_move_speed;
					
				}
			}
			
		}
		if (halt == 0) {
			if (attack_type == 0) {
				if (enemy__x + 16 < ballxy[0] + 4) {
					enemy__x += Enemy_move_speed;
				}
				if (enemy__x > ballxy[0] + 4) {
					enemy__x -= Enemy_move_speed;
				}
			}
			else {
				if (rand() % 2 == 0) {
					if (enemy__x + 14 < ballxy[0] + 4) {
						enemy__x += Enemy_move_speed;
					}
					if (enemy__x + 2 > ballxy[0] + 4) {
						enemy__x -= Enemy_move_speed;
					}
				}
				else
				{
					if (enemy__x + 10 < ballxy[0] + 4) {
						enemy__x += Enemy_move_speed;
					}
					if (enemy__x + 6 > ballxy[0] + 4) {
						enemy__x -= Enemy_move_speed;
					}
				}
			}
		}
		else { // halt = 1
			if (enemy__x + 4 > Centerx) {
				enemy__x--;
			}
			else if (enemy__x + 4 < Centerx) {
				enemy__x++;
			}
			if (pause4 == 0) {
				hits = 0;
				ball_move_speedy = Initial_ball_speed;
				halt = 0;	
				ballxy[0] = Centerx;
				ballxy[1] = Centery;
				ball_move_speedx = (rand() / 64) + Initial_ball_speed;
				if ((rand()+1) % 2 == 0) {
					ball_move_speedx -= 1;
				}
				//ball_move_speedx = 0; for testing

				if (player_score > enemy_score && menu == 0) {
					clear_text();

				}
				else if (player_score < enemy_score && menu == 0) {
					clear_text();
				}
				else if (menu == 0) {
					clear_text();
				}

			}
			pause4--;
		}
		


		if (halt == 0) {


			if (ballxy[0] < 4) {
				ball_move_speedx *= -1;
				ball_bounce_sound();
			}
			if (ballxy[0] >= Width+4) {
				ball_move_speedx *= -1;
				ball_bounce_sound();
			}

				if (ballxy[1] < 15 + offset_y) {
				if (ballxy[0] + 4 >= enemy__x && ballxy[0] <= enemy__x + 12) { // colides with paddle
					ball_move_speedy *= -1;

					if (ballxy[0] + 4 >= enemy__x && ballxy[0] + 4 <= enemy__x + 1) {
						nice_shot_sound();
						ball_move_speedx = - 3;

					}
					else if (ballxy[0] + 6 >= enemy__x && ballxy[0] + 4 <= enemy__x + 4) {
						ball_bounce_sound(); // play sound
						ball_move_speedx = - 1;
					}
					else if (ballxy[0] >= enemy__x + 6 && ballxy[0] <= enemy__x + 10) {
						ball_bounce_sound(); // play sound
						ball_move_speedx = 1;
					}
					else if (ballxy[0] >= enemy__x + 8 && ballxy[0] <= enemy__x + 12) {
						nice_shot_sound();
						ball_move_speedx = 3;
					}
					else
					{
						ball_bounce_sound(); // play sound
						ball_move_speedx = 0;
					}
					attack_type = rand() % 3;
					hits++;
					

				}
				else // does not colide with paddle
				{
					player_win_sound();
					player_score++;
					if (menu == 0) {
						gotogxy(score_offset_x,17);
						gprintf(" ");
						gotogxy(score_offset_x,17);
						gprintf("%d", player_score);
					}
					halt = 1;
					pause4 = 100;
					ballxy[0] = 200;
				}
				
			}
			if (ballxy[1] >= Width-7 - offset_y_player) {
				if (ballxy[0] + 4 >= player_x && ballxy[0] <= player_x + 12) { // colides with paddle
					ball_move_speedy *= -1;
					
					if (ballxy[0] + 4 >= player_x && ballxy[0] + 4 <= player_x + 1) {
						nice_shot_sound();
						ball_move_speedx = - 3;
					}
					else if (ballxy[0] + 6 >= player_x && ballxy[0] + 4 <= player_x + 4) {
						ball_bounce_sound(); // play sound
						ball_move_speedx = - 1;
					}
					else if (ballxy[0] >= player_x + 6 && ballxy[0] <= player_x + 10) {
						ball_bounce_sound(); // play sound
						ball_move_speedx = 1;
					}
					else if (ballxy[0] >= player_x + 8 && ballxy[0] <= player_x + 12) {
						nice_shot_sound();
						ball_move_speedx = 3;
					}
					else
					{
						ball_bounce_sound(); // play sound
						ball_move_speedx = 0;
					}
					hits++;


				}
				else // does not colide with paddle
				{
					player_lose_sound();
					enemy_score++;
					if (menu == 0) {
						gotogxy(score_offset_x, 0);
						gprintf(" ");
						gotogxy(score_offset_x, 0);
						gprintf("%d", enemy_score);

					}
					halt = 1;
					pause4 = 100;
					ballxy[0] = 200;
				}
				
			}

		}
		else 
		{
			if (player_x + 4 > Centerx) {
				player_x--;
			}
			else if (player_x + 4 < Centerx) {
				player_x++;
			}
		}

		if (joypad() & J_START && menu == 1) {
			gotogxy(2,4);
			gprintf("         ");
			gotogxy(2,6);
			gprintf("     ");
			gotogxy(2,7);
			gprintf("      ");
			gotogxy(12,4);
			gprintf("   ", pts);
			gotogxy(15,4);
			gprintf("     ");
			waitpadup();
			ballxy[0] = Centerx;
			ballxy[1] = Centery;
			player_score = 0;
			enemy_score = 0;
			gotogxy(score_offset_x, 0);
			gprintf("%d", enemy_score);
			gotogxy(score_offset_x,17);
			gprintf("%d", player_score);
			gotogxy(7, 0);
			gprintf("Enemy");
			gotogxy(winloseoffset, 0);
			gotogxy(7, 17);
			gprintf("Player");
			gotogxy(winloseoffset, 17);
			player_x = Centerx;
			enemy__x = Centerx;
			ball_move_speedx = Initial_ball_speed;
			ball_move_speedy = Initial_ball_speed;
			halt = 0;
			pause4 = 0;
			hits = 0;
			menu = 0;
		}

		if (menu == 1) {
			/*
			gotogxy(12,4);
			gprintf("%d", pts);
			gotogxy(16,15);
			gprintf("PTS:");
			*/
			if (joypad() & J_UP && pts < 64) {
				pts++;
				gotogxy(12, 4);
				gprintf("  ");
				gotogxy(12,4);
				gprintf("%d", pts);
				wait_vbl_done();

			}
			else if (joypad() & J_DOWN && pts > 0) {
				pts--;
				gotogxy(12, 4);
				gprintf("  ");
				gotogxy(12,4);
				gprintf("%d", pts);
				wait_vbl_done();


			}
			if (ballxy[0] > 8 && ballxy[0] + 8 < Width) player_x = ballxy[0];
		}

		/// player
		move_sprite(0, player_x, Width-8 - offset_y_player);
		move_sprite(2, player_x + 8, Width-8 - offset_y_player);
		// enemy
		move_sprite(1, enemy__x, 16 + offset_y);
		move_sprite(3, enemy__x + 8, 16 + offset_y);

		//  ball
		move_sprite(4, ballxy[0], ballxy[1]);

		while (paused && menu == 0) {
			if (joypad() & J_START) {
				waitpadup();
				paused = 0;
			}
		}

		wait_vbl_done();
	} // end in game loop

	// game over win/lose segment
	if (player_score > enemy_score) {
		gotogxy(7, 9);
		gprintf("WINNER");
		while (pause4++ < 150) {
			wait_vbl_done();
		}

	}
	else
	{
		gotogxy(6, 9);
		gprintf("game over");
		while (pause4++ < 255) {
			wait_vbl_done();
		}
	}

	gotogxy(5, 9);
	gprintf("          ");
	wait_vbl_done();
	ballxy[0] = Centerx;
	ballxy[1] = Centery;
    player_x = Centerx;
	enemy__x = Centerx;
	ball_move_speedx = Initial_ball_speed;
	ball_move_speedy = Initial_ball_speed;
	halt = 0;
	pause4 = 0;
	player_score = 0;
	enemy_score = 0;
	seed = 0;
	paused = 0;
	attack_type = 0;
	hits = 0;
	menu = 1;
	pts = 0;
	
	gotogxy(2,4);
	gprintf("First to ");
	gotogxy(2,6);
	gprintf("UP +");
	gotogxy(2,7);
	gprintf("DOWN -");
	gotogxy(12,4);
	gprintf("%d", pts);
	gotogxy(15,4);
	gprintf("PTS:");

	gotogxy(score_offset_x, 0);
	gprintf("  ");
	gotogxy(score_offset_x,17);
	gprintf("  ");
	gotogxy(4, 0);
	gprintf("         ");
	gotogxy(winloseoffset, 0);
	gprintf("         ");
	gotogxy(4, 17);
	gprintf("         ");
	gotogxy(winloseoffset, 17);
	gprintf("         ");

	wait_vbl_done();

	line(0,offset_y, 159,offset_y);
	line(0, 134, 159, 134);

	wait_vbl_done();

	}

}