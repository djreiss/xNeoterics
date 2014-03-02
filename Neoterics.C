/* Neoterics.C - main method for xNeoterics alife simulation

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
#include <time.h>
#define __need_getopt
#include <getopt.h>

#include "Neoterics.H"
#include "Creature.H"
#include "NeoWindow.H"
#include "Genome.H"

const float version = 2.05;

#undef int
int main( int argc, char *argv[] ) {
#define int short
   srand( time( NULL ) );
   Neoterics *neo1 = new Neoterics( argc, argv );
   if ( ! neo1->dontShowDisplayAtAll && neo1->window == NULL ) {
      try {
	 neo1->window = new NeoWindow( argv, neo1 );
      } catch( const char *msg ) {
	 fprintf( stderr, "Error: %s\n", msg );
	 neo1->window = NULL;
      }
   }

   while( ! neo1->done || neo1->keepTrying ) {
      neo1->SetUp( argv );
      neo1->RunSimulation();
   }
   delete neo1;
   //exit( 0 );
}

Neoterics::Neoterics( int argc, char *argv[] ) {
   Initialize( argc, argv );
}

Neoterics::~Neoterics() {
   if ( window != NULL ) {
      delete window;
      window = NULL;
   }
   ClearAllMemory();
}

void Neoterics::RunSimulation() {
   lastTime = time( NULL );
   while( ( window != NULL || dontShowDisplayAtAll ) && ! done ) {
      Iterate();
   }
}

void Neoterics::Iterate() {
   if ( started && ( ! paused || nextStep ) ) {
      SpendTime();
      if ( ( ( time_step - 1 ) % (long) outputSteps ) == 0 || nextStep ) {
	 Output();
	 if ( window != NULL ) window->RedrawChartWindow();
      }
      if ( window != NULL ) window->RedrawNNetPlot();
   }
   if ( nextStep ) nextStep = false; // Only do one step
   if ( ( saveEveryNsteps != -1 && time_step % (long) saveEveryNsteps == 0 && time_step != 0 ) ) {
      char tmp[50];
      strcpy( tmp, fileName );
      sprintf( fileName, "%s.%07d", tmp, time_step );
      SaveSimulation();
      strcpy( fileName, tmp );
   } 
   if ( window != NULL ) {
      if ( paused || ! started || 
	   difftime( time( NULL ), lastTime ) > 0.5 ) { // Only service the GUI once evey 0.5 seconds
	 window->CheckForEvent();
	 lastTime = time( NULL );
      }
   }
   if ( time_step > 50000 ) keepTrying = false; // It was viable if it lasted more than 50000 steps
}

void Neoterics::SpendTime() {
   nMaxAsexual = nMaxSexual = nMaxEnergy = nMaxAge = nMaxMass = nMaxKills = 0;
   dAvgMass = dAvgGWaste = dAvgBWaste = dAvgEnergy = dAvgAge = dAvgBreedAge = dAvgBreedEnergy = 0;
   dAvgFlesh = dAvgPlant = dAvgWaste = dAvgAttack = dAvgMut = dAvgCross = 0.0;

   Creature **ptr = ppCreatureList;	
   for (long i=0; i<max_index+1; i++) {
      Creature *creature = *ptr++;
      if (creature != NULL) {
	 long old_x = creature->xcoord;
	 long old_y = creature->ycoord;
	 int redraw_flags = 0;
	 if (creature->energy >= 5) {
	    redraw_flags = creature->DoAction();
	    if ( creature->Excrete() ) redraw_flags |= REDRAW_CURRENT_LOCATION;
	    if ( (redraw_flags & REDRAW_CURRENT_LOCATION) != 0 ) 
	       if ( window != NULL ) window->DrawAt( creature->xcoord, creature->ycoord, true );
	 } else {
	    delete creature;
	    continue;
	 }
	 if ( window != NULL && ( redraw_flags & REDRAW_OLD_LOCATION ) != 0 && 
	      ! leaveTrails ) {
	    window->DrawAt( old_x, old_y, true );
	 }
      }
   }
   //  Process the creatures who wanted to breed and/or excrete; sum up the statistics
   Boolean doStats = ( ( time_step % (long) outputSteps ) == 0 || nextStep );
   ptr = ppCreatureList;	
   for (long i=0; i<max_index+1; i++) {
      Creature *creature = *ptr++;
      if (creature != (Creature *)NULL) {
	 if (creature->bDesireToBreed) creature->Breed();
	 if ( doStats ) {
	    if (creature->a_nAsexualBirths > nMaxAsexual) nMaxAsexual = creature->a_nAsexualBirths;
	    if (creature->a_nSexualBirths > nMaxSexual) nMaxSexual = creature->a_nSexualBirths;
	    if (creature->energy > nMaxEnergy) nMaxEnergy = creature->energy;
	    if (creature->age > nMaxAge) nMaxAge = creature->age;
	    if (creature->a_ucMass > nMaxMass) nMaxMass = creature->a_ucMass;
	    if (creature->a_nKills > nMaxKills) nMaxKills = creature->a_nKills;
	    dAvgEnergy += (float) creature->energy;
	    dAvgGWaste += (float) creature->good_waste;
	    dAvgBWaste += (float) creature->bad_waste;
	    dAvgAge += (float) creature->age;
	    dAvgMass += (float) creature->a_ucMass;
	    dAvgBreedAge += (float) ( *( creature->genome->min_age_reproduce ) << 3 );
	    dAvgBreedEnergy += (float) ( *( creature->genome->min_energy_reproduce ) << 3 );
	    dAvgPlant += (float) creature->a_ucPlantEfficiency / 255.0;
	    dAvgFlesh += (float) creature->a_ucFleshEfficiency / 255.0;
	    dAvgWaste += (float) creature->a_ucWasteEfficiency / 255.0;
	    dAvgAttack += (float) creature->attack / 255.0;
	    dAvgMut += creature->genome->ProbMutation();
	    dAvgCross += creature->genome->ProbCrossover();
	 }
      }
   }
   if (doStats && num_creatures > 0) { // Determine the statistics
      double num = (double) num_creatures;
      dAvgEnergy = dAvgEnergy / num;
      dAvgGWaste = dAvgGWaste / num;
      dAvgBWaste = dAvgBWaste / num;
      dAvgAge = dAvgAge / num;
      dAvgMass = dAvgMass / num;
      dAvgBreedAge = dAvgBreedAge / num;
      dAvgBreedEnergy = dAvgBreedEnergy / num;
      dAvgPlant = dAvgPlant / num;
      dAvgFlesh = dAvgFlesh / num;
      dAvgWaste = dAvgWaste / num;
      dAvgAttack = dAvgAttack / num;
      dAvgMut = dAvgMut / num;
      dAvgCross = dAvgCross / num;
   }

   if ( varyEnv > 0 && time_step > startVarying ) {
      if ( total_energy < initial_energy / 2 ) energy_increment = varyEnv;
      else if ( total_energy > initial_energy / 4 * 3 ) energy_increment = -varyEnv;
      borrowed_energy -= energy_increment;
   }

   RedistributeEnergy(); // Redistribute the energy
   time_step ++;
}

void Neoterics::Output() {
   printf( "TimeStep: %d\tPop: %d\tBirths: %d,%d\tKills: %d\n", time_step, num_creatures,
	   asexual_births, sexual_births, num_kills );
   //printf( "\tAvg:      Energy: %5.0f\tAge: %5.0f\tMass: %4.0f\tBreedAge: %5.0f\tBreedEne: %5.0f\n",
   //	   dAvgEnergy, dAvgAge, dAvgMass, dAvgBreedAge, dAvgBreedEnergy );
   //printf( "\t          Effic(P,F,W): %5.3f,%5.3f,%5.3f\tWaste(G,B): %.0f,%.0f\tAttack: %5.3f\n",
   //	   dAvgPlant, dAvgFlesh, dAvgWaste, dAvgGWaste, dAvgBWaste, dAvgAttack );
   //#ifdef MUT_IN_GENOME
   //printf( "\t          Mut: %.5f\tCross: %.5f\n", dAvgMut, dAvgCross );
   //#endif
   //printf( "\tMax:      Energy: %7d\tAge: %6d\tMass: %6d\n", nMaxEnergy, nMaxAge, nMaxMass );
   CalculateTotalEnergy();
   printf( "\tTotal: %8d\tPlant: %7d\tFlesh: %7d\tWaste: %7d\tCreat: %7d\tPoisn: %7d\n", total_energy, 
	   nEnergyInPlants, nEnergyInFlesh, nEnergyInWaste, nEnergyInCreatures, nTotalPoison );
   //printf( "\t          Poison:   %7d\n", nTotalPoison );
   printf( "\n" );
}

void Neoterics::Initialize() {
   window = NULL;
   max_index = -1;
   energy_pot = time_step = asexual_births = sexual_births = num_kills = borrowed_energy = 0;
   useFile = quitOnZeroPop = keepTrying = saveStats = false;
   varyEnv = -1;
   startVarying = 30000;
   food_button_increment = 100000;

   terrain_size = 128;
   prob_mutation = 0.008; // Averages to about 8 mutations per reproduction
   prob_crossover = 0.002; // These are defaults for initial creatures
   prob_insert_delete = 0.002; 
   num_initial_food_locs = 2000;
   maximum_creatures = 1000;
   initial_creatures = 200;
   age_factor = 9;
   bKeepMinimumPopulation = bUseSurvivorForMinimum = false;
   nMinimumPopulation = 5;
   bGiveHeadStart = bAllowAsexual = bAllowSexual = true;
   num_initial_meat_locs = 1000;
   output_creature = -1;
   outputSteps = 50;
   startoffDisplaying = true;
   dontShowDisplayAtAll = false;
   keepTrying = false;
   scale = 7;
   useFile = false;
   sprintf( fileName, "%s.out", programName );
   strcpy( creatureFile, "creature" );
   initialBugSeed = 1;
   saveEveryNsteps = -1;
   nCarcassDecayRate = terrain_size * terrain_size;
   nPoisonDecayRate = nWasteDecayRate = 50;

   //  Determine the location "and" mask and shift count
   location_mask = terrain_size - 1;
   location_shift = 1;
   while ((1 << location_shift) < terrain_size) location_shift++;

   pcPlayingField = pcMassField = pcWasteField = pcPoisonField = pcBarrierField = (unsigned char *) NULL;
   ppCreatureList = (Creature **) NULL;
   keepDrawing = true;
   leaveTrails = nextStep = paused = done = started = setup = false;
}

void Neoterics::Initialize( int argc, char *argv[] ) {
   strcpy( programName, argv[ 0 ] );
   Initialize();
   int cc;
   extern char *optarg;
   while ( ( cc = getopt( argc, argv, "ASHUDRQkXs:p:P:f:F:m:i:a:d:w:W:K:o:c:N:n:B:b:e:V:v:I:x:" ) ) != EOF ) {
      switch( cc ) {
      case 'A': bAllowAsexual = false; break;
      case 'S': bAllowSexual = false; break;
      case 'H': bGiveHeadStart = false; break;
      case 'K': bKeepMinimumPopulation = true; nMinimumPopulation = atoi( optarg ); break;
      case 'U': bUseSurvivorForMinimum = true; break;
      case 'k': keepTrying = true; quitOnZeroPop = true; break;
      case 'V': varyEnv = atoi( optarg ); break;
      case 'v': startVarying = atoi( optarg ); break;
      case 's': terrain_size = atoi( optarg ); break;
      case 'x': food_button_increment = atoi( optarg ); break;
      case 'p': prob_mutation = atof( optarg ); break;
      case 'P': prob_crossover = atof( optarg ); break;
      case 'I': prob_insert_delete = atof( optarg ); break;
      case 'f': num_initial_food_locs = atoi( optarg ); break;
      case 'F': num_initial_meat_locs = atoi( optarg ); break;
      case 'm': maximum_creatures = atoi( optarg ); break;
      case 'i': initial_creatures = atoi( optarg ); break;
      case 'a': age_factor = atoi( optarg ); break;
      case 'd': nCarcassDecayRate = atoi( optarg ); break;
      case 'w': nWasteDecayRate = atoi( optarg ); break;
      case 'W': nPoisonDecayRate = atoi( optarg ); break;
      case 'o': outputSteps = atoi( optarg ); break;
      case 'D': startoffDisplaying = false; break;
      case 'R': dontShowDisplayAtAll = true; startoffDisplaying = false; break;
      case 'c': scale = atoi( optarg ); break;
      case 'N': strcpy( fileName, optarg ); useFile = true; break;
      case 'B': strcpy( creatureFile, optarg ); break;
      case 'b': initialBugSeed = atoi( optarg ); break;
      case 'n': saveEveryNsteps = atoi( optarg ); break;
      case 'Q': quitOnZeroPop = true; break;
      case 'X': saveStats = true; break;
      case '?':
      case 'h':
      default:
	 fprintf( stderr, "%s options:\n", argv[0] );
	 fprintf( stderr, "-A: do not allow asexual reproduction\n" );
	 fprintf( stderr, "-S: do not allow sexual reproduction\n" );
	 fprintf( stderr, "-H: do not give the creatures a head start\n" );
	 fprintf( stderr, "-K: keep a minimum population at desired level (default is zero)\n" );
	 fprintf( stderr, "-U: use remaining survivors to clone if at minimum population (for -K)\n" );
	 fprintf( stderr, "-s: set terrain size (default is %d)\n", terrain_size );
	 fprintf( stderr, "-c: set display scale (default is %d)\n", scale );
	 fprintf( stderr, "-p: set default probability of mutation (default is %f)\n", prob_mutation );
	 fprintf( stderr, "-P: set default probability of crossover (default is %f)\n", prob_crossover );
	 fprintf( stderr, "-I: set default probability of insertions/deletions (default is %f)\n", prob_insert_delete );
	 fprintf( stderr, "-f: set number of initial food locations (default is %d)\n", num_initial_food_locs );
	 fprintf( stderr, "-F: set number of initial meat locations (default is %d)\n", num_initial_meat_locs );
	 fprintf( stderr, "-m: set maximum population (default is %d)\n", maximum_creatures );
	 fprintf( stderr, "-i: set initial population (default is %d)\n", initial_creatures );
	 fprintf( stderr, "-a: set age factor (default is %d)\n", age_factor );
	 fprintf( stderr, "-d: set carcass decay rate (default is %d)\n", nCarcassDecayRate );
	 fprintf( stderr, "-w: set waste decay rate (default is %d)\n", nWasteDecayRate );
	 fprintf( stderr, "-W: set poison decay rate (default is %d)\n", nPoisonDecayRate );
	 fprintf( stderr, "-D: do not start off updating the display (faster)\n" );
	 fprintf( stderr, "-R: do not show the display at all (for running remotely over an xterm)\n" );
	 fprintf( stderr, "-V: continuously vary the total energy available in the environment\n" );
	 fprintf( stderr, "-v: start the varying after desired number of time steps\n" );
	 fprintf( stderr, "-x: set the food +/- button increment size (default is %d)\n", food_button_increment );
	 fprintf( stderr, "-k: keep trying until a viable population is reached (more than 50000 steps)\n" );
	 fprintf( stderr, "-B: read initial bugs from a creature file (default is %s)\n", creatureFile );
	 fprintf( stderr, "-b: number of bugs to seed from initial creature file (default is %d)\n", initialBugSeed );
	 fprintf( stderr, "-o: output global stats once every n steps (default is %d)\n", outputSteps );
	 fprintf( stderr, "-n: automatically save to file every n timesteps (default is none)\n" );
	 fprintf( stderr, "-N: name of file to save/load simulation to (default is %s)\n", fileName );
	 fprintf( stderr, "-X: also print all creature genome characteristics to the file %s.stats\n", fileName );
	 fprintf( stderr, "-Q: automatically quit when the population falls to zero\n" );
	 exit( 0 );
      }
   }
   //  Determine the location "and" mask and shift count
   location_mask = terrain_size - 1;
   location_shift = 1;
   while ((1 << location_shift) < terrain_size) location_shift++;
}

void Neoterics::SetUp( char **argv ) {
   done = false;
   paused = true;
   if ( dontShowDisplayAtAll || ! startoffDisplaying ) {
      SetUpRun();
      paused = false;
      started = true;
   }
   if ( useFile ) {
      char tmp[50]; // A hack to save some of the cmd-line options before they are erased by LoadSimulation
      strcpy( tmp, fileName );
      LoadSimulation();
      strcpy( fileName, tmp ); // End of hack
   }

   /*
#ifdef USE_BARRIER
   for ( long i = 0; i < 128; i ++ ) {
      long loc = ( i << location_shift ) + 64;
      pcBarrierField[loc] = 255;
   }
#endif
   */
}

