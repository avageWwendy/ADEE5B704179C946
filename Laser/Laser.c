#include <p32xxxx.h>
#include <plib.h>

#define Laser_Detect	PORTDbits.RD14

BOOL Laser_IsAlign(){
	return (!Laser_Detect);
}

void Config_Laser(){
	LATDCLR = 0x4000;
	TRISDCLR = 0x4000;
}

int main(){
	Config_Laser();

	while(1);
}
