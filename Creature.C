/* Creature.C - creature class for xNeoterics alife simulation

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "Neoterics.H"
#include "Creature.H"
#include "Genome.H"
#include "NeoWindow.H"

Creature::Creature( tParentage src, Neoterics *owner ) {
   neo = owner;
   genome = NULL;
   genome = new Genome( neo );
   source = src;
   Initialize();
   AddToGrid();
}

Creature::Creature( Creature *dad, Creature *mom ) {
   genome = NULL;
   neo = dad->neo;
   if ( neo == NULL ) neo = mom->neo;
   if ( mom != NULL ) {
     source = Sexual; //  Set the parentage
     Initialize();
     genome = new Genome( neo, dad->genome, mom->genome );
     energy = dad->energy / 2 + mom->energy / 2;
     dad->energy -= dad->energy / 2;
     mom->energy -= mom->energy / 2;
     energy += ( (long) dad->a_ucMass / 4 + (long) mom->a_ucMass / 4 ) << 3;
     dad->a_ucMass -= (long) dad->a_ucMass / 4;
     mom->a_ucMass -= (long) mom->a_ucMass / 4;
     SetUpAccelerators();
     a_ucMass = *(genome->maximum_mass) >> 3;
     energy -= (long) a_ucMass << 3;
     father = dad->Creature_ID;
     mother = mom->Creature_ID;
     xcoord = dad->xcoord; //  Copy the parents' location into the child
     ycoord = dad->ycoord;
     AddToGrid();
     neo->sexual_births++; //  Increment the birth counts
     dad->a_nSexualBirths++;
     mom->a_nSexualBirths++;
  } else {
    source = Asexual; //  Set the parentage
    Initialize();
    genome = new Genome( neo, dad->genome );
    energy = dad->energy / 2; //4 * 3;
    dad->energy -= dad->energy / 2; //4 * 3;
    energy += ( (long) dad->a_ucMass / 4 ) << 3;
    dad->a_ucMass -= (long) dad->a_ucMass / 4;
    SetUpAccelerators();
    a_ucMass = *(genome->maximum_mass) >> 3; //  Convert energy to 1/8th parent mass for offspring
    energy -= (long) a_ucMass << 3;
    father = dad->Creature_ID;
    xcoord = dad->xcoord; //  Copy the parent's location into the child
    ycoord = dad->ycoord;
    AddToGrid();
    neo->asexual_births++; //  Increment the birth count
    dad->a_nAsexualBirths++;
  }
}

Creature::~Creature() {
   if ( index == neo->output_creature ) neo->output_creature = -1;
   long loc = (xcoord << neo->location_shift) + ycoord;
   int new_food = (int) a_ucMass + (int) neo->pcMassField[loc];
   if (new_food > 255) {
      neo->energy_pot += (new_food - 255) << 3;
      new_food = 255;
   }

   neo->pcMassField[loc] = (unsigned char) new_food;
   neo->energy_pot += energy; //  Remove from list
   neo->energy_pot += good_waste;
   if ( neo->window != NULL ) neo->window->DrawAt( xcoord, ycoord, false );

   delete genome;
   if ( index >= 0 && neo->ppCreatureList[index] == this ) neo->ppCreatureList[index] = NULL;
   RemoveFromGrid();
   neo->num_creatures --;

   // If maintaining a minimum # of creatures, create or clone a new one, then draw it
   Creature *creature = NULL;
   if ( ( creature = neo->MaintainMinimumCreatures() ) != NULL ) {
      if ( neo->window != NULL ) neo->window->DrawAt( creature->xcoord, creature->ycoord, true );
   }
   //  Stop the simulation if no creatures left
   if ( neo->num_creatures <= 0 ) {
      neo->paused = true;
      neo->started = false;
      neo->done = neo->quitOnZeroPop; 
   }
}

void Creature::Initialize() {
   prev = next = NULL;
   bDesireToBreed = false;
   last_action = 0;
   a_ucMetabolismTimer = 0;
   xcoord = rand() & neo->location_mask;
   ycoord = rand() & neo->location_mask;
   direction = rand() & 0x0003;
   age = 0;
   energy = 1000;
   Creature_ID = neo->GetUniqueID();
   last_action = 0;
   a_nAsexualBirths = 0;
   a_nSexualBirths = 0;
   good_waste = bad_waste = 0;
   a_nKills = 0;

   for ( int i=0; i<neo->max_index+2; i++ ) { // Find a spot for the new creature
     if ( neo->ppCreatureList[i] == NULL ) { // in the list
       neo->ppCreatureList[i] = this;
       neo->num_creatures ++;
       index = i;
       if ( i > neo->max_index ) neo->max_index = i;
       break;
     }
   }

   if ( source == Genesis || source == Underflow ) {
     if (neo->bGiveHeadStart) genome->TrainNetwork();
     SetUpAccelerators();
   } else if ( source == Introduced ) {
     if ( ReadGenotype( neo->creatureFile ) ) {
        SetUpAccelerators(); //  Convert energy to initial mass
	neo->borrowed_energy += energy;
	a_ucMass = (unsigned char) ( (float) ( *(genome->maximum_mass) * 0.8 + 0.5 ) );
	neo->borrowed_energy += a_ucMass << 3;
	neo->output_creature = index;
     }
   }
}

void Creature::MoveForward() {
   RemoveFromGrid();
   int oldx = xcoord, oldy = ycoord;
   switch ( direction ) {
   case 0: ycoord--; ycoord &= neo->location_mask; break;
   case 1: xcoord++; xcoord &= neo->location_mask; break;
   case 2: ycoord++; ycoord &= neo->location_mask; break;
   case 3: xcoord--; xcoord &= neo->location_mask; break;
   }
   long loc = (ycoord << neo->location_shift) + xcoord;
   if ( neo->pcBarrierField[loc] > 0 ) {
      xcoord = oldx;
      ycoord = oldy;
   }
   AddToGrid();
}

void Creature::TurnRight() {
   direction ++;
   direction = direction & 0x0003;
}

void Creature::TurnLeft() {
   direction --;
   direction = direction & 0x0003;
}

Boolean Creature::Eat() {
   //  Determine which is the most abundant (weighted by food type), the creature eats that
   long loc = (ycoord << neo->location_shift) + xcoord;
   long plant = (long)neo->pcPlayingField[loc];
   plant *= (long)a_ucPlantEfficiency;
   plant = plant >> 3;
   long flesh = (long)neo->pcMassField[loc];
   flesh *= (long)a_ucFleshEfficiency;
   flesh = flesh >> 5;
   long waste = (long)neo->pcWasteField[loc];
   waste *= (long)a_ucWasteEfficiency;
   waste = waste >> 3;
	
   if (plant > flesh && plant > waste) { //  If more plants, eat them
      //  Remove up to the creatures mass in units from the plants
      unsigned char taken = neo->pcPlayingField[loc];
      if (taken == 0) return false;
      if (taken > a_ucMass) taken = a_ucMass;
      neo->pcPlayingField[loc] -= taken;
      plant = (long)taken; //  The amount taken is metabolised
      plant *= (long)a_ucPlantEfficiency;
      long gain = plant >> 8;
      energy += gain;
      good_waste += (long) taken - (long) gain;
      bad_waste += (long) ( (float) taken - (float) gain ) * 0.15; // Bad waste comes from immutable laws of physics
   } else if ( flesh > waste ) { //  If more flesh, eat that
      //  Remove up to the creatures mass/8 units from the carcass
      unsigned char taken = neo->pcMassField[loc];
      if (taken == 0) return false;
      if (taken > (a_ucMass >> 3)) taken = (a_ucMass >> 3);
      neo->pcMassField[loc] -= taken;
      flesh = (long)taken; //  The amount taken is metabolised
      flesh *= (long)a_ucFleshEfficiency;
      long gain = flesh >> 5;
      energy += gain;
      good_waste += ( (long) taken << 3 ) - (long) gain;
      bad_waste += (long) ( (float) ( (long) taken << 3 ) - (float) gain ) * 0.15;
   } else {
      unsigned char taken = neo->pcWasteField[loc];
      if (taken == 0) return false;
      if (taken > a_ucMass) taken = a_ucMass;
      neo->pcWasteField[loc] -= taken;
      waste = (long) taken; //  The amount taken is metabolised
      waste *= (long) a_ucWasteEfficiency;
      long gain = waste >> 8;
      energy += gain;
      bad_waste += (long) taken - (long) gain; // Eating waste only produces bad waste as metabolism
      neo->energy_pot += (long) taken - (long) gain; // To conserve energy
   }
   
   if (energy > 2047) { //  Any overrun from creature maximum goes to the pot
      good_waste += energy - 2047;
      energy = 2047;
   }
   return true;
}

Boolean Creature::Excrete() {
   Boolean out = false;
   long loc = (xcoord << neo->location_shift) + ycoord;

   if ( good_waste > 127 ) { // Right now all good waste just becomes "waste", which is edible
      int new_waste = good_waste;
      if ( new_waste + (int) neo->pcWasteField[loc] > 255 ) {
	 new_waste = 255 - (int) neo->pcWasteField[loc];
      }
      neo->pcWasteField[loc] += (unsigned char) new_waste;
      int extra = good_waste - new_waste;
      good_waste = extra;
      out = true;
   }
   
   if ( bad_waste > 127 ) { // For now make bad waste turn into "poison"
      int new_waste = bad_waste;
      if ( new_waste + (int) neo->pcPoisonField[loc] > 255 ) {
	 new_waste = 255 - (int) neo->pcPoisonField[loc];
      }
      neo->pcPoisonField[loc] += (unsigned char) new_waste;
      int extra = bad_waste - new_waste;
      bad_waste = extra;
      out = true;
   }
   return out;
}

void Creature::Breed() {
   bDesireToBreed = false; //  Turn off the desire flag
   Boolean breeded = false;
   if ( neo->num_creatures >= neo->maximum_creatures ) return;
   energy -= 5;
   neo->energy_pot += 5;
   if ( energy < 5 ) return;
   if ( neo->bAllowSexual ) { //  Try sexual reproduction if it is allowed
      Creature *mother = NULL; // See if there is another creature at this location who wants to breed
      int found = neo->FindCreatureAtLoc( index, xcoord, ycoord );
      if ( found >= 0 ) {
	 mother = neo->ppCreatureList[found];
	 /*if ( ! mother->bDesireToBreed ) {
	    if ( mother->next != NULL && mother->next != this && mother->next->bDesireToBreed ) mother = mother->next;
	    if ( mother->prev != NULL && mother->prev != this && mother->prev->bDesireToBreed ) mother = mother->prev;
	 }
	 if ( mother->bDesireToBreed ) {*/
	 if ( mother->CanBreed() ) { // If we have found a mate (willing or unwilling), try sexual reproduction
	    mother->bDesireToBreed = false;
	    mother->energy -= 5;
	    neo->energy_pot += 5;
	    new Creature( this, mother ); //  Create the new offspring
	    breeded = true;
	 }
      }
   }
   if ( neo->bAllowAsexual && ! breeded ) new Creature( this ); //  Create the new offspring
}