void Neoterics::SetUpRun() {
   long total_locs, i, x, y, loc;
   unsigned char *ptr;
	
   ClearAllMemory(); //  Clear any previous memory   
   setup = true;

   //  Update the location "and" mask and shift count
   location_mask = terrain_size - 1;
   location_shift = 1;
   while ((1 << location_shift) < terrain_size) location_shift++;

   //  Allocate a new terrain arrays
   total_locs = terrain_size * terrain_size;
   pcPlayingField = (unsigned char *)calloc((size_t)total_locs, sizeof(unsigned char));
   if (pcPlayingField == (unsigned char *)NULL) exit(1);
   pcMassField = (unsigned char *)calloc((size_t)total_locs, sizeof(unsigned char));
   if (pcMassField == (unsigned char *)NULL) exit(1);
   pcWasteField = (unsigned char *)calloc((size_t)total_locs, sizeof(unsigned char));
   if (pcWasteField == (unsigned char *)NULL) exit(1);
   pcPoisonField = (unsigned char *)calloc((size_t)total_locs, sizeof(unsigned char));
   if (pcPoisonField == (unsigned char *)NULL) exit(1);
   pcBarrierField = (unsigned char *)calloc((size_t)total_locs, sizeof(unsigned char));
   if (pcBarrierField == (unsigned char *)NULL) exit(1);

   pcCreatureField = (Creature **) calloc( (size_t) total_locs, sizeof( Creature * ) );

   // Initialize the specified number of food locations to the starting food value
   if (num_initial_food_locs > 0) {
      if (num_initial_food_locs < (int)(0.67 * total_locs)) {
	 for (i=0; i<num_initial_food_locs; i++) {
	    x = (long)rand() & location_mask; //  Pick a new location
	    y = (long)rand() & location_mask;
	    loc = y * terrain_size + x;
	    if (pcPlayingField[loc] > 0) i--;
	    else pcPlayingField[loc] = 128;
	 }
      } else { //  If more than 2/3 set, set all and reset difference
	 ptr = pcPlayingField;
	 for (i=0; i<total_locs; i++) {
	    *ptr = 128;
	    ptr++;
	 }
	 for (i=total_locs; i>num_initial_food_locs; i--) {
	    x = (long)rand() & location_mask; //  Pick a new location
	    y = (long)rand() & location_mask;
	    loc = y * terrain_size + x;
	    if (pcPlayingField[loc] < 128) i++;
	    else pcPlayingField[loc] = 0;
	 }
      }
   }
   
   // Initialize the specified number of mass locations to the starting mass value
   if (num_initial_meat_locs > 0) {
      if (num_initial_meat_locs < (int)(0.67 * total_locs)) {
	 for (i=0; i<num_initial_meat_locs; i++) {
	    x = (long)rand() & location_mask; //  Pick a new location
	    y = (long)rand() & location_mask;
	    loc = y * terrain_size + x;
	    if (pcMassField[loc] > 0) i--;
	    else pcMassField[loc] = 32;
	 }
      } else { //  If more than 2/3 set, set all and reset difference
	 ptr = pcMassField;
	 for (i=0; i<total_locs; i++) {
	    *ptr = 32;
	    ptr++;
	 }
	 for (i=total_locs; i>num_initial_meat_locs; i--) {
	    x = (long)rand() & location_mask; //  Pick a new location
	    y = (long)rand() & location_mask;
	    loc = y * terrain_size + x;
	    if (pcMassField[loc] < 32) i++;
	    else pcMassField[loc] = 0;
	 }
      }
   }
   
   //  Allocate space for the creature list
   ppCreatureList = (Creature **)calloc((size_t)maximum_creatures, sizeof(Creature *));
   ID_count = 0; //  Create the initial creatures
   if (initial_creatures > 0) for (i=0; i<initial_creatures; i++) new Creature( Genesis, this );
   num_creatures = initial_creatures;

   if ( initialBugSeed > 0 ) for ( int jj = 0; jj < initialBugSeed; jj ++ ) new Creature( Introduced, this );
   
   time_step = asexual_births = sexual_births = borrowed_energy = energy_pot = 0; //  We start at time 0
   last_decay_start = last_waste_decay_start = last_poison_decay_start = num_kills = 0;

   total_energy = CalculateTotalEnergy();
   initial_energy = total_energy;
}	

