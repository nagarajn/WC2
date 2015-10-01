/*
 * Attempting to learn glcd library
 */

#include "glcd.h"

#include "fonts/allFonts.h"

void setup() {
	GLCD.Init(NON_INVERTED);
}



void loop()
{
  GLCD.DrawVLine(0, 0, 20);
}
