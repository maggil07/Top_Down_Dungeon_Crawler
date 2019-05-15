/**
 * COMP 4300 - Assignment 4
 *
 * Created by:
 *      - Devin Marsh - 201239464
 *      - Justin Delaney - 201222684
 *	 
 *
 *	Use " ` " key to initalize leveling saving, do not add ".txt" to file name 
 *	
 * Issues:
 *	- Dragging, inserting and deleting only work in the 1st room loaded. We believe this is something to do with the way the mouse position is calculated 
 *  - Attacking an NPC entity after it has been dragged causes the game to crash, as the "click" variable of CDrag returns a nullptr
 *  - No grid snapping or asset changing working 
 *  - Save file level must be entered in console 
 */

#include <SFML/Graphics.hpp>
#include "GameEngine.h"

int main()
{
    GameEngine g("assets.txt");
    g.run();
}