void Neoterics::RedistributeEnergy() {
   if (borrowed_energy > 0 && energy_pot > 0) { //  Remove borrowed energy from energy pot
      if (borrowed_energy > energy_pot) {
	 borrowed_energy -= energy_pot;
	 energy_pot = 0;
      } else {
	 energy_pot -= borrowed_energy;
	 borrowed_energy = 0;
      }
   }
   if (borrowed_energy < 0) {
      energy_pot -= borrowed_energy;
      borrowed_energy = 0;
   }
   
   unsigned char *PlantPtr, *FleshPtr, decay; //  Decay flesh to plant
   long i, x, y, loc, total_locs;
   unsigned char add;
   if (nCarcassDecayRate > 0) {
      PlantPtr = pcPlayingField;
      PlantPtr += last_decay_start;
      FleshPtr = pcMassField;
      FleshPtr += last_decay_start;
      total_locs = terrain_size << location_shift;
      for (i=last_decay_start; i<total_locs; i+=nCarcassDecayRate) {
	 if (*FleshPtr > 0) {
	    decay = *FleshPtr >> 8;
	    if (decay == 0) decay = 1;
	    *FleshPtr -= decay;
	    loc = (decay << 3) + *PlantPtr;
	    if (loc > 255) {
	       energy_pot += loc - 255;
	       loc = 255;
	    }
	    *PlantPtr = loc;
	    if ( decay > 2 && window != NULL ) {
	       y = i >> location_shift;
	       x = i & location_mask;
	       window->DrawAt(x, y, true);
	    }
	 }
	 PlantPtr += nCarcassDecayRate;
	 FleshPtr += nCarcassDecayRate;
      }
      last_decay_start++;
      if (last_decay_start >= nCarcassDecayRate) last_decay_start = 0; 
   }	
   
   if (nWasteDecayRate > 0) { //  Decay waste to plant
      PlantPtr = pcPlayingField;
      PlantPtr += last_waste_decay_start;
      unsigned char *WastePtr = pcWasteField;
      WastePtr += last_waste_decay_start;
      total_locs = terrain_size << location_shift;
      for (i=last_waste_decay_start; i<total_locs; i+=nWasteDecayRate) {
	 if (*WastePtr > 1 << 6) { // Only decay if it's got a large enough amt of waste (64 here)
	    decay = *WastePtr >> 4; // Turn 1/8 into food
	    if (decay == 0) decay = 1;
	    *WastePtr -= decay;
	    loc = decay + *PlantPtr;
	    if (loc > 255) {
	       energy_pot += loc - 255;
	       loc = 255;
	    }
	    *PlantPtr = loc;
	    if ( decay > 2 && window != NULL ) {	
	       y = i >> location_shift;
	       x = i & location_mask;
	       window->DrawAt(x, y, true);
	    }
	 }
	 PlantPtr += nWasteDecayRate;
	 WastePtr += nWasteDecayRate;
      }
      last_waste_decay_start++;
      if (last_waste_decay_start >= nWasteDecayRate) last_waste_decay_start = 0; 
   }	
   
   if (nPoisonDecayRate > 0) { //  Decay poison to nothing
      unsigned char *PoisonPtr = pcPoisonField;
      PoisonPtr += last_poison_decay_start;
      total_locs = terrain_size << location_shift;
      for (i=last_poison_decay_start; i<total_locs; i+=nPoisonDecayRate) {
	 if (*PoisonPtr > 0) {
	    decay = *PoisonPtr >> 4; // Lose 1/8 of it to nothingness
	    if (decay == 0) decay = 1;
	    *PoisonPtr -= decay;
	    if ( decay > 2 && window != NULL ) {	
	       y = i >> location_shift;
	       x = i & location_mask;
	       window->DrawAt(x, y, true);
	    }
	 }
	 PoisonPtr += nPoisonDecayRate;
      }
      last_poison_decay_start++;
      if (last_poison_decay_start >= nPoisonDecayRate) last_poison_decay_start = 0; 
   }	
   
   while (energy_pot > 127) { //  Redistribute the energy pot
      x = rand() & location_mask;
      y = rand() & location_mask;
      loc = (y << location_shift) + x;
      if ( pcBarrierField[loc] > 0 ) continue;
      add = FMAX( 2, FMAX( pcPlayingField[loc-1], FMAX( pcPlayingField[loc+1], 
		      FMAX( pcPlayingField[((y-1) << location_shift) + x],
		      pcPlayingField[((y+1) << location_shift) + x] ) ) ) );
      if (pcPlayingField[loc] + add > 255) add = 255 - pcPlayingField[loc];
      pcPlayingField[loc] += add;
      energy_pot -= add;
      if ( window != NULL ) window->DrawAt(x, y, true);
   }
   while (energy_pot > 0) {
      x = rand() & location_mask;
      y = rand() & location_mask;
      loc = (y << location_shift) + x;
      if ( pcBarrierField[loc] > 0 ) continue;
      add = energy_pot;
      if (pcPlayingField[loc] > 255 - add) add = 255 - pcPlayingField[loc];
      pcPlayingField[loc] += add;
      energy_pot -= add;
      if ( window != NULL ) window->DrawAt(x, y, true);
   }
}

