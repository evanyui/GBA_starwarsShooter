#include "header.h"
#include <stdlib.h>
#include <stdio.h>
#include "text.h"
#include "starwars.h"

//prototypes
void waitForVblank();
int laserBoundsCheck(int x, int y);
int enemyBoundCheck(int x, int y, int position);
void delay(int);
void drawHealthbar(int);

//constants
const int DELAY_CONSTANT = 100;
static int gameMode = 1;	//starts at title screen, 2-game, 3-gameover, 4-win 0-exits
const int STARNUM = 30;
const int STARSPEED = 1;
const int ENEMYMAX = 10;

//structure
//millenium falcon
typedef struct {
	int mfX, mfY, mfSpeed, ammo, dead;
} Player;

//tiefighter
typedef struct {
	int tfX, tfY, tfSpeed, ammo, dead, clear, position;  
} Enemy;

//lasers or bullet of player
typedef struct {
	volatile int lX, lY, lXold, lYold, lW, lH, laserSpeed, shot, collided;
} Laser;

//deathstar
typedef struct {
	volatile int dX, dY, lives, ammo;
} DeathStar;

typedef struct {
	volatile int sX, sY, oldX;
} Stars;
//stars array
Stars stars[30];

int main()
{
	REG_DISPCTL = MODE3 | BG2_ENABLE;

	while(1) {
		if(gameMode==1){
			//starts with titlescreen
			drawImage3(0,0, TITLESCREEN_WIDTH, TITLESCREEN_HEIGHT, titlescreen);
			while(gameMode==1){
				char stringBuffer[50];
				sprintf(stringBuffer, "Press A to start");	
				drawString(146,80,stringBuffer,WHITE);
				if(KEY_DOWN_NOW(BUTTON_A) && gameMode==1) 
				{
					//start game
					gameMode=2;
					//clear titlescreen
					drawRect(0,0,160,240,BLACK);

					//show start string
					char stringBuffer[50];
					sprintf(stringBuffer, "Start!");	
					drawString(70,100,stringBuffer,WHITE);
					delay(20);
				}
			}
		}

		//starts game
		if(gameMode==2){
			//start music
			initMusic();

			//clear screen
			drawRect(0,0,160,240,BLACK);
			//settingup stars
			for(int s=0; s<STARNUM; s++) {
				int x = rand()%160;
				int y = rand()%240;
				stars[s].sX = x;
				stars[s].sY = y;
			}

			//max enemy amount
			int enemyNumber = ENEMYMAX;

			//initialize player position & properties
			Player player;
			player.mfX = 110;
			player.mfY = 125;
			player.mfSpeed = 2;
			player.ammo = 3;
			player.dead = 0;
			//laser shooting delay
			int fireLaser = 10;

			//initialize enemy position & properties
			//the come from the top right
			Enemy enemy;
			enemy.tfX = 210;
			enemy.tfY = 10;
			enemy.tfSpeed = 1;
			enemy.ammo = 3;
			enemy.dead = 1;		//started as dead, so can spawn
			enemy.clear = 0;
			enemy.position = 0; 	//1-left, 0-right, starts from right
			//laser shooting interval, yes enemy shoots faster
			int eFireLaser = 5;
			int spawnDelay = DELAY_CONSTANT;

			//Setup deathstar
			DeathStar deathstr;
			deathstr.dX = 120-DEATHSTAR_WIDTH/2;
			deathstr.dY = 15;
			deathstr.lives = 20;	//isit too hard to kill? *for some reason, its only 16
			deathstr.ammo = 8;
			int dFireLaser = 0;

			//set up laser for player and enemy
			const int laserAmmo = 3;
			Laser laser[laserAmmo];
			for(int i=0; i<3; i++) 
			{
				laser[i].lX = player.mfX+MILLFALC_HEIGHT/2-1;
				laser[i].lY = player.mfY+MILLFALC_WIDTH/4;
				laser[i].lXold = laser[i].lX;
				laser[i].lYold = laser[i].lY;
				laser[i].lW = 1;
				laser[i].lH = 6;
				laser[i].laserSpeed = 3;
				laser[i].shot = 0;
				laser[i].collided = 0;
			}
			const int eLaserAmmo = 3;
			Laser eLaser[eLaserAmmo];
			for(int q=0; q<3; q++) 
			{
				eLaser[q].lX = enemy.tfX+TIEFIGHTER_HEIGHT/2-1;
				eLaser[q].lY = enemy.tfY+TIEFIGHTER_WIDTH/4;
				eLaser[q].lXold = eLaser[q].lX;
				eLaser[q].lYold = eLaser[q].lY;
				eLaser[q].lW = 1;
				eLaser[q].lH = 6;
				eLaser[q].laserSpeed = 3;
				eLaser[q].shot = 0;
				eLaser[q].collided = 0;
			}
			const int dLaserAmmo = 8;
			Laser dLaser[dLaserAmmo];
			for(int d=0; d<8; d++) 
			{
				dLaser[d].lX = deathstr.dX+DEATHSTAR_HEIGHT/2-1;
				dLaser[d].lY = deathstr.dY+DEATHSTAR_WIDTH/4;
				dLaser[d].lXold = dLaser[d].lX;
				dLaser[d].lYold = dLaser[d].lY;
				dLaser[d].lW = 1;
				dLaser[d].lH = 6;
				dLaser[d].laserSpeed = 3;
				dLaser[d].shot = 0;
				dLaser[d].collided = 0;
			}

			//game loop
			while(gameMode==2)
			{				
				//controls
				if(KEY_DOWN_NOW(BUTTON_LEFT))
				{
					if(player.mfX>0)
						player.mfX -= player.mfSpeed;
				}
				if(KEY_DOWN_NOW(BUTTON_RIGHT))
				{	
					if(player.mfX<210)
						player.mfX += player.mfSpeed;
				}
				if(KEY_DOWN_NOW(BUTTON_A) && player.ammo>0 && laser[(player.ammo-1)%3].shot == 0)
				{
					if(fireLaser == 0) {
						//initialize laser starting point at player center
						laser[(player.ammo-1)%3].lY = player.mfY+MILLFALC_HEIGHT/4;
						laser[(player.ammo-1)%3].lX = player.mfX+MILLFALC_WIDTH/2-1;
						laser[(player.ammo-1)%3].shot = 1;
						laser[(player.ammo-1)%3].collided = 0;
						//decrease ammo
						player.ammo--;

						//set fireLaser = 4
						fireLaser = 10;
					}
				}
				if(KEY_DOWN_NOW(BUTTON_SELECT))
				{
					starwars_stop();
					gameMode = 1;
					break;
				}
				//interval for shooting
				if(fireLaser>0) {
					fireLaser--;
				}


				//collision of player with deathstar laser or enemy laser
				//collision of player with deathstar laser?
				for(int w=0; w<8; w++) {
					if((dLaser[w].lX>=player.mfX && dLaser[w].lX<=player.mfX+MILLFALC_WIDTH) 
						&& (dLaser[w].lY+dLaser[w].lH>=player.mfY && dLaser[w].lY+dLaser[w].lH<=player.mfY+MILLFALC_HEIGHT)) {
							player.dead = 1;
							dLaser[w].collided = 1;
							//player exploded
							drawImage3( player.mfX, player.mfY, EXPLOSION_WIDTH, EXPLOSION_HEIGHT, explosion);
							delay(20);
							break;
					}
				}
			
				//collision of player with enemy laser?
				for(int w=0; w<3; w++) {
					if((eLaser[w].lX>=player.mfX && eLaser[w].lX<=player.mfX+MILLFALC_WIDTH) 
						&& (eLaser[w].lY+eLaser[w].lH>=player.mfY+MILLFALC_HEIGHT/4 && eLaser[w].lY+eLaser[w].lH<=player.mfY+MILLFALC_HEIGHT)) {
							player.dead = 1;
							eLaser[w].collided = 1;
							//player exploded
							drawImage3( player.mfX, player.mfY, EXPLOSION_WIDTH, EXPLOSION_HEIGHT, explosion);
							delay(20);
							break;
					}
				}
				//check player status
				if(player.dead==1) {
					gameMode=3;
					//go to gameover screen
					break;
				}


				//enemy fires when you are inline
				if(enemy.dead==0 && enemy.tfX+TIEFIGHTER_WIDTH/2==player.mfX+MILLFALC_WIDTH/2 && eFireLaser == 0) {
					//initialize enemy laser starting point at enemy center
					eLaser[(enemy.ammo-1)%3].lY = enemy.tfY+TIEFIGHTER_HEIGHT/2;
					eLaser[(enemy.ammo-1)%3].lX = enemy.tfX+TIEFIGHTER_WIDTH/2-1;
					eLaser[(enemy.ammo-1)%3].shot = 1;
					eLaser[(enemy.ammo-1)%3].collided = 0;
					//decrease ammo
					enemy.ammo--;

					//set enemy fireLaser = 4
					eFireLaser = 10;
				}
				//interval for shooting (enemy)
				if(eFireLaser>0) {
					eFireLaser--;
				}

				//collision of player with enemy
				if(enemy.tfX>=player.mfX && enemy.tfX<=player.mfX+MILLFALC_WIDTH && enemy.tfY+TIEFIGHTER_HEIGHT>=player.mfY) {
					player.dead = 1;
					enemy.dead = 1;

					//enemy exploded
					drawImage3(enemy.tfX, enemy.tfY,EXPLOSION_WIDTH,EXPLOSION_HEIGHT, explosion);
					//player exploded
					drawImage3( player.mfX, player.mfY, EXPLOSION_WIDTH, EXPLOSION_HEIGHT, explosion);

					//gameover
					delay(20);
					gameMode=3;
					break;
				}
				//collision of enemy with player laser?
				for(int w=0; w<3; w++) {
					if((laser[w].lX>=enemy.tfX && laser[w].lX<=enemy.tfX+TIEFIGHTER_WIDTH) 
						&& (laser[w].lY>=enemy.tfY && laser[w].lY<=enemy.tfY+TIEFIGHTER_HEIGHT)
						&& enemy.clear==0) {
						enemy.dead = 1;
						laser[w].collided = 1;
						drawImage3(enemy.tfX, enemy.tfY,EXPLOSION_WIDTH,EXPLOSION_HEIGHT, explosion);
						break;
					}
				}
				//clear enemy dead out of bound body and reset position and SPAWN
				if(enemy.dead==1) {
					//clean dead enemy
					if(enemy.clear==0) {
						//clear dead enemy
						delay(2);
						drawRect(enemy.tfY, enemy.tfX, EXPLOSION_WIDTH, EXPLOSION_HEIGHT, BLACK);
						enemy.clear=1;
					}
					//calculate delay
					if(spawnDelay<0) {
						//spawn enemy 50% chance from different corner
						int positionCoin = rand()%2;
						if(positionCoin==1)
							enemy.position=1;
						else
							enemy.position=0;
						if(enemy.position==0) {
							enemy.tfX = 210;
						}else {
							enemy.tfX = 0;
						}
						enemy.tfY = 10;
						enemy.ammo = 3;
						enemy.dead = 0;
						enemy.clear = 0;
						
						//decrease enemyNumber
						enemyNumber--;

						//refresh delay countdown
						spawnDelay = DELAY_CONSTANT;
					}
					else {
						spawnDelay--;
					}
				}
				//check out of box but not dead
				if(!enemyBoundCheck(enemy.tfX, enemy.tfY, enemy.position)) {
					//enemy spawn from different direction
					drawRect(enemy.tfY, enemy.tfX, TIEFIGHTER_WIDTH, TIEFIGHTER_HEIGHT, BLACK);
					int positionCoin = rand()%2;
					if(positionCoin==1)
						enemy.position=1;
					else
						enemy.position=0;
					if(enemy.position==0) {
						enemy.tfX = 210;
					}else {
						enemy.tfX = 0;
					}
					enemy.tfY = 10;
					enemy.ammo = 3;
					enemy.dead=0;
					enemy.clear=0;
				}
				//move enemy to the left if they are alive
				if(enemyBoundCheck(enemy.tfX, enemy.tfY, enemy.position) && enemy.dead==0) {
					if(enemy.position==0)
						enemy.tfX-=enemy.tfSpeed;
					else
						enemy.tfX+=enemy.tfSpeed;
					
					//25% chance fly towards you
					int coin = rand()%3;
					if(!coin) 
						enemy.tfY+=enemy.tfSpeed;
				}

				//collision of deathstar with player laser?
				if(enemyNumber<=0){
					for(int w=0; w<3; w++) {
						if((laser[w].lX>=deathstr.dX && laser[w].lX<=deathstr.dX+DEATHSTAR_WIDTH) 
							&& (laser[w].lY>=deathstr.dY && laser[w].lY<=deathstr.dY+DEATHSTAR_HEIGHT)) {
								deathstr.lives--;
								laser[w].collided = 1;
								continue;
						}
					}
				}

				//vsync
				waitForVblank();

				//clear stars
				for(int s=0; s<STARNUM; s++) {
					drawRect(stars[s].oldX,stars[s].sY,1,1,BLACK);
				}
				//draw stars
				for(int s=0; s<STARNUM; s++) {
					drawRect(stars[s].sX,stars[s].sY,1,1,WHITE);
					//clear
					stars[s].oldX = stars[s].sX;

					//so when fighting deathstar, stars dont move
					if(enemyNumber>0) {
						//move stars
						stars[s].sX = (stars[s].sX+STARSPEED)%160;
					}
				}

				//happen only once, when enemy number reaches zero, but doesnt mean stop spawning
				if(enemyNumber==0){
					//BOSS WARNING
					char stringBuffer[50];
					drawRect(0,0,160,240,BLACK);
					sprintf(stringBuffer, "Deathstar appears!");
					drawString(70,70,stringBuffer,WHITE);
					delay(50);
					//clear string
					drawRect(70,70,10,140,BLACK);
					//count down
					//3
					sprintf(stringBuffer, "3");
					drawString(70,120,stringBuffer,WHITE);
					delay(20);
					//clear string
					drawRect(70,70,10,140,BLACK);
					//2
					sprintf(stringBuffer, "2");
					drawString(70,120,stringBuffer,WHITE);
					delay(20);
					//clear string
					drawRect(70,70,10,140,BLACK);
					//1
					sprintf(stringBuffer, "1");
					drawString(70,120,stringBuffer,WHITE);
					delay(20);
					//clear string
					drawRect(70,70,10,140,BLACK);
					//ready
					sprintf(stringBuffer, "May the force be with you!");
					drawString(70,50,stringBuffer,WHITE);
					delay(20);
					//clear string
					drawRect(70,50,10,180,BLACK);

					//START
					//draw deathstar
					drawImage3(deathstr.dX, deathstr.dY, DEATHSTAR_WIDTH, DEATHSTAR_HEIGHT, deathstar);
					//delay spawn of enemy
					spawnDelay=DELAY_CONSTANT;
					//player starts at very left
					player.mfX = 60;
					enemyNumber--;
				}

				//draw millenium falcon
				drawImage3(player.mfX, player.mfY, MILLFALC_WIDTH, MILLFALC_HEIGHT, millfalc);
				//check each ammo
				for(int j=0; j<3; j++) {
					if(!laserBoundsCheck(laser[j].lX, laser[j].lY)) {
						//out of box, clear laser, get ammo
						laser[j].shot = 0;
						player.ammo++;
					}
					else {	//if laser still moving
						laser[j].lXold = laser[j].lX;
						laser[j].lYold = laser[j].lY;
						laser[j].lY-=laser[j].laserSpeed;
					}
				}
				//clear prev laser movement for each shot laser
				for(int k=0; k<3; k++){
					if(laser[k].shot == 1)
						drawRect(laser[k].lYold, laser[k].lXold, laser[k].lH, laser[k].lW, BLACK);
				}
				//draw laser next step for each shot laser
				for(int l=0; l<3; l++) {
					//if laser collided with something
					if(laser[l].collided == 1) {
						drawRect(laser[l].lY, laser[l].lX, laser[l].lH, laser[l].lW, BLACK);
						//reset laser position and regain ammo
						laser[l].lY = player.mfY+MILLFALC_HEIGHT/4;
						laser[l].lX = player.mfX+MILLFALC_WIDTH/2-1;
						laser[l].shot = 0;
						laser[l].collided = 0;
						player.ammo++;
					}
					//or just keep moving 
					else if(laser[l].shot == 1) 
						drawRect(laser[l].lY, laser[l].lX, laser[l].lH, laser[l].lW, GREEN);
				}

				//draw enemy
				if(enemyBoundCheck(enemy.tfX, enemy.tfY, enemy.position) && enemy.dead==0) {
					if(enemy.position==0) 
						drawImage3(enemy.tfX, enemy.tfY, TIEFIGHTER_WIDTH, TIEFIGHTER_HEIGHT, tiefighter);
					else
						drawImage3(enemy.tfX, enemy.tfY, TIEFIGHTER1_WIDTH, TIEFIGHTER1_HEIGHT, tiefighter1);
				}
				//check each ammo for enemy
				for(int o=0; o<3; o++) {
					if(!laserBoundsCheck(eLaser[o].lX, eLaser[o].lY)) {
						//out of box, clear laser, get ammo
						eLaser[o].shot = 0;
						enemy.ammo++;
					}
					else {	//if laser still moving
						eLaser[o].lXold = eLaser[o].lX;
						eLaser[o].lYold = eLaser[o].lY;
						eLaser[o].lY+=eLaser[o].laserSpeed;
					}
				}
				//clear previous enemy laser
				for(int m=0; m<3; m++){
					if(eLaser[m].shot == 1)
						drawRect(eLaser[m].lYold, eLaser[m].lXold, eLaser[m].lH, eLaser[m].lW, BLACK);
				}
				//draw next enemy laser
				for(int n=0; n<3; n++) {
					if(eLaser[n].shot == 1) 
						drawRect(eLaser[n].lY, eLaser[n].lX, eLaser[n].lH, eLaser[n].lW, RED);
				}

				//draw deathstar
				if(enemyNumber<=0 && deathstr.lives>=0) {
					drawImage3(deathstr.dX, deathstr.dY, DEATHSTAR_WIDTH, DEATHSTAR_HEIGHT, deathstar);

					//deathstar fires when you are inline
					if(deathstr.lives>0 && deathstr.dX==player.mfX && dFireLaser == 0 && deathstr.ammo>0) {
						//initialize deathstar laser starting point at enemy center
						dLaser[(deathstr.ammo-1)%8].lY = deathstr.dY+DEATHSTAR_HEIGHT/2;
						dLaser[(deathstr.ammo-1)%8].lX = deathstr.dX+DEATHSTAR_WIDTH/2;
						dLaser[(deathstr.ammo-1)%8].shot = 1;
						dLaser[(deathstr.ammo-1)%8].collided = 0;
						//decrease ammo
						deathstr.ammo--;

						//set deathstar firing delay 
						dFireLaser = 10;
					}

					//interval for shooting (deathstar)
					if(dFireLaser>0) {
						dFireLaser--;
					}

					//check each ammo for deathstar
					for(int o=0; o<8; o++) {
						if(!laserBoundsCheck(dLaser[o].lX, dLaser[o].lY)) {
							//out of box, clear laser, get ammo
							dLaser[o].shot = 0;
							deathstr.ammo++;
						}
						else {	//if laser still moving
							dLaser[o].lXold = dLaser[o].lX;
							dLaser[o].lYold = dLaser[o].lY;
							dLaser[o].lY+=dLaser[o].laserSpeed;
						}
					}
					//clear previous daethstar laser
					for(int m=0; m<8; m++){
						if(dLaser[m].shot == 1)
							drawRect(dLaser[m].lYold, dLaser[m].lXold, dLaser[m].lH, dLaser[m].lW, BLACK);
					}
					//draw next deathstar laser
					for(int n=0; n<8; n++) {
						if(dLaser[n].shot == 1) 
							drawRect(dLaser[n].lY, dLaser[n].lX, dLaser[n].lH, dLaser[n].lW, RED);
					}

					//draw deathstar healthbar
					drawHealthbar(deathstr.lives);
				}

				//check deathstar status
				if(deathstr.lives<=0) {	//if dead?
					//go to gameover screen
					//clear deathstar
					drawRect(deathstr.dX, deathstr.dY, deathstr.dX, deathstr.dY, BLACK);
					drawImage3(deathstr.dX, deathstr.dY,
						EXPLOSION_WIDTH,EXPLOSION_HEIGHT, explosion);
					//draw string: you win!
					char stringBuffer[50];
					sprintf(stringBuffer, "You destroyed the deathstar");
					drawString(80,40,stringBuffer,WHITE);

					//go back to titlescreen after a few seconds delay
					delay(10);
					gameMode=4;
					break;
				}

			//end of game loop
			}
		}

		//game over
		if(gameMode==3) {
			//stop music
			starwars_stop();
			//draw gameover screen
			drawImage3(0,0,GAMEOVER_WIDTH, GAMEOVER_HEIGHT,gameover);
			//press a to start game
			if(KEY_DOWN_NOW(BUTTON_A) && gameMode==3) 
			{
				//start game
				gameMode=1;
			}
			if(KEY_DOWN_NOW(BUTTON_SELECT))
			{
				gameMode = 1;
			}
		}

		if(gameMode==4) {
			//stop music
			starwars_stop();
			//draw winning screen
			drawImage3(0,0,GAMEOVER1_WIDTH, GAMEOVER1_HEIGHT,gameover1);
			//press a to start game
			if(KEY_DOWN_NOW(BUTTON_A) && gameMode==4) 
			{
				//start game
				gameMode=1;
			}
			if(KEY_DOWN_NOW(BUTTON_SELECT))
			{
				gameMode = 1;
			}
		}
	}


}
void drawHealthbar(int hp) {
	//clear previous healthbar
	drawRect(5,0,8,200,BLACK); 		
	//draw healthbar
	int space=1;
	for(int i=0; i<hp; i++) {
		drawRect(5,5+i*5+i*space,8,4,WHITE);
	}
}

void waitForVblank()
{
	while(SCANLINECOUNTER > 160);
	while(SCANLINECOUNTER < 160);
}

int laserBoundsCheck(int x, int y) {
	if(x<-2 || x>240 || y<-6 || y>160) {
		return 0;
	}	
	return 1;
}

int enemyBoundCheck(int x, int y, int position) {
	if(position==0) {	//facing left
		if(x<0 || x>240 || y<0 || y>160) {
			return 0;
		}
	}else { 	//facing right
		if(x+TIEFIGHTER_WIDTH<0 || x+TIEFIGHTER_WIDTH>240 || y<0 || y>160) {
			return 0;
		}
	}
	return 1;
}

void delay(int n)
{
	volatile int x;
	for (int i=0; i<n*10000; i++)
	{
		x = x + 1;
	}
}