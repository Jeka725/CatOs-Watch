#pragma once
#include <Arduino.h>
#include <GyverOLED.h>
#include "GyverButton.h"

static void tetrisGame() {
  const int W=10,H=16,CELL=4;
  static uint8_t board[H][W];
  static const int8_t cells[7][4][2]={
    {{0,0},{1,0},{2,0},{3,0}},{{0,0},{0,1},{1,1},{2,1}},
    {{2,0},{0,1},{1,1},{2,1}},{{1,0},{2,0},{1,1},{2,1}},
    {{1,0},{2,0},{0,1},{1,1}},{{1,0},{0,1},{1,1},{2,1}},
    {{0,0},{1,0},{1,1},{2,1}}
  };
  auto xy=[&](int type,int rot,int n,int& x,int& y){
    x=cells[type][n][0]; y=cells[type][n][1];
    for(int r=0;r<rot;r++){int nx=-y,ny=x;x=nx;y=ny;}
  };
  auto valid=[&](int type,int rot,int px,int py){
    for(int n=0;n<4;n++){int x,y;xy(type,rot,n,x,y);
      if(x+px<0||x+px>=W||y+py<0||y+py>=H||board[y+py][x+px]) return false;
    } return true;
  };
  auto lock=[&](int type,int rot,int px,int py){
    for(int n=0;n<4;n++){int x,y;xy(type,rot,n,x,y);
      if(x+px>=0&&x+px<W&&y+py>=0&&y+py<H) board[y+py][x+px]=1;
    }
  };
  auto lines=[&](){
    int count=0;
    for(int y=H-1;y>=0;y--){bool full=true;
      for(int x=0;x<W;x++) if(!board[y][x]){full=false;break;}
      if(full){count++;for(int yy=y;yy>0;yy--) memcpy(board[yy],board[yy-1],W);memset(board[0],0,W);y++;}
    } return count;
  };
  auto block=[&](int x,int y){oled.rect(x*CELL,y*CELL,x*CELL+CELL-1,y*CELL+CELL-1,OLED_FILL);};

  memset(board,0,sizeof(board)); randomSeed((uint32_t)micros());
  int type=random(7),rot=0,px=3,py=0,score=0,cleared=0;
  unsigned long lastFall=millis(); bool over=!valid(type,rot,px,py);
  reset_buttons();

  while(!over){
    buttons_tick();
    if(left.isClick()&&valid(type,rot,px-1,py))px--;
    if(right.isClick()&&valid(type,rot,px+1,py))px++;
    if(ok.isClick()){int nr=(rot+1)&3;if(valid(type,nr,px,py))rot=nr;else if(valid(type,nr,px-1,py)){px--;rot=nr;}else if(valid(type,nr,px+1,py)){px++;rot=nr;}}
    if(ok.isHold())break;

    unsigned long interval=(unsigned long)max(120,520-cleared*20);
    if(millis()-lastFall>=interval){
      lastFall=millis();
      if(valid(type,rot,px,py+1))py++;
      else{
        lock(type,rot,px,py);int c=lines();
        if(c==1)score+=100;else if(c==2)score+=300;else if(c==3)score+=500;else if(c>=4)score+=800;
        cleared+=c;type=random(7);rot=0;px=3;py=0;over=!valid(type,rot,px,py);
      }
    }

    oled.clear();oled.rect(0,0,39,63,OLED_STROKE);
    for(int y=0;y<H;y++)for(int x=0;x<W;x++)if(board[y][x])block(x,y);
    for(int n=0;n<4;n++){int x,y;xy(type,rot,n,x,y);if(x+px>=0&&x+px<W&&y+py>=0&&y+py<H)block(x+px,y+py);}
    oled.setScale(1);oled.setCursorXY(44,4);oled.print("TETRIS");
    oled.setCursorXY(44,20);oled.print("Score");oled.setCursorXY(44,29);oled.print(score);
    oled.setCursorXY(44,42);oled.print("Lines");oled.setCursorXY(44,51);oled.print(cleared);
    oled.update();delay(20);
  }

  if(over){
    oled.clear();oled.setScale(2);oled.setCursorXY(30,12);oled.print("GAME");oled.setCursorXY(30,31);oled.print("OVER");
    oled.setScale(1);oled.setCursorXY(31,53);oled.print("OK = выход");oled.update();
    reset_buttons();delay(1000);
    while(true){buttons_tick();if(ok.isClick()||ok.isHold())break;delay(20);}
  }
  reset_buttons();oled.clear();oled.update();
}