long Neoterics::CalculateTotalEnergy() {
   long total_locs, i, energy;
	
   //  Start with the energy in the energy pots
   energy = energy_pot - borrowed_energy;
   nEnergyInPots = energy;
	
   //  Add the energy from the terrain
   total_locs = terrain_size << location_shift;
   unsigned char *PlantPtr = pcPlayingField;
   unsigned char *FleshPtr = pcMassField;
   unsigned char *WastePtr = pcWasteField;
   unsigned char *PoisonPtr = pcPoisonField;
   nEnergyInPlants = nEnergyInFlesh = nEnergyInWaste = nTotalPoison = 0;
   for (i=0; i<total_locs; i++) {
      nEnergyInPlants += (long) *PlantPtr ++;
      nEnergyInFlesh += (long) *FleshPtr << 3;
      nEnergyInWaste += (long) *WastePtr ++;
      nTotalPoison += (long) *PoisonPtr >> 2;
      FleshPtr ++;
      PoisonPtr ++;
   }
   energy += nEnergyInPlants + nEnergyInFlesh + nEnergyInWaste;
	
   //  Now add the energy from the creatures
   Creature **ptr = ppCreatureList;
   nEnergyInCreatures = 0;
   for (i=0; i<max_index+1; i++) {
      Creature *creature = *ptr++;
      if (creature != (Creature *)NULL) {
	 nEnergyInCreatures += creature->energy;
	 nEnergyInCreatures += (long) creature->a_ucMass << 3;
	 nEnergyInCreatures += (long) creature->good_waste;
      }
   }
   energy += nEnergyInCreatures;
   total_energy = energy;
   return energy;
}

