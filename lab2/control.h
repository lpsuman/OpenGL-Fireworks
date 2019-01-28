#ifndef CONTROL_H_
#define CONTROL_H_

void myKeyboard(unsigned char theKey, int mouseX, int mouseY);
void mouseMove(int x, int y);
void mouseButton(int button, int state, int x, int y);
void updateCamera();
void draw_horizon_grid();
void print_controls_info();

#endif