void Creature::Fight() {
   int found = index; //  See if there is a creature to fight
   while (found >= 0) {
      found = neo->FindCreatureAtLoc(found, xcoord, ycoord);
      if (found >= 0) {
	 Creature *victim = neo->ppCreatureList[found];
	 long damage = energy >> 2; // Damage is 1/4*energy*mass/victim->mass
	 damage *= (long) a_ucMass;
	 if ( victim->a_ucMass > 0 ) damage /= (long) victim->a_ucMass;
	 if ( (long) attack + (long) a_ucMass > (long) victim->defend + (long) victim->a_ucMass ) {
	    damage *= (long) attack; // Attacker is better than defender
	    if ( victim->defend > 0 ) damage /= (long) victim->defend;
	    if ( damage > victim->energy ) damage = victim->energy;
	    victim->energy -= damage; // so damage goes to defender.
	    energy += damage; // >> 1; // Gotta make it favorable for the bug to try it!
	 } else { // Defender's defenses are better than attacker
	    damage *= (long) victim->defend;
	    if ( attack > 0 ) damage /= (long) attack;
	    if ( damage > energy ) damage = energy;
	    energy -= damage; // so damage goes to attacker
	    victim->energy += damage; // >> 1; // and defender gets reward
	 }
	 //neo->energy_pot += ( damage - ( damage >> 1 ) );
	 if ( victim->energy < 5 ) {
	    neo->num_kills ++;
	    a_nKills ++;
	 }
	 if ( energy < 5 ) {
	    neo->num_kills ++;
	    victim->a_nKills ++;
	 }
	 return;
      }
   }
}