Creature *Neoterics::MaintainMinimumCreatures() {
   //  If less than minimum creatures, clone or create one
   if ( bKeepMinimumPopulation && num_creatures < nMinimumPopulation) {
      Creature *creature = NULL;
      if (bUseSurvivorForMinimum) {
	 for ( int j=0; j < max_index+1; j++ ) {
	    Creature *parent = ppCreatureList[j];
	    if ( parent != (Creature *) NULL ) {
	       creature = new Creature( parent );
	       creature->source = Underflow;
	       asexual_births --;
	       parent->a_nAsexualBirths --;
	       break;
	    }
	 }
      } else {
	 creature = new Creature( Underflow, this );
      }
      borrowed_energy += creature->energy;
      return creature;
   }
   return (Creature *) NULL;
}

int Neoterics::FindCreatureAtLoc(int skip, long x, long y) {
   if ( ppCreatureList == NULL ) return -1;
   if ( ppCreatureList[max_index] == NULL ) max_index --;
   long loc = (x << location_shift) + y;
   Creature *c = pcCreatureField[ loc ];
   if ( c == NULL ) return -1;
   //printf("seeking: ");
   while( 1 ) {
      //printf("%d  ", c->index);
      if ( c->index != skip ) break;
      c = c->next;
      if ( c == NULL ) break;
   }
   //printf("\n");
   if ( c == NULL || c->index == skip ) return -1;
   /*printf("found:  %d   %d ", skip,c->index);
   if ( c->prev != NULL && c->next != NULL ) printf( "%d %d\n", c->prev,c->next);
   else if ( c->prev != NULL ) printf("%d NULL\n",c->prev->index);
   else if ( c->next != NULL ) printf("NULL %d\n",c->next->index);
   else printf("NULL NULL\n");*/
   return c->index;

   /*
   if ( start < 0 || start >= max_index ) return -1;
   Creature **ptr = &( ppCreatureList[start] );
   for ( int i = start; i < max_index+1; i ++ ) {
      Creature *creature = *ptr++;
      if ( creature != NULL ) {
	 if ( x == creature->xcoord && y == creature->ycoord ) return i;
      }
   }
   return -1;*/
}	

