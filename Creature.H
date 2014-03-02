/* Creature.H - creature class defs for xNeoterics alife simulation

   Copyright (C) 2003 astrodud
   Copyright (C) 1995-1998, 2000, 2001 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef __CREATURE_H__
#define __CREATURE_H__

#include <stdio.h>

#define int short
#define Boolean int
#define true 1
#define false 0

typedef enum tagParentage {
   FromFile, Genesis, Asexual, Sexual, Underflow, Introduced
} tParentage;

//enum { moveForward = 0, turnRight, turnLeft, eat, breed, fight };
enum { eat = 0, breed, moveForward, turnRight, turnLeft, fight };

#define REDRAW_OLD_LOCATION 0x0001
#define REDRAW_CURRENT_LOCATION 0x0002

#define MAXNOISE 0x0007

class Genome;
class Neoterics;

class Creature {
public:
   long Creature_ID, xcoord, ycoord, age, energy, mother, father, a_nAsexualBirths, a_nSexualBirths;
   short direction;
   Genome *genome; // = NULL;
   tParentage source;
   int last_action, index, good_waste, bad_waste;
   Boolean bDesireToBreed;
   unsigned char attack, defend, a_ucMass;
   unsigned char a_ucMetabolismTimer, a_ucPlantEfficiency, a_ucFleshEfficiency, a_ucWasteEfficiency;
   long a_nKills, old_x, old_y;
   Neoterics *neo;
   Creature *prev, *next;

   Creature( tParentage src, Neoterics *owner );
   Creature( Creature *dad, Creature *mom = NULL );
   ~Creature();

   void Initialize();
   void MoveForward();
   void TurnRight();
   void TurnLeft();
   Boolean Eat();
   Boolean Excrete();
   Boolean CanBreed();
   void Breed();
   void Fight();
   int ComputeAction();
   int DoAction();
   unsigned char GetSensor( unsigned char sensor );
   void WriteToFile( FILE *file );
   void ReadFromFile( FILE *file );
   void SetUpAccelerators();
   void WriteGenotype( char *fName );
   Boolean ReadGenotype( char *fName );
   void GetSourceString( char *src );
   void Output();

   void AddToGrid();
   void RemoveFromGrid();
};

#endif