unsigned char Creature::GetSensor( unsigned char sensor ) {
   unsigned char temp = sensor & 31; // First 5 bits of sensor
   if ( temp < 25 ) { // Directional sensor
      long x, y, xoffset, yoffset;
      xoffset = ( temp % 5 ) - 2;
      yoffset = ( temp / 5 ) - 2;
      switch (direction) { //  Convert based on our direction
      case 0: x = xoffset; y = yoffset; break;
      case 1: x = -yoffset; y = xoffset; break;
      case 2: x = -xoffset; y = -yoffset; break;
      case 3: x = yoffset; y = -xoffset; break;
      }
      x += xcoord; //  Add the current location and wrap
      x &= neo->location_mask;
      y += ycoord;
      y &= neo->location_mask;
      temp = ( sensor >> 5 ) & 7; // Type of sensor is in last 3 bits
      long loc, plant, flesh, waste, poison; int found;
      switch ( temp ) {
      case 0: case 1: // Generic food sensor
	 loc = (y << neo->location_shift) + x;
         plant = (long) neo->pcPlayingField[loc];
         plant *= (long) a_ucPlantEfficiency;
         flesh = (long) neo->pcMassField[loc];
         flesh *= (long) a_ucFleshEfficiency;
         waste = (long) neo->pcWasteField[loc];
         waste *= (long) a_ucWasteEfficiency;
         plant = plant >> 8;
         flesh = flesh >> 5;
         waste = waste >> 8;
         if (plant > flesh && plant > waste) {
            return (unsigned char)plant;
         } else if ( flesh > waste ) {
            if (flesh > 255) flesh = 255;
            return (unsigned char)flesh;
         } else {
            return (unsigned char) waste;
         }
      case 2: // Poison sensor
#ifndef USE_BARRIER
      case 3:
#endif
	 loc = (y << neo->location_shift) + x;
	 poison = (long)neo->pcPoisonField[loc];
	 poison = poison << 1;
	 return (unsigned char) poison;
#ifdef USE_BARRIER
      case 3: // Barrier sensor
         loc = (y << neo->location_shift) + x;
         return neo->pcBarrierField[loc];
#endif
      case 4: // Creature's fitness
	 found = neo->FindCreatureAtLoc( index, x, y );
	 if (found >= 0) { // Return creature's relative fitness
	    Creature *seen = neo->ppCreatureList[found];
	    float rel = ( (float) seen->attack + (float) seen->a_ucMass ) / 
	       ( (float) attack + (float) a_ucMass ) * 127.0;
	    if ( rel > 255.0 ) rel = 255.0;
	    return (unsigned char) rel;
	 }
	 return 0;
      case 5: // Creature color1
	 found = neo->FindCreatureAtLoc( index, x, y );
	 if (found >= 0) return *( neo->ppCreatureList[found]->genome->color1 );
	 return 0;
      case 6: // Creature color2
	 found = neo->FindCreatureAtLoc( index, x, y );
	 if (found >= 0) return *( neo->ppCreatureList[found]->genome->color2 );
	 return 0;
      case 7: // Creature desire to breed
	 found = neo->FindCreatureAtLoc( index, x, y );
	 if (found >= 0) return neo->ppCreatureList[found]->bDesireToBreed * 255;
	 return 0;
      }
   } else { // Internal sensor: type is in last 3 bits
      int shift;
      temp = ( sensor >> 5 ) & 7;
      temp = temp % 7;
      switch( temp ) {
      case 0: return (unsigned char)(energy >> 3);
      case 1: return (unsigned char)(last_action << 5);
      case 2: return (unsigned char)(age >> neo->age_factor);
      case 3: return (unsigned char)(direction << 6);
      case 4:
	 shift = 8 - neo->location_shift;
	 if ( shift > 0 ) return (unsigned char) (xcoord << shift);
	 if ( shift == 0 ) return (unsigned char) xcoord;
	 shift = -shift;
	 return (unsigned char) (xcoord >> shift);
      case 5:
	 shift = 8 - neo->location_shift;
	 if ( shift > 0 ) return (unsigned char) (ycoord << shift);
	 if ( shift == 0 ) return (unsigned char) ycoord;
	 shift = -shift;
	 return (unsigned char) (ycoord >> shift);
      case 6: return a_ucMass;
      case 7: return ( rand() & 0xff );
      }
   }
}