long Neoterics::GetUniqueID() {
   long id = ID_count;
   ID_count ++;
   return id;
}

void Neoterics::ClearAllMemory() {
   //  Free any previous creature list
   if (ppCreatureList != (Creature **)NULL) {
      for (long i=0; i<maximum_creatures; i++) {
	 if (ppCreatureList[i] != (Creature *)NULL) delete ppCreatureList[i];
      }
      free(ppCreatureList);
   }
   ppCreatureList = (Creature **)NULL;

   //  Free any previous terrains
   if (pcPlayingField != (unsigned char *) NULL) free(pcPlayingField);
   pcPlayingField = (unsigned char *) NULL;
   if (pcMassField != (unsigned char *) NULL) free(pcMassField);
   pcMassField = (unsigned char *) NULL;
   if (pcWasteField != (unsigned char *) NULL) free(pcWasteField);
   pcWasteField = (unsigned char *) NULL;
   if (pcPoisonField != (unsigned char *) NULL) free(pcPoisonField);
   pcPoisonField = (unsigned char *) NULL;
   if (pcBarrierField != (unsigned char *) NULL) free(pcBarrierField);
   pcBarrierField = (unsigned char *) NULL;
   if (pcCreatureField != (Creature **) NULL) free(pcCreatureField);
   pcCreatureField = (Creature **) NULL;
   
   setup = false;
}

