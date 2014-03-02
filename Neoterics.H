/* Neoterics.H - main defs for xNeoterics alife simulation

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

#ifndef __NEOTERICS_H__
#define __NEOTERICS_H__

#include <sys/types.h>

#define int short
#define Boolean short
#define true 1
#define false 0

#define FMAX(a,b) ((a) > (b) ? (a) : (b))

#define USE_BARRIER

class NeoWindow;
class Creature;

class Neoterics {
public:
   Neoterics( int argc, char *argv[] );
   virtual ~Neoterics();

   long total_energy, nMaxAsexual, nMaxSexual, nMaxEnergy, nMaxAge, nMaxMass, nMaxKills;
   double dAvgEnergy, dAvgAge, dAvgMass, dAvgPlant, dAvgFlesh, dAvgBreedAge, dAvgBreedEnergy;
   double dAvgGWaste, dAvgBWaste, dAvgWaste, dAvgAttack, dAvgMut, dAvgCross;
   NeoWindow *window; // = NULL;
   time_t lastTime;
	
   void RunSimulation();
   void Iterate();
   void SpendTime();
   void Output();

   long terrain_size, nWasteDecayRate, nPoisonDecayRate, max_index; // = -1;
   float prob_mutation, prob_crossover, prob_insert_delete;
   long age_factor, initial_creatures, maximum_creatures, location_mask, num_initial_food_locs;
   Boolean bKeepMinimumPopulation, bUseSurvivorForMinimum, bGiveHeadStart, bAllowAsexual, bAllowSexual;
   long nMinimumPopulation, num_initial_meat_locs, nCarcassDecayRate, output_creature, outputSteps;
   Boolean startoffDisplaying, useFile, quitOnZeroPop;
   int scale, location_shift, saveEveryNsteps, initialBugSeed;
   char fileName[ 50 ], creatureFile[ 50 ], programName[ 50 ];
   Boolean dontShowDisplayAtAll, keepTrying, saveStats;

   void Initialize();
   void Initialize( int argc, char *argv[] );
   void SetUp( char *argv[] );
   void SetUpRun();
   
   unsigned char *pcPlayingField, *pcMassField, *pcWasteField, *pcPoisonField, *pcBarrierField;
   Creature **ppCreatureList, **pcCreatureField;
   long energy_pot, time_step, asexual_births, sexual_births, ID_count, num_kills;
   long borrowed_energy, num_creatures, last_decay_start, initial_energy, energy_increment;
   Boolean paused, done, nextStep, started, setup, leaveTrails, keepDrawing;
   long nEnergyInPlants, nEnergyInFlesh, nEnergyInCreatures, nEnergyInPots, nEnergyInWaste;
   long varyEnv, startVarying, food_button_increment;
   long last_waste_decay_start, last_poison_decay_start, nTotalPoison;

   void RedistributeEnergy();
   long CalculateTotalEnergy();
   Creature *MaintainMinimumCreatures();
   int FindCreatureAtLoc(int skip, long x, long y);
   long GetUniqueID();
   void ClearAllMemory();

   void LoadSimulation();
   void SaveSimulation();
   void DoStats();
};

#endif