int temp_input[32];
int Creature::ComputeAction() {
   int *input = temp_input;
   unsigned char *ptr = genome->sensors; //  Set up the input array
   int *input_ptr = input;
   for ( int neuron = 0; neuron < genome->NumSensors(); neuron ++ ) {
      unsigned char noise = rand() & MAXNOISE;
      *input_ptr++ = (int) GetSensor( *ptr ++ ) + ( noise & *( genome->noise_mask ) );
   }
   int action = genome->ComputeOutput( input );
   return action;
}   

int Creature::DoAction() {
   int action = ComputeAction();
   int redraw_flags = 0, mate_mass;
   switch (action) {
   case moveForward:
      MoveForward();
      energy -= 2;
      neo->energy_pot += 2;
      redraw_flags = REDRAW_OLD_LOCATION | REDRAW_CURRENT_LOCATION;
      break;
   case turnRight:
      TurnRight();
      energy--;
      neo->energy_pot ++;
      redraw_flags = REDRAW_CURRENT_LOCATION;
      break;
   case turnLeft:
      TurnLeft();
      energy--;
      neo->energy_pot ++;
      redraw_flags = REDRAW_CURRENT_LOCATION;
      break;
   case eat:
      if (Eat()) redraw_flags = REDRAW_CURRENT_LOCATION;
      break;
   case breed:
      bDesireToBreed = CanBreed();
      break;
   case fight:
      Fight();
      energy -= 5;
      neo->energy_pot += 5;
      break;
   }
   
   long loc = (ycoord << neo->location_shift) + xcoord; // If it's on poison, reduce energy
   energy -= neo->pcPoisonField[loc] >> 2;
   neo->energy_pot += neo->pcPoisonField[loc] >> 2;

   age++; //  Age the creature
   if (age >= (1 << neo->age_factor)) {
      long age_loss = age >> neo->age_factor;
      energy -= age_loss;
      neo->energy_pot += age_loss;
      bad_waste += (long) ( (float) age_loss * 0.15 );
   }
   if ( *(genome->metabolism) > 0) { //  Run the metabolism of the creature
      a_ucMetabolismTimer++;
      if (a_ucMetabolismTimer >= *(genome->metabolism)) {
	 a_ucMetabolismTimer = 0;
	 if (a_ucMass+3 < *(genome->maximum_mass)) {
	    energy -= 32;
	    a_ucMass += 4;
	    bad_waste += 4;
	 } else if (a_ucMass < *(genome->maximum_mass)) {
	    energy -= (*(genome->maximum_mass) - a_ucMass) << 3;
	    a_ucMass = *(genome->maximum_mass);
	    bad_waste += (long) ( (float) ( (*(genome->maximum_mass) - a_ucMass) << 3 ) * 0.15 );
	 }
      }
   }
   if (action != last_action) { //  If the draw scale is not x32, redraw creature when action changed
      redraw_flags |= REDRAW_CURRENT_LOCATION;
   }
   last_action = action;
   //if ( index == neo->output_creature ) Output();
   return redraw_flags;
}