void Neoterics::SaveSimulation() { //  Routine to save the current simulation state
   FILE *file = fopen( fileName, "w" );
   fwrite( &version, sizeof( float ), 1, file );
   
   long bytecount = sizeof( Neoterics );
   fwrite( this, bytecount, 1, file );

   bytecount = terrain_size; //  Write out the terrain
   bytecount *= bytecount;
   fwrite( pcPlayingField, bytecount, 1, file );
   fwrite( pcMassField, bytecount, 1, file );
   fwrite( pcWasteField, bytecount, 1, file );
   fwrite( pcPoisonField, bytecount, 1, file );
   fwrite( pcBarrierField, bytecount, 1, file );
	    
   for ( int i = 0; i < maximum_creatures; i ++ ) { //  Write out the creature set
      Creature *creature = ppCreatureList[ i ];
      if ( creature != (Creature *) NULL ) creature->WriteToFile( file );
   }
   fclose( file );

   if ( saveStats ) DoStats();
}

void Neoterics::LoadSimulation() {
   FILE *file = fopen( fileName, "r" );
   if ( file == NULL ) return;
   ClearAllMemory(); //  Clear the current simulation
   
   float vers; // Read the version number
   fread( &vers, sizeof( float ), 1, file );

   if ( vers != version ) {
      fprintf(stderr, "That file is from an incompatible version of Neoterics! (%.2f)", vers );
      return;
   }

   NeoWindow *oldWind = window;
   long bytecount = sizeof( Neoterics );
   fread( this, bytecount, 1, file );
   window = oldWind;

   bytecount = terrain_size; //  Read in the terrain
   bytecount *= bytecount;

   pcPlayingField = (unsigned char *) malloc( (size_t) bytecount );
   fread( pcPlayingField, bytecount, 1, file );
   pcMassField = (unsigned char *) malloc( (size_t) bytecount );
   fread( pcMassField, bytecount, 1, file );
   pcWasteField = (unsigned char *) malloc( (size_t) bytecount );
   fread( pcWasteField, bytecount, 1, file );
   pcPoisonField = (unsigned char *) malloc( (size_t) bytecount );
   fread( pcPoisonField, bytecount, 1, file );
   pcBarrierField = (unsigned char *) malloc( (size_t) bytecount );
   fread( pcBarrierField, bytecount, 1, file );
   pcCreatureField = (Creature **) calloc( (size_t) bytecount, sizeof( Creature * ) );
	    
   //  Allocate space for the creature list
   ppCreatureList = (Creature **) calloc( (size_t) maximum_creatures, sizeof( Creature * ) );
	    
   //  Read in the creature set
   if ( num_creatures > 0 ) {
      for (int i = 0; i < num_creatures; i ++ ) {
	 Creature *creature = new Creature( FromFile, this );
	 creature->ReadFromFile( file );
	 creature->neo = this;
	 num_creatures --; // creating a creature increases this by one...gotta keep it the same
      }
   }
   fclose( file );
}