Boolean Creature::CanBreed() {
   Boolean out = false;
   if ( bDesireToBreed == true ) return true;
   if ( energy > ( ( (long) *(genome->min_energy_reproduce) ) << 3 ) + 5 &&
	age > ( (long) *(genome->min_age_reproduce) ) << 3 ) out = true;
   //if ( energy > ( ( (long) *(genome->min_age_reproduce) ) << 3 ) + 5 ) bDesireToBreed = true;
   //if ( energy > 10 ) bDesireToBreed = true;
   return out;
}

void Creature::SetUpAccelerators() { //  Routine to initialize simulation accelerator variables
   a_ucMass = (unsigned char)((float)*(genome->maximum_mass) * 0.8);
   a_ucPlantEfficiency = *(genome->plant_eff);
   a_ucFleshEfficiency = *(genome->flesh_eff); // Dont let efficiencies add up to more than 255
   a_ucWasteEfficiency = *(genome->waste_eff);
   if ( a_ucPlantEfficiency == 0 && a_ucFleshEfficiency == 0 && a_ucWasteEfficiency == 0 ) {
      a_ucPlantEfficiency = a_ucFleshEfficiency = a_ucWasteEfficiency = 1;
   }
   long totalEff = a_ucPlantEfficiency + a_ucFleshEfficiency + a_ucWasteEfficiency;
   a_ucPlantEfficiency = (unsigned char) (float) a_ucPlantEfficiency / (float) totalEff * 255.0;
   a_ucFleshEfficiency = (unsigned char) (float) a_ucFleshEfficiency / (float) totalEff * 255.0;
   a_ucWasteEfficiency = (unsigned char) (float) a_ucWasteEfficiency / (float) totalEff * 255.0;
   attack = *(genome->attack_defend);
   defend = 255 - attack;
}	

void Creature::WriteToFile( FILE *file ) { //  Routine to write the creature to a file
   fwrite( this, sizeof( Creature ), 1, file );
   genome->WriteToFile( file );
}

void Creature::ReadFromFile( FILE *file ) {
   RemoveFromGrid();
   int oldindex = index;
   Neoterics *neoSave = neo;
   delete genome; // We saved the old pointer to the genome...gotta get rid of it.
   fread( this, sizeof( Creature ), 1, file );
   prev = next = NULL;
   neo = neoSave;
   index = oldindex;
   AddToGrid();
   genome = new Genome( neo );
   genome->ReadFromFile( file );
   genome->neo = neo;
}


void Creature::WriteGenotype(char *fName) { // Now writes to a binary file
   FILE *file = ::fopen(fName, "w");
   genome->WriteToFile( file );
   fclose( file );
}

Boolean Creature::ReadGenotype(char *fName) { //  Routine to read the genotype from a file
   FILE *file = ::fopen(fName, "r");
   if ( file != NULL ) genome->ReadFromFile( file );
   else return false;
   fclose( file );
   return true;
}

void Creature::GetSourceString( char *src ) {
   if ( source == Genesis ) strcpy( src, "Gen" );
   else if ( source == Asexual ) strcpy( src, "Asex" );
   else if ( source == Sexual ) strcpy( src, "Sex" );
   else if ( source == Underflow ) strcpy( src, "Under" );   
}	

void Creature::Output() {
   if ( ( neo->time_step % (long) neo->outputSteps ) == 0 || neo->nextStep ) {
      char src[16]; GetSourceString( src );
      printf( "Creature %d at index %d: ", Creature_ID, index );
      printf( "x = %d; y = %d; direction = %d; energy = %d; mass = %d; age = %d\n", xcoord, ycoord, 
	      direction, energy, a_ucMass, age ); 
      printf( "Next Action = %d; Last Action = %d; desireToBreed = %d; Source = %s\n", ComputeAction(), 
	      last_action, bDesireToBreed, src );
      printf( "Births = %d,%d; Kills = %d\n", a_nAsexualBirths, a_nSexualBirths, a_nKills );
      genome->Output();
   }
}

void Creature::AddToGrid() {
   next = prev = NULL;
   if ( neo == NULL || neo->pcCreatureField == NULL ) return;
   long loc = (xcoord << neo->location_shift) + ycoord;
   if ( neo->pcCreatureField[ loc ] == NULL ) {
      neo->pcCreatureField[ loc ] = this;
   } else {
      Creature *c = neo->pcCreatureField[ loc ];
      while( c->next != NULL && c != this ) c = c->next;
      if ( c != NULL && c != this ) c->next = this;
      prev = c;
   }
}

void Creature::RemoveFromGrid() {
   if ( neo == NULL ||  neo->pcCreatureField == NULL ) {
      next = prev = NULL;
      return;
   }
   long loc = (xcoord << neo->location_shift) + ycoord;
   if ( neo->pcCreatureField[ loc ] == this ) {
      neo->pcCreatureField[ loc ] = next;
      if ( next != NULL ) next->prev = NULL;
   } else {
      if ( prev != NULL ) prev->next = next;
      if ( next != NULL ) next->prev = prev;
   }
   next = prev = NULL;
}