void Neoterics::DoStats() {
   char tmp[50];
   sprintf( tmp, "%s.stats", fileName );
   FILE *file = fopen( tmp, "w" );
         fprintf( file, "%3s %3s %3s  %3s %3s %3s %3s %3s %3s %3s %3s %3s %3s %3s %3s  %3s %3s %3s %3s %3s %3s %3s %3s\n\n",
		  "NSE", "NNE", "NOU",
		  "MET", "MAS", "PLA", "FLE", "WAS", "ATT", "CO1", "CO2", "MUT", "CRO", "BAG", "BEN",
		  "SE0", "SE1", "SE2", "SE3", "SE4", "SE5", "SE6", "SE7" );
   for ( long i = 0; i < max_index+1; i ++ ) {
      if ( ppCreatureList[i] != (Creature *) NULL ) {
	 Genome *g = ppCreatureList[i]->genome;
	 fprintf( file, "%3d %3d %3d  %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d  %3d %3d %3d %3d %3d %3d %3d %3d\n",
		  g->NumSensors(), g->NumNeurons(), g->NumOutputs(),
		  *(g->metabolism), *(g->maximum_mass), *(g->plant_eff), *(g->flesh_eff),
		  *(g->waste_eff), *(g->attack_defend), *(g->color1), *(g->color2),
		  *(g->prob_mut), *(g->prob_cross), *(g->min_age_reproduce), *(g->min_energy_reproduce),
		  g->sensors[0], g->sensors[1], g->sensors[2], g->sensors[3],
		  g->sensors[4], g->sensors[5], g->sensors[6], g->sensors[7] );
      }
   }
   fclose( file );
}
